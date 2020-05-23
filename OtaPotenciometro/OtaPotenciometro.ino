//THINGSBOARD
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

// ----------------------- OTA -----------------------
#define S Serial

#define mSSID "GST_IGOR"
#define mPASS "13579ig*"

#define TIMEOUT_MS 3000

#include <Arduino.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>

extern "C"{
#include "user_interface.h"
}

os_timer_t mTimer;

ESP8266WiFiMulti WiFiMulti;
HTTPClient http;

bool _timeout = false;
bool canGo    = true;

String VERSION = "1.2.4";

void tCallback(void *tCall){
    _timeout = true;
}

void usrInit(void){
    os_timer_setfn(&mTimer, tCallback, NULL);
    os_timer_arm(&mTimer, TIMEOUT_MS, true);
}

void checkUpdate(void){
    if (canGo){
      canGo = false;
    }
    else{
      return;
    }
  
    if((WiFiMulti.run() == WL_CONNECTED)){
        http.begin("192.168.0.103", 80, "/version.html");       
        // inicio da conexao HTTP e envio do header
        int httpCode = http.GET();
        
        if(httpCode){
            //S.printf("[HTTP] GET... code: %d\n", httpCode);
            if(httpCode == 200) {
                String payload = http.getString();
                http.end();
                payload.replace(".","");
                VERSION.replace(".","");
                
                S.print("Local Version: ");
                S.println(VERSION);
                S.print("Remote Version: ");
                S.println(payload);

                if (VERSION.toInt() < payload.toInt()){
                    Serial.println("New version");
                    payload = "";
                    
                    t_httpUpdate_return ret = ESPhttpUpdate.update("http://192.168.0.103/fw.bin");
                    
                    switch(ret) {
                        case HTTP_UPDATE_FAILED:
                            S.println("HTTP_UPDATE_FAILED");
                        break;

                    case HTTP_UPDATE_NO_UPDATES:
                        S.println("HTTP_UPDATE_NO_UPDATES");
                        break;

                    case HTTP_UPDATE_OK:
                        S.println("HTTP_UPDATE_OK");
                        break;
                    }
                }
                else{
                  S.println("No updates available for now... ");
                  
                }
            }
        }
        else{
            S.print("[HTTP] GET... failed, no connection or no HTTP server\n");
        }
    }
    canGo = true;
}

//-----------------------------------------------------------------------------------------------------

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

//-----------------------------------------------------------------------------------------------------


void setup() {
  //ESP.wdtDisable();
  //ESP.wdtEnable(5000);

    //Potenciometro
    dht.begin();
    delay(10);
    InitWiFi();
    
    lastSend = 0;
    usrInit();
    S.begin(115200);
    S.println();

    for(char a = 'd'; a > 'a'; a--) {
        S.printf("[SETUP] WAIT %d...\n", a-97);
        S.flush();
        delay(1000);
    }
    
    S.println("Starting connection, please wait...");
    WiFiMulti.addAP(mSSID, mPASS);
}

//variaveis OTA
bool statusUpadate = true;
int cont = 0;

//Chamada pra verificação de Update
void callUpdate(){
  if (_timeout){
      //S.println(system_get_free_heap_size());
      S.println("cuco!");
      checkUpdate();
      _timeout = false;
    }
  yield();
}

//Verificar variavel contadora
void checkConter(){
  //Verifica Atualização a cada 50
    if(cont == 10){
      cont = 0;
      statusUpadate = true;
    }
    
    cont = cont + 1;
    //S.println(cont);
}

void loop() {

    S.print("Verificar Atualização? ");
    S.println(statusUpadate);
    
    //Verificação de atualização no primeiro Boot
    if (statusUpadate == true){
      callUpdate();
      statusUpadate = false;
    }
  
    //Main do Codigo Principal
    if ( !tb.connected() ) {
      reconnect();
    }

    if ( millis() - lastSend > 1000 ) { // Update and send only after 1 seconds
      getAndSendTemperatureAndHumidityData();
      lastSend = millis();
    }

    tb.loop();
    delay(1000);

    //Função contadora
    checkConter();
    
}
