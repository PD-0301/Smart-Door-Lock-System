String mp = "123456"; //master password


int sleep=0; // variable to flag if the lock should go to sleep mode
int to_login=0; // variable to flag if the lock should return to login screen


//Initialising variables and headers for RFID
String RF_User="";
String RF_Pass="";
int RF_Inp=0;
#include <SPI.h>
#include <MFRC522.h>
#define RST_PIN         9          
#define SS_PIN          10          
MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;


//Initialising variables and headers for keypad
#include<Keypad.h>
const byte ROWS = 4;
const byte COLS = 4;
byte rowPins[ROWS] = {33,34,35,36};
byte colPins[COLS] = {32,31,29,28};
char hexaKeys[4][4] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
Keypad customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);


//Initialising variables and headers for parallax LCD
#include<ParallaxLCD.h>
ParallaxLCD lcd(1,2,16);
byte customCharacters[8][8] = {0, 4, 14, 31, 14, 4, 0, 0,   // Diamond
                               15, 0, 9, 13, 15, 0, 9, 13, // Heart
                               0, 4, 14, 31, 14, 4, 14, 0,  // Spade
                               4, 14, 4, 10, 31, 10, 4, 14, // Club (sorta...)
                               4, 14, 21, 4, 4, 4, 4, 4,    // Up
                               4, 4, 4, 4, 4, 21, 14, 4,    // Down
                               27, 22, 13, 27, 22, 13, 27, 22, // Stipple pattern #1
                               42,20,42,20,42,20,42,20 // Stipple pattern #2
                             };


//Initiialising variables and header for SD card
#include<SD.h>
const int chipSelect = BUILTIN_SDCARD;


//pin for Ultrasonic sensor
#define ping 7


//pin for IR Sensor
#define op 6


//pin for solenoid
#define sol 37


void setup() {
//setting up solenoid pin
pinMode(sol,OUTPUT);


//RFID initialization
SPI.begin();              
mfrc522.PCD_Init();      
for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;


//Setting up IR Sensor
pinMode(op,INPUT);


//Setting up the LCD
lcd.setup();
delay(1000);
lcd.at(0,0,"                                                                                                       ");
int i;
  for(i=0; i < 8; i++) {
    lcd.setCustomCharacter(i, customCharacters[i]);
  }
  lcd.cursorOff();


//initialising SD card
if(SD.begin(chipSelect)){/*card initialised*/}
else{display(0,0,1,1,"No SD Card"); delay(1000);}
}


void loop() {
if(sleep){asleep();}  // goes to sleep mode if sleep variable is set to high
awake();
}


