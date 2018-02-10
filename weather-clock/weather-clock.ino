// Часы погодная станция
// Креатед бай voltNik (c) в 2018 году нашей эры
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <stdio.h>
#include <DS1302.h>
#include <Adafruit_BMP085.h>
#include <EEPROMex.h>
#include "tones.h" // файл нот, лежит в папке прошивки
//****************************
// пины кнопок управления
#define BTN_UP 8  // кнопка увеличения 
#define BTN_DOWN 9  // кнопка уменьшения
#define BTN_SET 10 // кнопка установки

// пины подключения модуля часов
#define kCePin 5  // RST
#define kIoPin 6  // DAT
#define kSclkPin 7  // CLK

#define FOTORES 14    // A0 пин подключения фоторезистора
#define LCD_LED 3  // ШИМ пин подключения подсветки LCD
#define BUZZER_PIN 12 // пин подключения спикера

#define BTN_PROTECT 100         // защита дребезга кнопки 
#define LCD_RENEW 250           // обновление экрана
//****************************
LiquidCrystal_I2C lcd(0x27,20,4);  // обычно китайские модули I2C для экрана имеют адрес 0x27 или 0x3F
DS1302 rtc(kCePin, kIoPin, kSclkPin);
Time t = rtc.time();
Adafruit_BMP085 bmp;
//****************************
int bright, btn_up_val, btn_down_val, btn_set_val, now_year, now_temp; 
float now_press;
byte now_disp, now_month, now_date, now_hour, now_min, now_sec, now_week_day, alarm_hour, alarm_min;
long now_millis, lcd_millis, time_millis, btn_up_millis, btn_down_millis, btn_set_millis, disp_millis, horn_millis;
boolean dot, blnk, alarm, horn, note;
byte set_time;
char sep;
int disp[4] = {20000,4000,4000,4000}; // тайминг работы экранов
//**************************** 
int melody[] = { NOTE_D7, NOTE_D8, NOTE_D7, NOTE_D8, NOTE_D7, NOTE_D8, NOTE_D7, NOTE_D8 }; // мелодия
int noteDurations[] = { 4, 4, 4, 4, 4, 4, 4, 4 };
//**************************** 
String week_day[7] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun", };
//**************************** 
byte stolb[8][8] =  {    // столбцы для графика (еще не используется)
{ B11111, B11111, B11111, B11111, B11111, B11111, B11111},
{ B00000, B11111, B11111, B11111, B11111, B11111, B11111},
{ B00000, B00000, B11111, B11111, B11111, B11111, B11111},
{ B00000, B00000, B00000, B11111, B11111, B11111, B11111},
{ B00000, B00000, B00000, B00000, B11111, B11111, B11111},
{ B00000, B00000, B00000, B00000, B00000, B11111, B11111},
{ B00000, B00000, B00000, B00000, B00000, B00000, B11111},
{ B00000, B00000, B00000, B00000, B00000, B00000, B00000}
};
// Eight programmable character definitions
byte custom[8][8] = {   // символы большого шрифта
{ B11111,B11111,B11111,B00000,B00000,B00000,B00000,B00000 },
{ B11100,B11110,B11111,B11111,B11111,B11111,B11111,B11111 },
{ B11111,B11111,B11111,B11111,B11111,B11111,B01111,B00111 },
{ B00000,B00000,B00000,B00000,B00000,B11111,B11111,B11111 },
{ B11111,B11111,B11111,B11111,B11111,B11111,B11110,B11100 },
{ B11111,B11111,B11111,B00000,B00000,B00000,B11111,B11111 },
{ B11111,B00000,B00000,B00000,B00000,B11111,B11111,B11111 },
{ B00111,B01111,B11111,B11111,B11111,B11111,B11111,B11111 }
};

// Characters, each with top and bottom half strings
// \nnn string encoding is octal, so:
// \010 = 8 decimal (8th programmable character)
// \024 = 20 decimal (space)
// \377 = 255 decimal (black square)

