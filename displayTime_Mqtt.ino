/* wifi 2.4GHz 대역폭에서만 동작함 */

#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <PubSubClient.h>

//#define BUTTON_PIN D3
#define DHT_PIN D8
#define DHT_TYPE DHT11
#define CDS_PIN A0

#define MSG_BUFFER_SIZE	(100)
char msg[MSG_BUFFER_SIZE];
int value = 0;

DHT dht(DHT_PIN, DHT_TYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);

const char *ssid = "eenoo";
const char *password = "00000001";
const char* mqtt_server = "192.168.103.191";

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "kr.pool.ntp.org", 32400, 3600000);

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;

unsigned long cdsValue = 0;

// int prevBtn = 0;
// int currBtn = 0;
// int ledState = 0;

void setup_wifi() {

  delay(5000);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  //randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  //if ((char)payload[0] == '1') {
  //  digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
  //} else {
  //  digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  //}

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "arduinoClient";
    //clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect("arduinoClient")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "start MQTT..");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  // pinMode(BUTTON_PIN, INPUT_PULLUP);
  dht.begin();
  setup_wifi();
 // WiFi.begin(ssid, pwd);

  //delay(3000);
  //Serial.print("Connecting");
  //while(WiFi.status() != WL_CONNECTED){
  //  delay(500);
    
  //  Serial.print(".");
  //}

  timeClient.begin();

  lcd.init();
  lcd.backlight();

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);  

  //lcd.setCursor(0,0);
  //lcd.print("JNK SOFT TIME");
}

void loop() {
  // put your main code here, to run repeatedly:
  //스위치추가
  // currBtn = !digitalRead(BUTTON_PIN);

  // if(currBtn != prevBtn){
  //   prevBtn = currBtn;
  //   if(currBtn == 1){
  //     ledState =!ledState;
  //     Serial.println(ledState);
  //     lcd.noBacklight();
  //   }else{
  //     lcd.backlight();

  //   }
  //   delay(50);
  // }
  //스위치 추가 끝
  //조도값 측정
  cdsValue = analogRead(CDS_PIN);
  Serial.println(cdsValue);
  
  float temp = dht.readTemperature();
  float humi = dht.readHumidity();

  Serial.println(temp);
  Serial.println(humi);

  String str1 = "T:" + String(temp,2) + " " + "H:" + String(humi,2);

  String str2 = "L:" + String(cdsValue) + " " + String(timeClient.getFormattedTime());

  lcd.setCursor(0,0);
  lcd.print(str1);
  
  timeClient.update();
  Serial.println(timeClient.getFormattedTime());
  lcd.setCursor(0,1);
  lcd.print(str2);  

  delay(1000);

  //MQTT publishing....
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  /* 밑에서 delay를 상요하면 재접속 reconnect 함수에 영향을 줌으로
     delay함수를 사용하는 기능의 로직을 구현 한다.*/  
  unsigned long now = millis();
  if (now - lastMsg > 1000) {
    lastMsg = now;
    ++value;
    snprintf (msg, MSG_BUFFER_SIZE, "Temp : %.2f\nHumi : %.2f\nLux : %ld\nTime : %s\n", temp, humi, cdsValue, timeClient.getFormattedTime());
    //snprintf (msg, MSG_BUFFER_SIZE, "Temp : %.2f\nHumi : %.2f\nTime : %s\n", temp, humi, timeClient.getFormattedTime());
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("outTopic", msg);
  }
  
}