// awake fucntion handles the program flow when screen is awake
void awake(){
display(0,5,1,1,"Hello!");
delay(1000);
Login:
display(0,1,1,1,"Enter User ID");
String ID,Pass_act;
ID=usr_inp(1); // obtain  user ID input
if(to_login){to_login=0; goto Login;} //goes back to login screen when 'D' key is pressed
else if(sleep){return;} // exits awake funciton and goes to sleep mode
if(ID=="0000"){}  
else if(usrState(ID)==0){ //when user ID is not found
  if(RF_Inp){lcd.playTone(300, 160, 160); RF_User=""; RF_Pass=""; RF_Inp=0; goto Login;}
  display(0,2,1,1,"ID not found");
  delay(1000);
  goto Login;
  }else if(usrState(ID)==2){ // When the user ID is blocked
    display(0,3,1,1,String("User ").concat(ID));
    display(1,4,1,0,"Blocked");
    if(RF_Inp){RF_User=""; RF_Pass=""; RF_Inp=0;}
    delay(1000);
  goto Login;
  }


Pass_act = Password(ID); //Obtaining Actual Password from Database


if(validate_usr(Pass_act," Enter Password","   Incorrect","   Password!",1)){ // when user enters correct password within 3 attempts or uses valid RFID tag


  if((ID!="0000")/*master*/ || (RF_Inp==1)){  // Opening door for validated regular user
    display(0,3,1,1,"Welcome!!");
    RF_User=""; RF_Pass=""; RF_Inp=0;
    digitalWrite(sol,HIGH);
    delay(3000);
    sleep=1;
    return; //goes to sleep mode (Door should be locked in sleep mode)
  }
  else if(ID=="0000"){  // goes to master mode for correct master credentials
  MasterPage:
  int opn_mng = choice("Open Door","Manage Users"); // choice between "Open Door" and "Manage Users"
  if(to_login){to_login=0; goto Login;} //To handle D key input
  if(opn_mng==1){//Open Door
    display(0,3,1,1,"Welcome!!");
    digitalWrite(sol,HIGH);
    delay(3000); //door is opened here for 3 seconds
    sleep=1;
    return; //goes to sleep mode (Door will be locked in sleep mode)
  }else{//Manage Users
    int add_edit = choice("Add User","Edit User");  // choice between "Add User" and "Edit User"
    if(to_login){to_login=0; goto Login;} //To handle D key input
    if(add_edit==1){//add new user
      String new_ID = newID(); //Deciding new_ID
      display(0,2,1,1,"New User ID:");
      display(1,6,1,0,new_ID);
      delay(1500);
    TryAgain: // return point for re-attempt at setting password
      display(0,2,1,1,"Set Password");
      String p1,p2;
      p1 = pass_inp(0);
      if(to_login){to_login=0; goto Login;} //To handle D key input
      else if(sleep){return;}
      if(validate_usr(p1,"Confirm Password","  Passwords do","   not match!",0)){ //when the "Set Password" and "Confirm Password" entries match
        if(to_login){to_login=0; goto Login;} //To handle D key input
        else if(sleep){return;}
        add(new_ID,p1); // adding the new ID and password to database
        display(0,3,1,1,String("User ").concat(new_ID));
        display(1,5,1,0,"Added");
        delay(1000);
        TagReg:
        int RFID = choice("Register RFID","No RFID"); //choice bbetween "Register RFID" and "No RFID"
        if(to_login){to_login=0; goto Login;} //To handle D key input
        if(RFID==1){
          if(rfid_write(new_ID+p1)){display(0,1,1,1,"Tag registred");
          display(1,2,1,0,"successfully");
          delay(2000);
          return;
          }
          else{goto TagReg;}
          }
        else{
          return; //go to mastermode first page
        }
      }else{
        //options to try again or exit
        int try_exit = choice("Try Again","Exit");
        if(to_login){to_login=0; goto Login;} //To handle D key input
        if(try_exit==1){
          goto TryAgain;
        }else{
          goto MasterPage;
        }
      }


    }else{//(Un)Block  |  Delete
      int blo_del = choice("(Un)Block","Delete");
      if(to_login){to_login=0; goto Login;} //To handle D key input
      if(blo_del==1){
        int blo_un = choice("Block","Unblock");
        if(to_login){to_login=0; goto Login;} //To handle D key input
        if(blo_un==1){//block
        Block:
          int attmpt=1;
          while(attmpt<=3){
            display(0,1,1,1,"Enter User ID");
            String usr = usr_inp(0);
            if(to_login){to_login=0; goto Login;} //To handle D key input
            else if(sleep){return;}
            switch(usrState(usr)){
              case 0: display(0,2,1,1,"ID not found");
                      delay(1000);
                      attmpt++;
                      break;
              case 1: if(validate_usr(mp/*master pwd*/," Enter Password","   Incorrect","   Password!",0)){
                      if(to_login){to_login=0; goto Login;} //To handle D key input
                      else if(sleep){return;}
                      block(usr);
                      display(0,3,1,1,String("User ").concat(usr));
                      display(1,4,1,0,"Blocked");
                      delay(1000);
                      goto MasterPage;
                      }
                      else{
                      display(0,2,1,1,"Verification");
                      display(1,5,1,0,"Failed");
                      delay(1000);
                      return;
                      }
                      break;
              case 2: display(0,2,1,1,"User already");
                      display(1,4,1,0,"blocked!");
                      delay(1000);
                      goto MasterPage;
              default: break;
            }
           
          }
        if(attmpt>3){
          if(choice("Try Again","Exit")==1){if(to_login){to_login=0; goto Login;} //To handle D key input
            goto Block;
          }else{
            goto MasterPage;
          }
        }
        }else{//unblock
        Unblock:
          int attmpt=1;
          while(attmpt<=3){
            display(0,1,1,1,"Enter User ID");
            String usr = usr_inp(0);
            if(to_login){to_login=0; goto Login;} //To handle D key input
            else if(sleep){return;}
            switch(usrState(usr)){
              case 0: display(0,2,1,1,"ID not found");
                      delay(1000);
                      attmpt++;
                      break;
              case 2: if(validate_usr(mp/*master pwd*/," Enter Password","   Incorrect","   Password!",0)){
                      if(to_login){to_login=0; goto Login;} //To handle D key input
                      else if(sleep){return;}
                      unblock(usr);
                      display(0,3,1,1,String("User ").concat(usr));
                      display(1,3,1,0,"Unblocked");
                      delay(1000);
                      goto MasterPage;
                      }
                      else{
                      display(0,2,1,1,"Verification");
                      display(1,5,1,0,"failed");
                      delay(1000);
                      return;//goto wakeup;
                      }
                      break;
              case 1: display(0,4,1,1,"User not");
                      display(1,4,1,0,"blocked!");
                      delay(1000);
                      goto MasterPage;
              default: break;
            }
          }
        if(attmpt>3){
          if(choice("Try Again","Exit")==1){to_login=0; goto Login;} //To handle D key input
            goto Unblock;
          }else{ if(to_login){
            goto MasterPage;
          }
        }
        }
      }else{//deletion
            display(0,1,1,1,"Enter User ID");
            String usr = usr_inp(0);
            if(to_login){to_login=0; goto Login;} //To handle D key input
            else if(sleep){return;}
            if(usrState(usr)!=0){
                if(validate_usr(mp/*master pwd*/," Enter Password","   Incorrect","   Password!",0)){
                      if(to_login){to_login=0; goto Login;} //To handle D key input
                      else if(sleep){return;}
                      del(usr);
                      display(0,3,1,1,String("User ").concat(usr));
                      display(1,4,1,0,"Deleted");
                      delay(1000);
                      goto MasterPage;
                      }
                      else{
                      display(0,2,1,1,"Verification");
                      display(1,5,1,0,"failed");
                      delay(1000);
                      return; //goto wakeup;
                      }
            }else{
              display(0,2,1,1,"ID not found");
              delay(1000);
              return; //goto wakeup;
            }
      }
    }
  }
  }  
}
else{
  if(RF_Inp==1){RF_User="";RF_Pass="";RF_Inp=0;}
  if(sleep==1){return;}
  else if(to_login==1){goto Login;}
  display(0,0,0,1,"");
  lcd.playTone(5000,220,220);
  if(ID!="0000"/*master*/){block(ID);}
  delay(5000);
  return;  //to go to sleep mode - loop() restarts
}
}






