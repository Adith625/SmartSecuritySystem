#include <SPI.h>
#include <MFRC522.h>
#define unlock          7
#define lock            4
#define RST_PIN         9          
#define SS_PIN          10
#define buzzer          5
#define ir              8
#define enA             6
MFRC522 mfrc522(SS_PIN, RST_PIN);               // Create MFRC522 instance

bool verify();                                 //define functions
void set_password();

byte password[4][16] = {
{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15},       //list of defalut passwords
{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15},
{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15},
{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15}
};

byte uid[4][4] ={
{105,44,44,1},                                 //2D array of UIDs of tags
{231,101,52,38},
{73,214,42,164},
{25,101,44,1}
};

int tag_id = -1;                                //set current tag id to -1
int time   =  3500;                             // time in ms to run the motor
int pwm    =  225 ;                             //analog write value

MFRC522::MIFARE_Key key; 
MFRC522::StatusCode status;


void setup() {
  Serial.begin(9600);                                          
  pinMode(buzzer,OUTPUT);
  pinMode(lock,OUTPUT);
  pinMode(unlock,OUTPUT);
  pinMode(ir,INPUT);
  pinMode(enA,OUTPUT);
  digitalWrite(unlock, LOW);
  digitalWrite(lock, LOW);
  digitalWrite(enA, LOW);
  SPI.begin();                                                  
  mfrc522.PCD_Init();
  for (byte i = 0; i < 6; i++) {          // Prepare the security key for the read and write functions.
    key.keyByte[i] = 0xFF;
  }
}


void loop() {
   // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if (! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  byte current_uid[4];
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    current_uid[i]= mfrc522.uid.uidByte[i];
  }

  int val=0;
  for(int i =0;i<4;i++) {
    for(int j=0;j<4;j++) {
      if(current_uid[j]==uid[i][j]){
        val++;
      }
    }

    if(val==4) {
      tag_id=i;
      break;
    }
    val=0;
  }

  if(tag_id!=-1) {
    if(verify()==true) { 
      Serial.write(tag_id+'0');
      Serial.write(10);
      set_password();
      tag_id = -1;
      unlock_door();
    }
    else {
        for (int i = 0; i < 5; i++){             //Wrong password
          tone(buzzer, 400, 250);delay(400);noTone(buzzer);
        }
      }
    }
    else{
      tone(buzzer,5000,5000);                  //Wron UID
    }

  delay(1500); //change value if you want to read cards faster

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}


bool verify() {
  byte read_buffer[18];// buffer for storing read password
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 7, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    return false;
  }
  byte buffer_size = 18;
  status = mfrc522.MIFARE_Read(4,read_buffer, &buffer_size);//Read data from block4 and store it in read_buffer
  if (status != MFRC522::STATUS_OK) {
    return false;
  }
  int check = 0;
  for (int c = 0; c < 16; c++) {               //check if read password matches to stored copy
    if (read_buffer[c] == password[tag_id][c]) {
      check++;                                 //increment for each correct match
    }
  }
  if (check == 16) {
    return true;
  }  
  else {
    return false;
  }
}


void set_password() {
  byte write_buffer[16];
  for (int i = 0; i < 16; i++) {
    write_buffer[i] = random(31, 126); //genarate random number and convert it into character
    password[tag_id][i] = write_buffer[i];    //store new password
  }

  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 7, &key, &(mfrc522.uid));
  status = mfrc522.MIFARE_Write(4, write_buffer, 16);
}


void unlock_door() {
  analogWrite(enA, pwm);
  digitalWrite(unlock, HIGH);
  delay (time);              //
  digitalWrite(unlock, LOW);
  delay(1000);
  while(digitalRead(ir)==LOW) {
    delay(1500);
    tone(buzzer,1000,200);
  }
  lock_door();
}


void lock_door() {
  delay(1500);
  while(digitalRead(ir)==HIGH) {
    delay(1500);
    tone(buzzer,1500,300);
  }
  delay(500);
  if (digitalRead(ir)==LOW) {
    digitalWrite(lock, HIGH);
    delay(time);
    digitalWrite(lock, LOW);
  }
  digitalWrite(enA, LOW);
}

