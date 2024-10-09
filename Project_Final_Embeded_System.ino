#include <Wire.h>
#include <Adafruit_LiquidCrystal.h>
#include <Keypad.h>
#include <Servo.h>

Adafruit_LiquidCrystal lcd(0);  // กำหนด LCD I2C address
Servo myservo;                  // สร้างตัวแปร servo
const int servoPin = 10;         // กำหนดขา signal ของ servo
const int piezoPin = 11;         // กำหนดขา piezo

const int greenLEDPin = 12;      // ขาของ LED สีเขียว
const int redLEDPin = 13;        // ขาของ LED สีแดง
const int motorPin = A1;         // ขา DC motor (ใช้ A1 แทนขา 9)
const int pirPin = A2;           // ขา PIR sensor
const int lightPin = A3;         // ขาของหลอดไฟผ่านทรานซิสเตอร์
const int tmp36Pin = A0;         // ขาของ TMP36 sensor

unsigned long lastMotionTime = 0;  // เวลาเมื่อมีการเคลื่อนไหวครั้งสุดท้าย
const unsigned long delayTime = 30000;  // กำหนดระยะเวลาไฟเปิด

// กำหนดรหัสผ่านที่ถูกต้อง
String password = "2024";
String inputPassword = "";

// กำหนดขา row และ column ของ keypad
const byte ROWS = 4;  // จำนวนแถวของ keypad
const byte COLS = 4;  // จำนวนคอลัมน์ของ keypad
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {9, 8, 7, 6};    // ขา row
byte colPins[COLS] = {5, 4, 3, 2};    // ขา column

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS); // สร้าง keypad object

void setup() {
  lcd.begin(16, 2);             // เริ่มการทำงานของ LCD 16x2
  lcd.setBacklight(HIGH);       // เปิด backlight
  lcd.setCursor(0, 0);
  lcd.print("Enter password:");

  myservo.attach(servoPin);     // เริ่มการทำงานของ servo
  myservo.write(0);             // เริ่มต้น servo ที่มุม 0 องศา (ประตูปิด)

  pinMode(piezoPin, OUTPUT);    // ตั้งค่า piezo เป็น output
  noTone(piezoPin);             // ปิดเสียง piezo

  pinMode(greenLEDPin, OUTPUT);  // ตั้งค่า LED สีเขียวเป็น output
  pinMode(redLEDPin, OUTPUT);    // ตั้งค่า LED สีแดงเป็น output
  pinMode(motorPin, OUTPUT);     // ตั้งค่ามอเตอร์และหลอดไฟเป็น output
  pinMode(pirPin, INPUT);        // ตั้งค่า PIR sensor เป็น input
  pinMode(lightPin, OUTPUT);     // ตั้งค่าหลอดไฟเป็น output
}

void loop() {
  // อ่านค่าอุณหภูมิจาก TMP36
  int sensorValue = analogRead(tmp36Pin);
  float voltage = sensorValue * (5.0 / 1023.0);
  float temperatureC = (voltage - 0.5) * 100.0;

  // ควบคุม DC motor ตามอุณหภูมิ
  if (temperatureC > 35) {
    digitalWrite(motorPin, HIGH);  // เปิดมอเตอร์ถ้าอุณหภูมิต่ำกว่า 35°C
  } else {
    digitalWrite(motorPin, LOW);   // ปิดมอเตอร์ถ้าอุณหภูมิสูงกว่า 35°C
  }

  // อ่านค่าจาก keypad
  char key = keypad.getKey();

  if (key) {
    lcd.setCursor(0, 1);
    if (key == '#') {
      // กด '#' เพื่อตรวจสอบรหัสผ่าน
      if (inputPassword == password) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Access Granted");
        digitalWrite(greenLEDPin, HIGH);  // เปิดไฟ LED สีเขียว
        digitalWrite(redLEDPin, LOW);     // ปิดไฟ LED สีแดง
        myservo.write(90);  // เปิดประตู (servo หมุนไปที่ 90 องศา)
        delay(5000);        // รอ 5 วินาที
        myservo.write(0);   // ปิดประตู (servo หมุนกลับไปที่ 0 องศา)
        digitalWrite(greenLEDPin, LOW);   // ปิดไฟ LED สีเขียว
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Enter password:");
      } else {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Access Denied");
        tone(piezoPin, 1000);  // ส่งเสียง piezo เพื่อแจ้งเตือน
        digitalWrite(redLEDPin, HIGH);   // เปิดไฟ LED สีแดง
        digitalWrite(greenLEDPin, LOW);  // ปิดไฟ LED สีเขียว
        delay(1000);           // รอ 1 วินาที
        noTone(piezoPin);      // ปิดเสียง piezo
        digitalWrite(redLEDPin, LOW);    // ปิดไฟ LED สีแดง
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Enter password:");
      }
      inputPassword = "";  // รีเซ็ตรหัสผ่านที่กรอก
    } else if (key == '*') {
      // กด '*' เพื่อลบรหัสที่กรอก
      inputPassword = "";
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Enter password:");
    } else {
      // กรอกรหัสผ่าน
      inputPassword += key;
      lcd.setCursor(0, 1);
      lcd.print(inputPassword);
    }
  }

  // เพิ่มการควบคุม PIR Sensor และ Light Bulb
  int pirState = digitalRead(pirPin);

  // ถ้ามีการเคลื่อนไหวจาก PIR sensor
  if (pirState == HIGH) {
    digitalWrite(lightPin, HIGH);  // เปิดไฟ
    lastMotionTime = millis();     // บันทึกเวลาที่มีการเคลื่อนไหว
  }

  // ถ้าไม่มีการเคลื่อนไหวเกิน 30 วินาที
  if (millis() - lastMotionTime > delayTime) {
    digitalWrite(lightPin, LOW);   // ปิดไฟ
  }

  delay(500);  // หน่วงเวลาเล็กน้อยเพื่อให้การอ่านค่าเสถียร
}