// To display a given message at specified place in LCD, with backlight state and option to clear old contents in the screen as fucntion call parameters
void display(int x, int y, int bl,int cl,String msg){
lcd.cursorOff();
if(cl){
lcd.empty();
lcd.at(0,0,"                                                                                            ");
delay(10);
lcd.empty();
}
if(bl){
lcd.backLightOn();
delay(5);
}
else{
  lcd.backLightOff();
  delay(5);
}
lcd.at(x,y,msg);
}


//To validate a user with 3 allowed attempts
bool validate_usr(String pwd_act,String msg, String res_1, String res_2, int rfid){
int attmpt=1;
String pwd;
if(RF_Inp){
  if(RF_Pass == Password(RF_User)) return true;
}
while(attmpt<=3){
display(0,0,1,1,msg);
pwd = pass_inp(rfid);
mfrc522.PICC_HaltA();
mfrc522.PCD_StopCrypto1();
if(to_login){return false;} //To handle D key input
else if(sleep){return false;}
if(RF_Inp){
  switch(usrState(RF_User)){
    case 0: lcd.playTone(300, 160, 160);
            RF_User=" "; RF_Pass=" "; RF_Inp=0;
            continue;
            break;
    default: if(Password(RF_User)==RF_Pass){return true;}
            else{lcd.playTone(300, 160, 160);RF_User=" "; RF_Pass=" "; RF_Inp=0;continue;}
            break;
  }
}
if(pwd==pwd_act){return true;}
else{
  display(0,0,1,1,res_1);
  display(1,0,1,0,res_2);
  delay(2000);
  attmpt++;
  }
}
return false;
}


//Obtaining  User ID input
String usr_inp(bool rfid){
String a="";
for(int i=6;i<10;i++){
  lcd.pos(1,i);
  lcd.printCustomCharacter(7);
}
lcd.pos(1,6);
int pause=0;
while(true){
  char k=customKeypad.getKey();
  if(pause>=500){
    sleep=1;
    return " ";
  }
  //RFID LISTEN
  if(rfid==1){if(rfid_listen()){ return RF_User; }}
  if(k){
    pause=0;
  switch(k){
    case 'D':
      to_login=1;//exit to login screen
      return " ";//goto Login;
      break;
    case 'B':
      if(a.length()==4){
      return a;
      }
      break;
    case 'C':
      if(a.length()>0){
        a.remove((a.length()-1));
        lcd.scrollLeft();
        lcd.printCustomCharacter(7);
        lcd.scrollLeft();
      }
      break;
    case '*':
      break;
    case '#':
      break;
    case 'A':
      break;
    default:
    if(a.length()<4){
      a=a+k;
      lcd.print(k);}
      break;
   }
  }else{pause++;}
  delay(10);
}
return a;
}


