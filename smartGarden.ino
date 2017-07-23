#include <SoftwareSerial.h>

SoftwareSerial esp8266(4, 5);

#define ID routerID
#define password routerPassword
#define motor 13
#define espReset 3
#define moistureSensor 2

void homePrint();
void moisture();
void getTime();
void water();
void ISR1();

bool waterBool=false;
int waterTime=10000; //milliseconds
String timeRespond;
String lastWaterTime;
String moistureStatus="DRY";

void setup() {
  attachInterrupt(digitalPinToInterrupt(moistureSensor), ISR1, RISING );
reset:
  // put your setup code here, to run once:
  Serial.begin(9600);
  esp8266.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  
  bool serverReady = false;
  pinMode(espReset,OUTPUT);
  pinMode(motor,OUTPUT);
  pinMode(moistureSensor,INPUT);

  digitalWrite(espReset,LOW);
  delay(100);
  digitalWrite(espReset,HIGH);
  delay(100);
  esp8266.write("AT+RST\r\n");
   delay(3000);
   esp8266.write("AT+CWMODE=3\r\n");
  delay(1000);
  if(esp8266.find("OK"))
      serverReady=true;
  else
      serverReady=false;
  esp8266.flush();
  delay(100);
  
  esp8266.write("AT+CIPMUX=1\r\n");
  delay(1000);
  if(esp8266.find("OK"))
    serverReady=true;
  else
    serverReady=false;
    esp8266.flush();
    delay(100);

  
  esp8266.println("AT+CWJAP=\"" + ID + "\",\"" + password + \"");
  delay(5000);
  if(esp8266.find("OK"))
    serverReady=true;
  else
    serverReady=false;
  esp8266.flush();
  delay(100);

  esp8266.println("AT+CIPSERVER=1,80");
  delay(1000);
  if(esp8266.find("OK"))
    serverReady=true;
  else
    serverReady=false;
  esp8266.flush();
  delay(100);
  
  if(serverReady)
    Serial.println("Server Ready!");
  else
  {
    Serial.println("Restart");
    delay(100);
    goto reset;
  }
}

void loop(){
  if(waterBool)
  {
    water();
  }
  if(esp8266.available()){
    if(esp8266.find("+IPD")){
      String gelen=esp8266.readString();
      Serial.println(gelen);
      if(gelen.indexOf(":GET /?water")>1){ 
        water();
      } 
      if(gelen.indexOf(":GET /favicon.ico")>1){
        Serial.println("FAVICON!");
      }  
      moisture();
      homePrint();
      delay(500);
        esp8266.println("AT+CIPCLOSE=0");
        Serial.println("CLOSE!!");
    }
    else if(esp8266.find("ready")){
      setup();
    }
  }
}

void homePrint(){    
    String text = "<html><head> Test Server </head>";
      text+="<br><a href=\" ?water\"><button type='button'>ON</button></a>"; 
      text+="<br> Last Water=";
      text+=lastWaterTime;
      text+="<br> Moisture Sensor Status=";
      text+=moistureStatus;
      text+="</html>";
      String cipsend = "AT+CIPSEND=";
      cipsend +="0";
      cipsend +=",";
      cipsend += text.length();
      cipsend += "\r\n";
      esp8266.print(cipsend);
      delay(10);
      esp8266.println(text);
}

void moisture(){
  if(digitalRead(moistureSensor)==HIGH)
    moistureStatus="DRY";
  else
    moistureStatus="WET";
}

void getTime(){
startGetTime:
  String cmd = "AT+CIPSTART=4,\"TCP\",\"www.google.com\",80";
  esp8266.println(cmd);
  cmd = "GET / HTTP/1.1\r\nHost: www.google.com.tr\r\n\r\n";
  if(esp8266.find("Error"))
    goto startGetTime;
  esp8266.print("AT+CIPSEND=4,");
  esp8266.println(cmd.length());
  if(esp8266.find(">"))
  {
    esp8266.println(cmd);
  }
  char c;
  while(1)
  {
    if(esp8266.available()){
    if(esp8266.find("Date: ")){
       int i;
       for(i=0;i<31;i++)
       {
        if(esp8266.available())
        {
          c=esp8266.read();
          timeRespond+=c;
        }
        else
        i--;
       }
      esp8266.println("AT+CIPCLOSE=4");
      }

    }
    else
    {
      esp8266.println("AT+CIPCLOSE=4");
      break;
    }
  }
}

void water()
{
  digitalWrite(motor,HIGH);
  delay(waterTime);
  digitalWrite(motor,LOW);
  getTime();
  lastWaterTime = timeRespond;
  waterBool = false;
  Serial.println("Water!!");
}
void ISR1(){
  waterBool = true;
}

