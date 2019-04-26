#include <SoftwareSerial.h>
#include <Wire.h>
#include <SimpleDHT.h>
 
// pin setting
int ledPin[5] = {22,23,24,25,26}; //living, bedroom, bathroom, kitchen, pool
#define dhtPin 53
#define fanPin 52
#define R 11
#define G 12
#define B 13
 
// response type
#define text 0
#define json 1
 
SoftwareSerial mySerial(50, 51); // RX | TX
 
// dht
SimpleDHT11 dht11(dhtPin);
byte temperature = 0;
byte humidity = 0;
 
// status
int ledStatus[5] = {0, 0, 0, 0, 0};
int RGBStatus = 0;
int RGBred    = 0;
int RGBgreen  = 0;
int RGBblue   = 0;
int fanStatus = 0;
 
 
void setTimmer(bool interruptEnable = false) {
  cli();
  TCCR1A = 0;
  TCCR1B = 0; //先將負責控制的register初始化為0。
  TCNT1 = 0; //時間最一開始是0。
 
  TCCR1B |= (1 << WGM12); //將模式設為CTC。
  TCCR1B |= (1<<CS12) | (1<<CS10); //將時間除上1024。
  OCR1A = 7812*3;   //因為16M/1024~=15625，所以將OCR1A設為7812就能得到0.5秒。
  if (interruptEnable) {
    TIMSK1 |= (1<<OCIE1A); // enable timer compare int.
  }
  sei();
}
 
void setup() 
{
 
  // pin setting
  pinMode(ledPin[0], OUTPUT);
  pinMode(ledPin[1], OUTPUT);
  pinMode(ledPin[2], OUTPUT);
  pinMode(ledPin[3], OUTPUT);
  pinMode(ledPin[4], OUTPUT);
  pinMode(dhtPin, OUTPUT);
  pinMode(fanPin, OUTPUT);
  pinMode(R, OUTPUT);
  pinMode(G, OUTPUT);
  pinMode(B, OUTPUT);
 
  delay(100);
  Serial.begin(9600);
  mySerial.begin(9600);
 
  delay(100);
  dht11.read(&temperature, &humidity, NULL);;
  delay(100);
//  setTimmer(true);
}
 
void loop() {
  if (mySerial.available()) receiveEvent();
  delay(100);
}
 
ISR(TIMER1_COMPA_vect) { // Timer1 ISR
  dht11.read(&temperature, &humidity, NULL);
}
 
 
// 接收指令
void receiveEvent()
{
 
  String req = "";
  Serial.println("receive request");
  while (mySerial.available() > 0)
  {
    char c = mySerial.read();
    req += c;
  }
 
  mySerial.flush();
  Serial.println(req);
  Serial.flush();
  decoder(req);
}
 
// 回傳訊號
void requestEvent(String res, int type)
{
  String response = "";
  String sending  = "";
 
  if (type == text)
  {
    response += "text/html\r\n\r\n";
    response += res;
 
  } else if (type == json)
  {
    response += "application/json\r\nCache-Control: no-store\r\n\r\n";
    response += "{";
    response += res;
    response += "}";
  }
 
  Serial.print("response:");
  Serial.println(response);
  Serial.flush();
 
  // start sending
  while (response.length() > 20)
  {
    sending  = response.substring(0, 20);
    response = response.substring(20);
    Serial.println(sending);
    mySerial.print(sending);
    mySerial.flush();
    delay(20);
  }
 
  mySerial.print(response);
  mySerial.flush();
 
  // send end signal
  delay(100);
  mySerial.print("fine");
  mySerial.flush();
 
}
 
 
void decoder(String req)
{
  if      (req.indexOf("/led") != -1)     ledControl(req);
  else if (req.indexOf("/dht") != -1)     dhtControl(req);
  else if (req.indexOf("/RGB") != -1)     RGBControl(req);
  else if (req.indexOf("/fan") != -1)     fanControl(req);
  else                                    Serial.println("error request");
}
 
 
void fanControl(String req)
{
  if      (req.indexOf("on") != -1)     fanStatus = 1;
  else if (req.indexOf("off") != -1)    fanStatus = 0;
  else if (req.indexOf("status") != -1);
 
  digitalWrite(fanPin, fanStatus);
  requestEvent(String(fanStatus), text);
}
 
 
void ledControl(String req)
{
  int accessory = 0;
 
  if      (req.indexOf("/0/") != -1) accessory = 0; // living room
  else if (req.indexOf("/1/") != -1) accessory = 1; // bedroom
  else if (req.indexOf("/2/") != -1) accessory = 2; // bathroom
  else if (req.indexOf("/3/") != -1) accessory = 3; // kitchen
  else if (req.indexOf("/4/") != -1) accessory = 4; // pool
 
  if      (req.indexOf("on") != -1)     ledStatus[accessory] = 1;
  else if (req.indexOf("off") != -1)    ledStatus[accessory] = 0;
  else if (req.indexOf("status") != -1);
 
  digitalWrite(ledPin[accessory], ledStatus[accessory]);
  requestEvent(String(ledStatus[accessory]), text);
}
 
 
void dhtControl(String req)
{
  String res = "";
  dht11.read(&temperature, &humidity, NULL);
 
  res += "\"temperature\":";
  res += String(temperature);
  res += ", ";
  res += "\"humidity\":";
  res += String(humidity);
 
  requestEvent(res, json);
}
 
 
String DECtoHEX (int color)
{
  String hex = "";
  if (color < 16) hex = "0";
  hex += String(color, HEX);
  return hex;
}
 
