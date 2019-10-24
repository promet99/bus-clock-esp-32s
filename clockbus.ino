#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#define USE_SERIAL Serial

WiFiMulti wifiMulti;
LiquidCrystal_I2C lcd(0x27, 16, 2);

//Blynk
char auth[] = "??????????";  
char ssid[] = "??????????";
char pass[] = "??????????";

int threshold = 50;
int bttimer = 10;

bool mode_on = true;
bool was_on = false;
bool wifion = false;
bool apion = false;
bool alarmon = false;   // alarm

int sectocome = 100; //time left

bool button1= false;
bool button2= false;
bool button3= false;

int thebus = 0;      //

int apicounter = 0;   
int oncounter = 0;   

int ss1 = 0;   
int ss2 = 0;

uint8_t bell[8] = {0x4, 0xe, 0xe, 0xe, 0x1f, 0x0, 0x4};
uint8_t wifionlcd[8] = {0x00,0x00,0x00,0x01,0x01,0x05,0x05,0x15};
uint8_t wifiofflcd[8] ={0x14,0x08,0x14,0x01,0x01,0x05,0x05,0x15};

uint8_t boon[8] ={0x09,0x0F,0x0F,0x00,0x0F,0x02,0x08,0x0F};
uint8_t cho[8] ={0x04,0x1F,0x04,0x0A,0x11,0x04,0x04,0x1F};



int count=0;


//using delay() inside interrupt function causes kernel panic. it works the same way.
void delay_(int ms){
  while(count!=ms){
  delayMicroseconds(1000);
  count++;}
  count=0; 
}


//Same as above, but Blynk requires constant run of Blynk.run() function for it to work.
//Therefore, while delay() Blynk.run() should be runned
void Blynk_Delay(int milli){
  int end_time = millis() + milli;
  while (millis() < end_time) {
    if (Blynk.connected()) {
      Blynk.run();
    }
    yield();
  }
}

//This is function that is called when interrupt happens (which is when the button is pushed for longer than 100ms)
void f_bt1() {   
  bttimer=10 ;
  delay_(50);
  while (touchRead(T3) < threshold) {
    bttimer=bttimer - 1;
    delay_(10);
    if(bttimer < 0) {
      button1=true ;
      return ;
    }
  }
  button1=false;
  return;
}

void f_bt2() {   
  bttimer=10;
  delay_(50);
  while (touchRead(T7) < threshold) {
    bttimer=bttimer - 1;
    delay_(10);
    if(bttimer < 0) {
      button2=true;
      return;
    }
  }
    button2=false;
    return;
}

void f_bt3() {   
  bttimer=10;
  delay_(50);
  while (touchRead(T9) < threshold) {
    bttimer=bttimer - 1;
    delay_(10);
    if(bttimer < 0) {
      button3=true;
     return;
    }
  }
  button3=false;
  return;
}


//api 
void runapi() {
  if((wifiMulti.run() == WL_CONNECTED)) {
    wifion = true;
    HTTPClient http;
    String bus_1 = "";// api url
    String bus_2 = "";// api url
    String bus_3 = "";// api url
      
    switch (thebus) {       
    case 0:    
      http.begin(bus_1); // start connection & send HTTP header
    case 1:    
      http.begin(bus_2); 
    break;
    default:   
      http.begin(bus_3); 
    break;
    }
    int httpCode = http.GET();
    if(httpCode > 0) { // httpCode will be negative on error.  HTTP normal
      USE_SERIAL.printf("[HTTP] GET... code: %d\n", httpCode);
      if(httpCode == HTTP_CODE_OK) {                           // file found at server
        apion = true;  
        String payload = http.getString();
        ss1 = payload.indexOf("<exps1>");
        ss1 = ss1 + 7;
        ss2 = payload.indexOf("</exps1>");
        String sectocomes = payload.substring(ss1, ss2);
        sectocome = sectocomes.toInt();
        ss1 = payload.indexOf("<mkTm>");
        ss1 = ss1 + 6;
        ss2 = payload.indexOf("</mkTm>");
        String mkTm = payload.substring(ss1, ss2);
        USE_SERIAL.println(sectocome+" "+mkTm+" "+httpCode);
      }
    } else {
      apion = false; 
      USE_SERIAL.println("sdf");
    }
      http.end();
  } else {
    wifion = false;
    USE_SERIAL.println("wf!");
  }
}

