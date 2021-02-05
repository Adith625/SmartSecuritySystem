//This program resets the password.
//Data present in sector1 block4 will be modified.
#include <SPI.h>
#include <MFRC522.h>
#define buzzer          5           //buzzer pin
#define RST_PIN         9           // Configurable, see typical pin layout above
#define SS_PIN          10          // Configurable, see typical pin layout above

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance
MFRC522::MIFARE_Key key;
MFRC522::StatusCode status;
void setup() {
  Serial.begin(9600);        // Initialize serial communications with the PC
  SPI.begin();               // Init SPI bus
  mfrc522.PCD_Init();        // Init MFRC522 card
  for (byte i = 0; i < 6; i++) {
  key.keyByte[i] = 0xFF;     // Prepare the security key for the read and write functions.
  }
  Serial.println(F("Ready to reset passwords on RFID tags"));
}

void loop() {

  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  Serial.println(F("Card UID:"));    //Dump UID
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(i == 0 ? "{" : " ");
    Serial.print(mfrc522.uid.uidByte[i]);
    Serial.print(i < 3 ? "," : "}\n");
  }

  byte buffer[16]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};    //define default password
  Serial.println(F("Authenticating using key A..."));
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 7, &key, &(mfrc522.uid));//Authenticate sector 1
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  status = mfrc522.MIFARE_Write(4, buffer, 16);     // Write block
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Write() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  else {
    Serial.println(F("MIFARE_Write() success"));
    tone(buzzer,2000,1000);
  }

  mfrc522.PICC_HaltA(); // Halt PICC
  mfrc522.PCD_StopCrypto1();  // Stop encryption on PCD

}