int _HEXtoDEC(char c)
{
  switch (c)
  {
    case '0': return 0;  break;
    case '1': return 1;  break;
    case '2': return 2;  break;
    case '3': return 3;  break;
    case '4': return 4;  break;
    case '5': return 5;  break;
    case '6': return 6;  break;
    case '7': return 7;  break;
    case '8': return 8;  break;
    case '9': return 9;  break;
    case 'A': return 10; break;
    case 'a': return 10; break;
    case 'B': return 11; break;
    case 'b': return 11; break;
    case 'C': return 12; break;
    case 'c': return 12; break;
    case 'D': return 13; break;
    case 'd': return 13; break;
    case 'E': return 14; break;
    case 'e': return 14; break;
    case 'F': return 15; break;
    case 'f': return 15; break;
    default:  return 0;  break;
  }
}
 
int HEXtoDEC (String colorSet)
{
  return _HEXtoDEC(colorSet[0]) * 16 + _HEXtoDEC(colorSet[1]);
}
 
void RGBbrightnessControl(String req)
{
  int brightness;
  if (req.indexOf("status") != -1) // return a number 0 ~ 100
  {
    brightness = (RGBred + RGBgreen + RGBblue) / 3;
    requestEvent(String(brightness), text);
 
  } else
  {
    int Start   = req.indexOf("s/") + 2;
    int End = req.indexOf("HT") - 1;
 
    brightness = req.substring(Start, End).toInt();
 
    RGBred = RGBgreen = RGBblue = brightness;
    analogWrite(R, RGBred);
    analogWrite(G, RGBgreen);
    analogWrite(B, RGBblue);
 
    RGBStatus = (brightness == 0) ? 0 : 1;
    requestEvent(String(brightness), text);
  }
}
 
void RGBcolorControl(String req)
{
  if (req.indexOf("status") != -1) // return a 6-bit HEX
  {
    String colorSet = DECtoHEX(RGBred) + DECtoHEX(RGBgreen) + DECtoHEX(RGBblue);
    requestEvent(colorSet, text);
 
  } else
  {
    int Start         = req.indexOf("r/") + 2;
    int End       = req.indexOf("HT") - 1;
 
    String colorSet = req.substring(Start, End);
    String colorR   = "";
    String colorG   = "";
    String colorB   = "";
 
    colorR = colorR + colorSet[0] + colorSet[1];
    colorG = colorG + colorSet[2] + colorSet[3];
    colorB = colorB + colorSet[4] + colorSet[5];
 
    RGBred =   HEXtoDEC(colorR);
    RGBgreen = HEXtoDEC(colorG);
    RGBblue =  HEXtoDEC(colorB);
 
    analogWrite(R, RGBred);
    analogWrite(G, RGBgreen);
    analogWrite(B, RGBblue);
 
    RGBStatus = (!RGBred && !RGBgreen && !RGBblue) ? 0 : 1;
    requestEvent(colorSet, text);
  }
}
 
void RGBControl(String req)
{
  if      (req.indexOf("brightness")  != -1)   RGBbrightnessControl(req);
  else if (req.indexOf("color")       != -1)   RGBcolorControl(req);
  else if (req.indexOf("status")      != -1)   requestEvent(String(RGBStatus), text);
  else if (req.indexOf("on")          != -1)
  {
    analogWrite(R, 100);
    analogWrite(G, 100);
    analogWrite(B, 100);
 
    RGBStatus = 1;
    RGBred    = 100;
    RGBgreen  = 100;
    RGBblue   = 100;
 
    requestEvent(String(RGBStatus), text);
 
  } else if (req.indexOf("off") != -1)
  {
    analogWrite(R, 0);
    analogWrite(G, 0);
    analogWrite(B, 0);
 
    RGBStatus = 0;
    RGBred    = 0;
    RGBgreen  = 0;
    RGBblue   = 0;
 
    requestEvent(String(RGBStatus), text);
  }
}