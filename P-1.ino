#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <MFRC522.h>
#include <SPI.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

#define SS_PIN 5  
#define RST_PIN 0

MFRC522 mfrc522(SS_PIN, RST_PIN);

char auth[]="J_pUhP89E_us4d3rTyuH1gvd4x0MInBm";
char ssid[]="M";
char pass[]="12345678";
const int buzzerPin = 26;

// Define the LCD module parameters
const int lcdColumns = 16;  // Number of columns in the LCD display
const int lcdRows = 2;      // Number of rows in the LCD display

// Initialize the LCD module with its I2C address
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);

// Pin configuration for IR sensor
const int irSensorPin = 34;

// Pin configuration for servo motor
const int servoPin = 13;

// Pin configuration for ultrasonic sensor
const int trigPin = 2;
const int echoPin = 4;
String c1="0382152f";
String c2="8becd322";
String c3="2195bf26";
String c4="aa60d984";
String cardUID="";
String test="";

// Servo motor object
Servo servo;

// Threshold for lid opening/closing
const int Threshold = 3000;

void setup() {
  Serial.begin(115200);
 // Initialize serial communication

  SPI.begin();  
  Blynk.begin(auth,ssid,pass);      // Initialize SPI bus
  mfrc522.PCD_Init();
  mfrc522.PCD_SetAntennaGain(mfrc522.RxGain_max);
  mfrc522.PCD_DumpVersionToSerial(); // Initialize MFRC522 card

  // Initialize serial communication
  pinMode(buzzerPin, OUTPUT);
  lcd.begin(lcdColumns, lcdRows);
  lcd.init();

  // Turn on the backlight (if available)
  lcd.backlight();

  // Display a welcome message
  lcd.setCursor(2, 0);
  lcd.print("WELCOME TO");
  lcd.setCursor(2, 1);
  lcd.print("SMART BIN");
  delay(2000);

  // IR sensor pin setup
  pinMode(irSensorPin, INPUT);

  // Servo motor pin setup
  servo.attach(servoPin);
  Blynk.virtualWrite(V0,0);
  servo.write(0); // Close the lid initially

  // Ultrasonic sensor pin setup
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
}

void loop() {
  Blynk.run();
  int sensorValue = digitalRead(irSensorPin);
  Serial.println(sensorValue);
  float distance = measureDistance();
  Serial.println(distance);
  Blynk.virtualWrite(V2,distance);//Distance
  distance=(distance/22.7)*100;
  Blynk.virtualWrite(V1,distance);//Bin Level
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    String cardUID="";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      cardUID += String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
      cardUID += String(mfrc522.uid.uidByte[i], HEX);
    }
    if(c1.equals(cardUID) || c2.equals(cardUID) || c3.equals(cardUID) || c4.equals(cardUID)){
      Blynk.virtualWrite(V4,"Card Accepted");//RFID
      lcd.setCursor(1, 0);
      lcd.print("Card Accepted");
      lcd.setCursor(0,1);
      lcd.print("Work In Progress");
      openLid();
      RFIDreader(cardUID);
      Blynk.virtualWrite(V4,"Work Completed");//RFID
  }
  Serial.println(cardUID);
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
  }
  

  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("SMART BIN");
  if(distance<15 && distance!=0){
    Blynk.virtualWrite(V6,"BIN IS FULL");//BIN EMPTY STATUS
    Blynk.virtualWrite(V7,"BIN IS FULL");//BIN FULL STATUS
  lcd.setCursor(3, 0);
  lcd.print("Bin is Full");
  }
  else{
    String Data="BIN LEVEL:";
    Data+=int(distance);
    Data+="% IS EMPTY";
    Blynk.virtualWrite(V6,Data);
    String Data1="BIN LEVEL:";
    Data1+=100-int(distance);
    Data1+="% IS FULL";
    Blynk.virtualWrite(V7,Data1);
  lcd.setCursor(0, 1);
  lcd.print("Bin Level:");
  lcd.setCursor(10, 1);
  lcd.print(int(distance));
  lcd.print("%");
  digitalWrite(buzzerPin, LOW);
  }
  
  if (sensorValue ==1 && distance>15) {
    openLid();
  }else if(sensorValue ==1 && distance<15) {
    Blynk.virtualWrite(V3,HIGH);//BUZZER
    digitalWrite(buzzerPin, HIGH);
    closeLid();
  }
  if(sensorValue ==0){
    Blynk.virtualWrite(V3,LOW);
    digitalWrite(buzzerPin, LOW);
    closeLid();
  }
  // Measure bin level using ultrasonic sensor

  // Perform actions based on bin level
}

void openLid() {
  Blynk.virtualWrite(V5,"BIN IS OPENED");
  Serial.println("Opening the lid...");
  Blynk.virtualWrite(V0,120);
  servo.write(120);
  delay(1000); // Open the lid // Delay for the lid to fully open
}

void closeLid() {
  Blynk.virtualWrite(V5,"BIN IS CLOSED");
  Serial.println("Closing the lid...");
  Blynk.virtualWrite(V0,0);
  servo.write(0); // Close the lid
  delay(1000); // Delay for the lid to fully close
}

float measureDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH);
  float distance = duration * 0.034 / 2;

  return distance;
}
void RFIDreader(String c){
  test="";
  while(1){
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    String test="";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      test += String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
      test += String(mfrc522.uid.uidByte[i], HEX);
    }
    if(c.equals(test))
    {
      break;
    }
    mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
  }
  }
}