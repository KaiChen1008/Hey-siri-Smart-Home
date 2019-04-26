#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiClient.h>
#include <SoftwareSerial.h>
#include <Wire.h>
 
// communication setting
SoftwareSerial mySerial(D2, D3); // RX | TX
 
// wifi setting
WiFiServer server(80);
IPAddress ip;
 
// wifi config
char* ssid1 = "kaichen-phone";
char* password1 = "*****************";
 
char* ssid2 = "Kai-chen-link";
char* password2 = "***************";
 
const char* HostName = "kaichenHome";
 
// resquest
String lastResquest = "";
 
// response
String res = "";
boolean finish = false;
 
 
bool TryConnecting(char* ssid, char* password)
{
  String connectingMesg = "try to connect to " + String(ssid);
  Serial.println(connectingMesg);
 
  WiFi.hostname(HostName);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
 
  // waiting response
  int times = 0;
  while (WiFi.status() != WL_CONNECTED && times < 15)
  {
    Serial.print(".");
    delay(500);
    times++;
  }
 
  Serial.println("");
  if (times < 15) return true;
  else        return false;
}
 
 
void setup() {
 
  pinMode(D2, INPUT);
  pinMode(D3, OUTPUT);
 
  Serial.begin(115200);
  mySerial.begin(9600);
 
  delay(10);
 
  // connect to phone network
  bool connected = TryConnecting(ssid1, password1);
  if (!connected)  TryConnecting(ssid2, password2);
 
  // send connected messages
  ip = WiFi.localIP();
  Serial.println("WiFi Connected");
  Serial.println(WiFi.localIP());
  Serial.println(WiFi.hostname());
 
  // set domain name
  while (!MDNS.begin("kaichenHome"))
  {
    Serial.println("Error setting up MDNS responder!");
    delay(500);
  }
 
  Serial.println("mDNS responder started");
 
  // start server
  server.begin();
  Serial.println("Server started");
 
  // add service to MDNS-SD
  MDNS.addService("http", "tcp", 80);
}
 
void loop()
{
  MDNS.update();
 
  WiFiClient client = server.available();
  if (!client) return;
  while (!client.available()) delay(100);
 
  // 接收 request from HomeBridge
  String req = client.readStringUntil('\r');
 
  client.flush();
 
  // 傳送給 Arduino
  if (req.indexOf("favicon") != -1) return;
  Serial.println();
  Serial.println(req);
 
  mySerial.print(req);
  mySerial.flush();
 
  // waiting for response
  volatile int timer = 0;
  while (!mySerial.available() && timer < 10)
  {
    Serial.print(".");
    delay(100);
    timer ++;
  }
 
  Serial.print("\n");
 
  //get response
  String s = "HTTP/1.1 200 OK\r\nContent-Type: ";
  volatile int receive_count = 0;
  while (receive_count < 10)
  {
    receiveResponse();
    if (res.indexOf("fi") != -1)
    {
      Serial.println("------------------------");
      break;
    }
    s += res;
    receive_count ++;
  }
 
  if (receive_count == 10)
  {
    Serial.println();
    Serial.println("time out");
  } else
  {
    Serial.println(s);
    Serial.println("------------------------");
    client.print(s);
    client.flush();
  }
 
  delay(100);
}
 
 
void receiveResponse()
{
  volatile int times = 0;
  while (!mySerial.available() && times < 5)
  {
    times++;
    delay(100);
    Serial.print(".");
  }
  res = "";
  while (mySerial.available())
  {
    char c = char(mySerial.read());
    res += c;
  }
  mySerial.flush();
}