const char *bigChars[][2] = {   // символы из новых букв
{"\024\024\024", "\024\024\024"}, // Space
{"\377", "\007"}, // !
{"\005\005", "\024\024"}, // "
{"\004\377\004\377\004", "\001\377\001\377\001"}, // #
{"\010\377\006", "\007\377\005"}, // $
{"\001\024\004\001", "\004\001\024\004"}, // %
{"\010\006\002\024", "\003\007\002\004"}, // &
{"\005", "\024"}, // '
{"\010\001", "\003\004"}, // (
{"\001\002", "\004\005"}, // )
{"\001\004\004\001", "\004\001\001\004"}, // *
{"\004\377\004", "\001\377\001"}, // +
{"\024", "\005"}, // ,
{"\004\004\004", "\024\024\024"}, // -
{"\024", "\004"}, // .
{"\024\024\004\001", "\004\001\024\024"}, // /
{"\010\001\002", "\003\004\005"}, // 0
{"\001\002\024", "\024\377\024"}, // 1
{"\006\006\002", "\003\007\007"}, // 2
{"\006\006\002", "\007\007\005"}, // 3
{"\003\004\002", "\024\024\377"}, // 4
{"\377\006\006", "\007\007\005"}, // 5
{"\010\006\006", "\003\007\005"}, // 6
{"\001\001\002", "\024\010\024"}, // 7
{"\010\006\002", "\003\007\005"}, // 8
{"\010\006\002", "\024\024\377"}, // 9
{"\004", "\001"}, // :
{"\004", "\005"}, // ;
{"\024\004\001", "\001\001\004"}, // <
{"\004\004\004", "\001\001\001"}, // =
{"\001\004\024", "\004\001\001"}, // >
{"\001\006\002", "\024\007\024"}, // ?
{"\010\006\002", "\003\004\004"}, // @
{"\010\006\002", "\377\024\377"}, // A
{"\377\006\005", "\377\007\002"}, // B
{"\010\001\001", "\003\004\004"}, // C
{"\377\001\002", "\377\004\005"}, // D
{"\377\006\006", "\377\007\007"}, // E
{"\377\006\006", "\377\024\024"}, // F
{"\010\001\001", "\003\004\002"}, // G
{"\377\004\377", "\377\024\377"}, // H
{"\001\377\001", "\004\377\004"}, // I
{"\024\024\377", "\004\004\005"}, // J
{"\377\004\005", "\377\024\002"}, // K
{"\377\024\024", "\377\004\004"}, // L
{"\010\003\005\002", "\377\024\024\377"}, // M
{"\010\002\024\377", "\377\024\003\005"}, // N
{"\010\001\002", "\003\004\005"}, // 0/0
{"\377\006\002", "\377\024\024"}, // P
{"\010\001\002\024", "\003\004\377\004"}, // Q
{"\377\006\002", "\377\024\002"}, // R
{"\010\006\006", "\007\007\005"}, // S
{"\001\377\001", "\024\377\024"}, // T
{"\377\024\377", "\003\004\005"}, // U
{"\003\024\024\005", "\024\002\010\024"}, // V
{"\377\024\024\377", "\003\010\002\005"}, // W
{"\003\004\005", "\010\024\002"}, // X
{"\003\004\005", "\024\377\024"}, // Y
{"\001\006\005", "\010\007\004"}, // Z
{"\377\001", "\377\004"}, // [
{"\001\004\024\024", "\024\024\001\004"}, // Backslash
{"\001\377", "\004\377"}, // ]
{"\010\002", "\024\024"}, // ^
{"\024\024\024", "\004\004\004"}, // _ 
};