//Obtaining Password input
String pass_inp(bool rfid){
String a="";
for(int i=5;i<11;i++){
  lcd.pos(1,i);
  lcd.printCustomCharacter(7);
}
lcd.pos(1,5);
int pause=0;
while(true){
  char k=customKeypad.getKey();
   if((pause>=1000 && !rfid) || (pause>=250 && rfid)){
    sleep=1;
    return " ";
  }
  //RFID LISTEN
   if(rfid){if(rfid_listen()){ return RF_User; }}
  if(k){
    pause=0;
  switch(k){
    case 'A': break;
    case 'B': if(a.length()==6){return a;}
              break;
    case 'C': if(a.length()>0){
              a.remove(a.length()-1);
              lcd.scrollLeft();
              lcd.printCustomCharacter(7);
              lcd.scrollLeft();
              }
              break;
    case 'D': to_login=1;
              return " ";//goto Login;
              break;
    default:  if(a.length()<6){
              a=a+k;
              lcd.print(k);
              delay(200);
              lcd.scrollLeft();
              lcd.print("*");
              }
              break;
  }
  }else{pause++;}
  delay(10);
}
}




//To check if userID is present in Database
//returns 0 if not found, 1 if unblocked, 2 if blocked
int usrState(String ID){
const char* fn = (ID.concat(".csv")).c_str();
if(!SD.exists(fn)){
  return 0;
}
else{
String pass="";
File db=SD.open(fn,FILE_READ);
if(db){
while(db.available()){
pass=pass.concat((char)(db.read()));  
}
db.close();
}
if(pass.endsWith("1")){
return 2;
}else{
return 1;
}
}
}


//To handle screens that require users to choose between options A or B (function parameters)
int choice(String A, String B){
int c=0;
display(0,1,1,1,A);
display(1,1,1,0,B);
lcd.pos(0,0);
lcd.printCustomCharacter(0);
while(true){
  char K = customKeypad.getKey();
  if(K){
    switch(K){
      case 'D': to_login=1;
                return 1; //goto Login;
                break;
      case 'B': return (c+1);
                break;
      case 'A': lcd.at(c,0," ");
                c=(c+1)%2;
                lcd.pos(c,0);
                lcd.printCustomCharacter(0);
      default: break;
    }
  }
}
}


//Adding a user credentials to the database  
void add(String ID,String pass){
const char* fn = (ID.concat(".csv")).c_str();
File db = SD.open(fn,FILE_WRITE);
db.print(pass.concat("0"));
db.close();
}


//Function to block a user
void block(String ID){
const char* fn = (ID.concat(".csv")).c_str();
//display(0,0,1,1,fn);
//delay(1000);
String pass="";
File db=SD.open(fn,FILE_READ);
if(db){
while(db.available()){
pass=pass.concat((char)(db.read()));  
}
db.close();
}
pass=pass.substring(0,6);
SD.remove(fn);
db=SD.open(fn,FILE_WRITE);
db.print(pass.concat("1"));
db.close();
}


//Function to unblock a user
void unblock(String ID){
const char* fn = (ID.concat(".csv")).c_str();
String pass="";
File db=SD.open(fn,FILE_READ);
if(db){
while(db.available()){
pass=pass.concat((char)(db.read()));  
}
db.close();
}
pass=pass.substring(0,6);
SD.remove(fn);
db=SD.open(fn,FILE_WRITE);
db.print(pass.concat("0"));
db.close();
}


//returns actual password stored in database
String Password(String ID){
if(ID=="0000"){return mp;};
const char* fn = (ID.concat(".csv")).c_str();
String pass="";
File db=SD.open(fn,FILE_READ);
if(db){
while(db.available()){
pass=pass.concat((char)(db.read()));  
}
db.close();
}
pass=pass.substring(0,6);
return pass;
}


//Deleting a user from database
void del(String ID){
const char* fn = (ID.concat(".csv")).c_str();
SD.remove(fn);
}


//Suggest new unused User ID
String newID(){
  int i=1;
  String a="";
while(true){
  char ID[4];
  sprintf(ID,"%04d",i);
  String a="";
  for(int i=0;i<4;i++){
    a=a+String(ID[i]);
  }
  a=a+".csv";
  if((SD.exists(a.c_str()))){
    i++;
  }
  else{return a.substring(0,4);}
}
}


