#include <Arduino.h>
#include <ArduinoJson.h>
#include <rpcWiFi.h>
#include <SPI.h>

#include "ntp.h"
#include "config.h"

#include <AzureIoTHub.h>
#include <AzureIotProtocol_MQTT.h>
#include <iothubtransportmqtt.h>
#include"TFT_eSPI.h"
TFT_eSPI tft;
TFT_eSprite spr = TFT_eSprite(&tft); //Initializing buffer
IOTHUB_DEVICE_CLIENT_LL_HANDLE _device_ll_handle;

static void connectionStatusCallback(IOTHUB_CLIENT_CONNECTION_STATUS result,
                                         IOTHUB_CLIENT_CONNECTION_STATUS_REASON reason,
                                         void* context){
                                          if(result == IOTHUB_CLIENT_CONNECTION_AUTHENTICATED)
                                           { Serial.println("Connected to Iot");
                                           }
                                          else{
                                            Serial.println("Not Connected to Iot");
                                          }
                                          // not
                                         }
void ConnectIoTHub()
{
  IoTHub_Init();
  _device_ll_handle = IoTHubDeviceClient_LL_CreateFromConnectionString(CONNECTION_STRING, MQTT_Protocol);
  if(_device_ll_handle == NULL)
  {
    return;
  }
  IoTHubDeviceClient_LL_SetConnectionStatusCallback(_device_ll_handle, connectionStatusCallback, NULL);
}
void connectWiFi()
{
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("Connecting to WiFi..");
        WiFi.begin(SSID, PASSWORD);
        delay(500);
    }

    Serial.println("Connected!");
}

void setup()
{

	Serial.begin(9600);

	while (!Serial)
		; // Wait for Serial to be ready

	delay(1000);
    tft.begin();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLUE);
    tft.setTextColor(TFT_BLACK);
    tft.setTextSize(8);
    tft.drawString("Light ", 10,15);
    tft.drawString("Value: ", 17,65);
    connectWiFi();

    //COnnect IoTHub
    initTime();
    ConnectIoTHub();

    pinMode(WIO_LIGHT, INPUT);
    pinMode(D0, OUTPUT);
    pinMode(WIO_BUZZER, OUTPUT);
}

void sendTelemetry(const char *telemetry)
{
    IOTHUB_MESSAGE_HANDLE message_handle = IoTHubMessage_CreateFromString(telemetry);
    IoTHubDeviceClient_LL_SendEventAsync(_device_ll_handle, message_handle, NULL, NULL);
    IoTHubMessage_Destroy(message_handle);
    Serial.print("telemetry_send: ");
    Serial.println(telemetry);
}

int counter = 15;

void loop()
{
    IoTHubDeviceClient_LL_DoWork(_device_ll_handle);

    int light = analogRead(WIO_LIGHT);
    Serial.print("Light value: ");
    Serial.println(light);

    DynamicJsonDocument doc(1024);
    doc["Light"] = light;
    string telemetry;
    serializeJson(doc, telemetry);
    sendTelemetry(telemetry.c_str());
    
    //sprite buffer for light
    spr.createSprite(200, 200); //create buffer
    spr.fillSprite(TFT_WHITE); //fill background color
    spr.setTextSize(15);
    spr.setTextColor(TFT_BLACK);
    spr.drawNumber(light,20,20); //display value
    spr.pushSprite(100,150); //push to LCD
    spr.deleteSprite(); //clear buffer

    delay(2000);
    if (light < 100)
    {
        analogWrite(WIO_BUZZER, 0);
    }
    else
    {
        analogWrite(WIO_BUZZER, 140);
    }
    
}