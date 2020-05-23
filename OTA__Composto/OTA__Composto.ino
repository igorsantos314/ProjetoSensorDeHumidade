#define S Serial

#define mSSID "GST_IGOR"
#define mPASS "13579ig*"

#define TIMEOUT_MS 3000


#include <Arduino.h>
#include <ESP8266WiFi.h>
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

String VERSION = "1.2.3";

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
        http.begin("ns1.dobitaobyte.lan", 80, "/version.html");       
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
                    
                    t_httpUpdate_return ret = ESPhttpUpdate.update("http://ns1.dobitaobyte.lan/fw.bin");
                    
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

void setup() {
  //ESP.wdtDisable();
  //ESP.wdtEnable(5000);
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

void loop() {
    if (_timeout){
      //S.println(system_get_free_heap_size());
      S.println("cuco!");
      checkUpdate();
      _timeout = false;
    }
    yield();
}