//reads rfid tag data if detected and returns true, returns false if undetected or reads an unregistered card
bool rfid_listen(){
 
  if ( ! mfrc522.PICC_IsNewCardPresent()) { mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
    return false;   //card not present
  }
  if ( ! mfrc522.PICC_ReadCardSerial()) { mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
    return false;   //select one of the cards
  }
  byte buffer[18];  //byte array to store read data
  byte block = 5;  //data block number
  byte len = 18;   //number of bytes to read
  MFRC522::StatusCode status;
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));  //autenticating RFID. returns true if authenticated
  if (status != MFRC522::STATUS_OK) {
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    lcd.playTone(1000, 220, 220);    //when auhentication fails
    delay(100);
    return false;
  }
  status = mfrc522.MIFARE_Read(block, buffer, &len);
  if (status != MFRC522::STATUS_OK) {
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    lcd.playTone(1000, 220, 220);   // when read fails
    delay(100);
    return false;
  }


  RF_User="";
  RF_Pass="";
  //write read user and password to global variables
  for(int i=0;i<4;i++){RF_User+=char(buffer[i]);}
  for(int i=4;i<10;i++){RF_Pass+=char(buffer[i]);}
  RF_Inp=1;
   mfrc522.PICC_HaltA();
   mfrc522.PCD_StopCrypto1();
   delay(20);
   return true;
}


// Writes userID and Password data to a tag, returns true if writing is successful else returns false
bool rfid_write(String rfid_data){  
  display(0,3,1,1,"Place tag");
  display(1,3,1,0,"to register");
  int wait_dur=0;
  while(true){
    delay(20);
    if(wait_dur>=500){display(0,5,1,1,"No tag"); display(1,2,1,0,"was detected");delay(1000); return false;}
    char K = customKeypad.getKey();
    if(K=='D'){return false;}
    if ( ! mfrc522.PICC_IsNewCardPresent()) { wait_dur++;
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
     continue;
    }
    // Select one of the cards
    if ( ! mfrc522.PICC_ReadCardSerial()) { wait_dur++;
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    continue;
    }
    byte buffer1[16];
    for(int i=0;i<10;i++){buffer1[i]=byte((char)(rfid_data.charAt(i)));}
    for(int i=10;i<16;i++){buffer1[i]=byte((char)(" "));}
    byte block = 5;
    MFRC522::StatusCode status;
    status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK) {
    wait_dur =0;
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    display(0,3,1,1,"Try again");
    delay(500);
    display(0,3,1,1,"Place tag");
    display(1,3,1,0,"to register");
    continue;
    }
    status = mfrc522.MIFARE_Write(block, buffer1, 16);
    if (status != MFRC522::STATUS_OK) {
    wait_dur =0;
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    display(0,3,1,1,"Try again");
    delay(500);
    display(0,3,1,1,"Place tag");
    display(1,3,1,0,"to register");
    continue;
    }
    mfrc522.PICC_HaltA(); // Halt PICC
    mfrc522.PCD_StopCrypto1();  // Stop encryption on PCD
    return true;
  }
}


// reads ping signals and returns true if distance measured is less than 25cm
bool PersonNearby(){
int dur;
pinMode(ping, OUTPUT);
digitalWrite(ping, HIGH);
delay(10);
digitalWrite(ping, LOW);
pinMode(ping, INPUT);
dur = pulseIn(ping, HIGH);
digitalWrite(ping, LOW);
if((dur*0.017)<=25){return true;}
else{return false;}
}


// reads IR signals and returns false if object (door) is detected (closed), else returns true indicating that door is open
bool DoorOpen(){
if(digitalRead(op)==HIGH){return true;}
else{return false;}
}


// function to listen ping signals and keypad inputs in sleep mode
// turns off solenoid (locks door), turns off LCD and rings alarm if door is kept opened for more than a specified time duration
void asleep(){
  int open_dur=0;
  display(0,0,0,1,"                                                                    ");
  digitalWrite(sol,LOW);
  while(true){
    if(DoorOpen()){open_dur++; } //check if door is open    //if open, keep door unlocked and increment open_dur
    else{open_dur=0;}    //if not open, open_dur=0, lock door
    if(open_dur>=100){lcd.playTone(50, 220, 220);delay(10);}  //if open_dur > 5s, ring alarm for 1s
    if(PersonNearby()){sleep=0;return;} //check for ping signal
    char K = customKeypad.getKey(); //listen for keypad input
    if(K){
      sleep=0;
      return; //exit sleep mode
    }
    delay(10);
  }
}