//lcd writing function
void refreshlcd() {
  lcd.clear(); //clean slate

  switch (thebus) {    //name of bus
  case 0:  
    lcd.print("0 ");
  break;
  case 1:
    lcd.print(" 5  ");
  break;
  default:
    lcd.print(" 5  ");
  break;
  }
  lcd.print("| "); // for spacing
  if(alarmon == true) {      //shows alarm setting  (on/off)
    lcd.write(4);
    lcd.print(" ");
  } else {
    lcd.print("Z ");
  }
  if(apion == true) {    // shows wether api works of not
    lcd.print("A ");
  } else {
    lcd.print("X ");
  }
  if(wifion == true) {       // shows wether wifi works of not
    lcd.write(5);
  } else {
    lcd.write(6);
  }

  lcd.setCursor(0, 1); //move cursor to next line

  int timemin = sectocome / 60 ;
  int timesec = sectocome % 60 ;

  if(apion == true) {    // 
    lcd.print(timemin);  //
////min
    lcd.write(3);
    lcd.print(" "); 
    lcd.print(timesec);  
    lcd.write(7);
  } else {
    lcd.print("Loading..."); // 
  }
 
  if(sectocome > 1) {
    sectocome = sectocome -1 ;  //
  }

  Blynk.virtualWrite(V5, timemin);  
  Blynk.virtualWrite(V6, timesec);
}


//  blinks lcd and sends Blynk notification
void alarm() {
  switch (thebus) {       //
    case 0:    // 
      Blynk.notify("1 arrive soon!");
    case 1:    // 
      Blynk.notify("2 arrive soon!");
    break;
    default:    //
      Blynk.notify("3 arrive soon!");
    break;
  }
  lcd.noBacklight();
  Blynk_Delay(495);
  lcd.backlight();
  Blynk_Delay(495);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void setup() {
  Blynk.begin(auth, ssid, pass);
  Serial.begin(115200);
  lcd.begin();
  lcd.home();
  lcd.clear();
  lcd.noBacklight();
  touchAttachInterrupt(T3, f_bt1, threshold);   //15
  touchAttachInterrupt(T7, f_bt2, threshold);   //27
  touchAttachInterrupt(T9, f_bt3, threshold);   //32

  lcd.createChar(5, wifionlcd);
  lcd.createChar(6, wifiofflcd);

  lcd.createChar(3, boon);
  lcd.createChar(7, cho);

  wifiMulti.addAP("upjoon", "123456789");
}

void loop() {
Blynk.run();
if(mode_on == true) {
  if(was_on == false) {
    was_on = !was_on;
    lcd.clear();
    lcd.backlight();
    lcd.print("Boot!");
    Blynk_Delay(1000);
  }
  if(button1 == true) {                
    USE_SERIAL.println("1"); 
    button1 = false;
    mode_on = false;
    was_on = !was_on;
    lcd.clear();
    lcd.noBacklight();
  }
  lcd.backlight();  
    if(button2 == true) {                // toggle bus
      USE_SERIAL.println("2"); 
      button2=false;
      if(thebus==2){                   //=>api 
        thebus = 0;
      }  else {
        thebus = thebus +1;
      }
    runapi();
    }
    if(button3 == true) {                // toggle alarm
      USE_SERIAL.println("3"); 
      button3 = false;
      alarmon = !alarmon;
    }
    refreshlcd();

    if( apicounter == 8) {
      apicounter = 0;
      runapi(); // every 8 sec
    } else {
      apicounter = apicounter + 1;  // count
    }

    if(sectocome < 60 && alarmon == true && apion==true) {   // alarm 
      alarm();    //    runs alarm for 1sec
    } else {
      Blynk_Delay(990);
    }

    // turns off in 8 min, 
    if( oncounter == 480) {
      oncounter = 0;
      mode_on = false;
      lcd.noBacklight();
    } else {
      oncounter = oncounter + 1;  
    }
} else {     //on / off
    was_on = true;
    lcd.clear();
    lcd.noBacklight();
    if(button1 == true) {                // toggle on/off
      button1 = false;
      mode_on = true;
    }
}
} // loop 함수의 끝.
