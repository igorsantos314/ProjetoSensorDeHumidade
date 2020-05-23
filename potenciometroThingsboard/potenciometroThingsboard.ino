#include "DHT.h"
#include <ESP8266WiFi.h>
#include <ThingsBoard.h>

#define WIFI_AP "GST_IGOR"
#define WIFI_PASSWORD "13579ig*"

#define TOKEN "rKKF9megL5c9Qja7WV5N"

// DHT
#define DHTPIN 2
#define DHTTYPE DHT22

char thingsboardServer[] = "192.168.0.103";

WiFiClient wifiClient;

// Initialize DHT sensor.
DHT dht(DHTPIN, DHTTYPE);

ThingsBoard tb(wifiClient);

int status = WL_IDLE_STATUS;
unsigned long lastSend;

void setup()
{
  Serial.begin(9600);
  dht.begin();
  delay(10);
  InitWiFi();
  lastSend = 0;
}

void loop()
{
  if ( !tb.connected() ) {
    reconnect();
  }

  if ( millis() - lastSend > 1000 ) { // Update and send only after 1 seconds
    getAndSendTemperatureAndHumidityData();
    lastSend = millis();
  }

  tb.loop();
}

void getAndSendTemperatureAndHumidityData()
{
  float leitura = 0;
  Serial.println("Collecting Potentiometer.");

  Serial.print(analogRead(A0));
  Serial.print("         ");
  leitura = (analogRead(A0)*3.3/1023);
  Serial.println(leitura);

  tb.sendTelemetryFloat("Pino", analogRead(A0));
  tb.sendTelemetryFloat("Potentiometer", leitura);

  delay(1500);

}

void InitWiFi()
{
  Serial.println("Connecting to AP ...");
  // attempt to connect to WiFi network

  WiFi.begin(WIFI_AP, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to AP");
}


void reconnect() {
  // Loop until we're reconnected
  while (!tb.connected()) {
    status = WiFi.status();
    if ( status != WL_CONNECTED) {
      WiFi.begin(WIFI_AP, WIFI_PASSWORD);
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
      }
      Serial.println("Connected to AP");
    }
    Serial.print("Connecting to ThingsBoard node ...");
    if ( tb.connect(thingsboardServer, TOKEN) ) {
      Serial.println( "[DONE]" );
    } else {
      Serial.print( "[FAILED]" );
      Serial.println( " : retrying in 5 seconds]" );
      // Wait 5 seconds before retrying
      delay( 5000 );
    }
  }
}
