#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>

#define DS3231_I2C_ADDRESS 104
DFRobotDFPlayerMini MP3Player;
LiquidCrystal_I2C lcd(0x27,20,4);
SoftwareSerial MP3Module(2, 3);
  
byte seconds, minutes, hours, day, date, month, year;
char weekDay[4];
char bseconds[2], bminutes[2], bhours[2], bdate[2], bmonth[2], byear[2]; // 서식 지정 날짜
char nseconds[2], nminutes[2], nhours[2], ndate[2], nmonth[2], nyear[2]; // 현재시간 저장
 
byte tMSB, tLSB;
float temp3231;

int flag = 0;
boolean toggle_state = 0;

void setup(){
  Serial.begin(9600);
  Wire.begin();
  lcd.init();
  pinMode(9,OUTPUT);
  pinMode(8,OUTPUT);
  pinMode(7,INPUT);
  pinMode(6,INPUT);
  
  MP3Module.begin(9600);
  if (!MP3Player.begin(MP3Module)) { 
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    while (true);
  }
  delay(1);
  MP3Player.volume(15);  
}
 
void loop(){
  watchConsole();
  get3231Date();
  
  // 문자열 서식 지정
  sprintf(bseconds, "%02d", seconds);
  sprintf(bminutes, "%02d", minutes);
  sprintf(bhours, "%02d", hours);
  sprintf(bdate, "%02d", date);
  sprintf(bmonth, "%02d", month);
  sprintf(byear, "%02d", year);

  
  int B_yellow = digitalRead(7);
  if(B_yellow == HIGH){
    digitalWrite(9,HIGH);
    for(int i=0; i<2; i++){
     nseconds[i] = bseconds[i];
     nminutes[i] = bminutes[i];
     nhours[i] = bhours[i];
     ndate[i] = bdate[i];
     nmonth[i] = bmonth[i];
    }
    MP3Player.play(1);  // 0001.mp3를 재생합니다.
    //delay (30000);
  }
  else{
    digitalWrite(9, LOW);
  }

  int B_black = digitalRead(6);
  if(B_black == HIGH){
    if(flag == 0){
      flag = 1;
      toggle_state = !toggle_state;
      lcd.clear();
    }
  }
  else{
    if(flag==1){
      flag = 0;
      lcd.clear();
    }
  }
  if(flag == 1){
    lcd.backlight();
    // 마지막 입력 시간 lcd 출력
    lcd.setCursor(0,0);
    lcd.print("[Last Time]");

    lcd.setCursor(11, 0);
    lcd.print(nmonth);
    lcd.setCursor(13,0);
    lcd.print("/");
    lcd.setCursor(14, 0);
    lcd.print(ndate);
    
    lcd.setCursor(4,1);
    lcd.print(nhours);
    lcd.setCursor(6,1);
    lcd.print(":");
    lcd.setCursor(7,1);
    lcd.print(nminutes);
    lcd.setCursor(9,1);
    lcd.print(":");
    lcd.setCursor(10,1);
    lcd.print(nseconds);
    lcd.setCursor(12,1);
    lcd.print("    ");
    MP3Player.pause();
    delay(1000);
  }
  else{
    lcd.noBacklight();
    // 현재 시간 lcd 출력
    lcd.setCursor(0,0);
    lcd.print(weekDay);
    lcd.setCursor(3,0);
    lcd.print(", 20");
    lcd.setCursor(7,0);
    lcd.print(byear);
    lcd.setCursor(9,0);
    lcd.print("/");
    lcd.setCursor(10, 0);
    lcd.print(bmonth);
    lcd.setCursor(12,0);
    lcd.print("/");
    lcd.setCursor(13, 0);
    lcd.print(bdate);
    lcd.setCursor(4,1);
    lcd.print(bhours);
    lcd.setCursor(6,1);
    lcd.print(":");
    lcd.setCursor(7,1);
    lcd.print(bminutes);
    lcd.setCursor(9,1);
    lcd.print(":");
    lcd.setCursor(10,1);
    lcd.print(bseconds);
  
    delay(1000);
  }
}
 
// 10진수를 2진화 10진수인 BCD 로 변환 (Binary Coded Decimal)
byte decToBcd(byte val)
{
  return ( (val/10*16) + (val%10) );
}
 