int writeBigChar(char ch, int x, int y) {
  const char *(*blocks)[2] = NULL; // Pointer to an array of two strings (character pointers)
  if (ch < ' ' || ch > '_') // If outside our table range, do nothing
  return 0;
  blocks = &bigChars[ch-' ']; // Look up the definition
  for (int half = 0; half <=1; half++) {
    int t = x; // Write out top or bottom string, byte at a time
    for (const char *cp = (*blocks)[half]; *cp; cp++) {
      lcd.setCursor(t, y+half); 
      lcd.write(*cp);
      t = (t+1) % 40; // Circular scroll buffer of 40 characters, loop back at 40
    }
    lcd.setCursor(t, y+half);
    lcd.write(' '); // Make space between letters, in case overwriting
  }
  return strlen((*blocks)[0]); // Return char width
}
//****************************
void setup()
{
  Serial.begin(9600);
  Serial.println("Weather Clock 1.0");
  
  //кнопки управления
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_SET, INPUT_PULLUP);

  pinMode(FOTORES, INPUT);
  pinMode(LCD_LED, OUTPUT);
  analogWrite(LCD_LED, 255);
  pinMode(BUZZER_PIN, OUTPUT);

  alarm_hour = EEPROM.readByte(0);
  alarm_min = EEPROM.readByte(1);
  alarm = EEPROM.readByte(2);

  if (!bmp.begin()) {   // если не подключен барометр - часы не включатся
    Serial.println("Could not find a valid BMP085 sensor, check wiring!");
  while (1) {}
  }

  rtc.writeProtect(false);
  rtc.halt(false);
  // первичная установка времени, если требуется
  //Time t(2017, 2, 6, 1, 39, 50, 1); // год-месяц-дата-час-минута-секунда-день.недели
  //rtc.time(t);
  
  lcd.init();
  lcd.backlight();
  lcd.clear();
  for (int i=0; i<8; i++) lcd.createChar(i+1, custom[i]);
  
  writeBigString("CLOCK", 0, 0);
  writeBigString("1.1", 5, 2);

  tone(BUZZER_PIN, NOTE_D7, 100);   // разово пищим при старте. проверка зуммера
  delay(1000);

  writeBigString("     ", 0, 0);
  writeBigString("   ", 5, 2);
  time_read();
}
//****************************
void(* resetFunc) (void) = 0;  // функция ресета. так надо.
//****************************
void loop()
{
  now_millis = millis();
  // считываем состояние кнопок
  btn_up_val = digitalRead(BTN_UP);
  btn_down_val = digitalRead(BTN_DOWN);
  btn_set_val = digitalRead(BTN_SET);
  
  // обработка нажатия кнопок с защитой от дребезга
  if ((btn_up_val == LOW) & (now_millis - btn_up_millis)> BTN_PROTECT) {  // обработка кнопки вверх
    horn = false;
    switch (set_time) {
      case 1:
        now_hour++;
        if (now_hour >= 24) now_hour=0;
        break;
      case 2:
        now_min++;
        if (now_min >= 60) now_min=0;
        break;
      case 3:
        now_sec = 0;
        set_time_now();
        disp_millis = now_millis;
        set_time = 0;
        break;
      case 4:
        alarm_hour++;
        if (alarm_hour >= 24) alarm_hour=0;
        break;
      case 5:
        alarm_min++;
        if (alarm_min >= 60) alarm_min=0;
        break;
      case 6:
        alarm = !alarm;
        break;
      case 7:
        now_year++;
        if (now_year >= 2100) now_year=2000;
        break;
      case 8:
        now_month++;
        if (now_month >= 13) now_month=1;
        break;
      case 9:
        now_date++;
        if (now_date >= 32) now_date=1;
        break;
      case 10:
        now_week_day++;
        if (now_week_day >= 7) now_week_day=0;
        break;
    }
    
    btn_up_millis = now_millis + 300;
  }
  if ((btn_down_val == LOW) & (now_millis - btn_down_millis)> BTN_PROTECT) {  // обработка кнопки вниз
    horn = false;
    switch (set_time) {
      case 1:
        now_hour--;
        if (now_hour == 255) now_hour=23;
        break;
      case 2:
        now_min--;
        if (now_min == 255) now_min=59;
        break;
      case 3:
        now_sec = 0;
        set_time_now();
        disp_millis = now_millis;
        set_time = 0;
        break;
      case 4:
        alarm_hour--;
        if (alarm_hour == 255) alarm_hour=23;
        break;
      case 5:
        alarm_min--;
        if (alarm_min == 255) alarm_min=59;
        break;
      case 6:
        alarm = !alarm;
        break;
      case 7:
        now_year--;
        if (now_year == 2000) now_year=2099;
        break;
      case 8:
        now_month--;
        if (now_month == 0) now_month=12;
        break;
      case 9:
        now_date--;
        if (now_date == 0) now_date=31;
        break;
      case 10:
        now_week_day--;
        if (now_week_day == 255) now_week_day=6;
        break;
    }
    btn_down_millis = now_millis + 300;
  }
  if ((btn_set_val == LOW) & (now_millis - btn_set_millis)> BTN_PROTECT) {  // обработка кнопки установки
    horn = false;
    if (now_disp!=0) {now_disp=0; lcd.clear(); }
    set_time = (set_time + 1) % 12;
    if (set_time == 11) { // выход из режима установки и запись времени
      set_time_now();
      set_time = 0;
      now_disp=0;
      disp_millis = now_millis;
    }
    btn_set_millis = now_millis + 300;
  }
  // обновление времени раз в секунду
  if (now_millis - time_millis > 1000) {
    dot = !dot;
    if (dot) {sep = ':';} else {sep = '.';};
    if (set_time == 0) {
      time_read();
    }  
    set_lcd_led();
    if ((now_hour == alarm_hour)and(now_min == alarm_min)and(now_sec==0)and(alarm)) { horn = true;} // проверка будильника
    if ((now_hour != alarm_hour)or(now_min != alarm_min)) { horn = false;};  // отключение будильника через 1 минуту

    if (millis()>4000000000) {resetFunc();}; // проверка переполения millis и сброс раз в 46 суток. максимально возможно значение 4294967295, это около 50 суток.
    time_millis = now_millis;
  } 

  if ((horn)and(now_millis - horn_millis > 250)) { // будильник
    if (note) {
      noTone(BUZZER_PIN);
      tone(BUZZER_PIN, NOTE_D8, 250); 
      analogWrite(LCD_LED, 255);
    } else {
      noTone(BUZZER_PIN);
      tone(BUZZER_PIN, NOTE_D7, 250); 
      analogWrite(LCD_LED, 0);
    }   
    note = !note; 
    horn_millis = now_millis;
  }
    
  if ((now_millis - disp_millis  > disp[now_disp])and(set_time==0)) {  // смена экранов по таймингу
    now_disp = (now_disp + 1) % 5;
    lcd.clear();
    disp_millis = now_millis;
  };
  if (now_millis - lcd_millis > LCD_RENEW) {  // обновление экрана
   print_lcd();
   lcd_millis = now_millis;
  } 
}
//****************************
void print_lcd(void) { // отрисовка экрана
 char time_str[6], sec_str[3], date_str[15], pres_str[21], davl_str[4], temp_str[3], alarm_str[6], set_str;

 snprintf(time_str, sizeof(time_str), "%02d%c%02d", now_hour, sep, now_min);
 snprintf(sec_str, sizeof(sec_str), "%02d", now_sec);
 snprintf(date_str, sizeof(date_str), "%04d-%02d-%02d %s", now_year, now_month, now_date, week_day[now_week_day].c_str() );
 dtostrf(now_press, 3, 0, davl_str);
 snprintf(temp_str, sizeof(temp_str), "%02d", now_temp);
 snprintf(pres_str, sizeof(pres_str), "T:%02dC P:%smm", now_temp, davl_str );
 snprintf(alarm_str, sizeof(alarm_str), "%02d:%02d", alarm_hour, alarm_min);
 if (alarm) {set_str='+';} else {set_str='-';};

 if ((set_time!=0)and(blnk)and(now_disp==0)) {  // мигание при установке времени
  switch (set_time) {
    case 1:
      time_str[0]=' '; time_str[1]=' ';
      break;
    case 2:
      time_str[3]=' '; time_str[4]=' ';
      break;
    case 3:
      sec_str[0]=' '; sec_str[1]=' ';
      break;
    case 4:
      alarm_str[0]=' '; alarm_str[1]=' ';
      break;
    case 5:
      alarm_str[3]=' '; alarm_str[4]=' ';
      break;
    case 6:
      set_str=' ';
      break;
    case 7:
      date_str[0]=' '; date_str[1]=' '; date_str[2]=' '; date_str[3]=' ';
      break;
    case 8:
      date_str[5]=' '; date_str[6]=' ';
      break;
    case 9:
      date_str[8]=' '; date_str[9]=' ';
      break;
    case 10:
      date_str[11]=' '; date_str[12]=' '; date_str[13]=' ';
      break;
  }
 }
 blnk = !blnk;

 switch (now_disp) {
  case 0:
   lcd.setCursor(0,0);
   lcd.print(date_str);
   lcd.setCursor(18,1);
   lcd.print(sec_str);
   writeBigString(time_str, 0, 1);
   lcd.setCursor(0,3);
   lcd.print(alarm_str);
   lcd.print(set_str);
   lcd.setCursor(7,3);
   lcd.print(pres_str);
   break;
  case 1:
   writeBigString(davl_str, 1, 0); writeBigString("MM", 9, 2);
   break;
  case 2:
   writeBigString(time_str, 0, 0);
   writeBigString(time_str, 2, 2);
   break;
  case 3:
   writeBigString(temp_str, 4, 1); writeBigString("C", 13, 1);
   break;
 }
}
//****************************
void writeBigString(char *str, int x, int y) { // пишем большие буквы
  char c;
  while ((c = *str++))
  x += writeBigChar(c, x, y) + 1;
}
//****************************
void time_read() { // читаем время из модуля и записываем значения в переменные для работы
  t = rtc.time();
  now_year = t.yr;
  now_month = t.mon;
  now_date = t.date; 
  now_hour = t.hr;
  now_min = t.min;
  now_sec = t.sec;
  now_week_day = t.day;
  now_temp = bmp.readTemperature();
  now_press = bmp.readPressure()/133.3;
}
//****************************
void set_lcd_led() { // установка уровня яркости подстветки экрана
  bright = map(analogRead(FOTORES), 320, 1024, 0, 5);
  if (bright > 3) bright = 5;
  if (bright < 2) bright = 1;
  analogWrite(LCD_LED, bright*51);
}
//****************************
void set_time_now() { // установка времени
  Time tt(now_year, now_month, now_date, now_hour, now_min, now_sec, now_week_day);
  rtc.time(tt);
  EEPROM.writeByte(0, alarm_hour);
  EEPROM.writeByte(1, alarm_min);
  EEPROM.writeByte(2, alarm);
}
//****************************