void watchConsole()
{
  if (Serial.available()) {      // Look for char in serial queue and process if found
    if (Serial.read() == 84) {   //If command = "T" Set Date
      set3231Date();
      get3231Date();
      Serial.println(" ");
    }
  }
}
 
//시간설정
// T(설정명령) + 년(00~99) + 월(01~12) + 일(01~31) + 시(00~23) + 분(00~59) + 초(00~59) + 요일(1~7, 일1 월2 화3 수4 목5 금6 토7)
// 예: T1605091300002 (2016년 5월 9일 13시 00분 00초 월요일)
void set3231Date()
{
  year    = (byte) ((Serial.read() - 48) *10 +  (Serial.read() - 48));
  month   = (byte) ((Serial.read() - 48) *10 +  (Serial.read() - 48));
  date    = (byte) ((Serial.read() - 48) *10 +  (Serial.read() - 48));
  hours   = (byte) ((Serial.read() - 48) *10 +  (Serial.read() - 48));
  minutes = (byte) ((Serial.read() - 48) *10 +  (Serial.read() - 48));
  seconds = (byte) ((Serial.read() - 48) * 10 + (Serial.read() - 48));
  day     = (byte) (Serial.read() - 48);
 
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0x00);
  Wire.write(decToBcd(seconds));
  Wire.write(decToBcd(minutes));
  Wire.write(decToBcd(hours));
  Wire.write(decToBcd(day));
  Wire.write(decToBcd(date));
  Wire.write(decToBcd(month));
  Wire.write(decToBcd(year));
  Wire.endTransmission();
}
 
 
void get3231Date()
{
  // send request to receive data starting at register 0
  Wire.beginTransmission(DS3231_I2C_ADDRESS); // 104 is DS3231 device address
  Wire.write(0x00); // start at register 0
  Wire.endTransmission();
  Wire.requestFrom(DS3231_I2C_ADDRESS, 7); // request seven bytes
 
  if(Wire.available()) {
    seconds = Wire.read(); // get seconds
    minutes = Wire.read(); // get minutes
    hours   = Wire.read();   // get hours
    day     = Wire.read();
    date    = Wire.read();
    month   = Wire.read(); //temp month
    year    = Wire.read();
       
    seconds = (((seconds & B11110000)>>4)*10 + (seconds & B00001111)); // convert BCD to decimal
    minutes = (((minutes & B11110000)>>4)*10 + (minutes & B00001111)); // convert BCD to decimal
    hours   = (((hours & B00110000)>>4)*10 + (hours & B00001111)); // convert BCD to decimal (assume 24 hour mode)
    day     = (day & B00000111); // 1-7
    date    = (((date & B00110000)>>4)*10 + (date & B00001111)); // 1-31
    month   = (((month & B00010000)>>4)*10 + (month & B00001111)); //msb7 is century overflow
    year    = (((year & B11110000)>>4)*10 + (year & B00001111));
  }
  else {
    //oh noes, no data!
  }
 
  switch (day) {
    case 1:
      strcpy(weekDay, "Sun");
      break;
    case 2:
      strcpy(weekDay, "Mon");
      break;
    case 3:
      strcpy(weekDay, "Tue");
      break;
    case 4:
      strcpy(weekDay, "Wed");
      break;
    case 5:
      strcpy(weekDay, "Thu");
      break;
    case 6:
      strcpy(weekDay, "Fri");
      break;
    case 7:
      strcpy(weekDay, "Sat");
      break;
  }
}
 
float get3231Temp()
{
  //temp registers (11h-12h) get updated automatically every 64s
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0x11);
  Wire.endTransmission();
  Wire.requestFrom(DS3231_I2C_ADDRESS, 2);
 
  if(Wire.available()) {
    tMSB = Wire.read(); //2's complement int portion
    tLSB = Wire.read(); //fraction portion
   
    temp3231 = (tMSB & B01111111); //do 2's math on Tmsb
    temp3231 += ( (tLSB >> 6) * 0.25 ); //only care about bits 7 & 8
  }
  else {
    //error! no data!
  }
  return temp3231;
}
