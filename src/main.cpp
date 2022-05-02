#define USE_LittleFS

#include <WiFiManager.h> //https://github.com/tzapu/WiFiManager
#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>

#include <FS.h>
#ifdef USE_LittleFS
#define SPIFFS LITTLEFS
#include <LITTLEFS.h>
#else
#include <SPIFFS.h>
#endif

#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#include <Thermistor.h>
#include <NTC_Thermistor.h>

#include <NTPClient.h>
#include <WiFiUdp.h>

#include <ESP32Servo.h>
#include <TaskScheduler.h>
void runCallback();
void blinkCallback();
Task task_blink(100, TASK_FOREVER, &blinkCallback);
Task task_run(1000, TASK_FOREVER, &runCallback);

#include "../lib/GrowLightInterface/GrowLightLED.h"
#include "../lib/GrowLightInterface/GrowLightFAN.h"
#include "../lib/Timer/Timer.h"

#define FORMAT_LITTLEFS_IF_FAILED false

AsyncWebServer server(80);

#define SENSOR_PIN 34
#define REFERENCE_RESISTANCE 9900
#define NOMINAL_RESISTANCE 10000
#define NOMINAL_TEMPERATURE 25
#define B_VALUE 3977
#define ANALOG_RESULTION 4095

#define LIGHT_PIN 12
#define FAN_PIN 14
ESP32PWM pwm_light;
ESP32PWM pwm_fan;
int freq = 1000;

Thermistor *thermistor;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600 * 2, 60000);

#define BLINK_LED 26
bool BLINK_STATE = false;

boolean light_led_state = false;
String light_led_mode = "MANUAL";
int light_led_duty_cycle = 50;
boolean light_fan_state = false;
String light_fan_mode = "MANUAL";
int light_fan_duty_cycle = 50;
double light_fan_duty_temp_is = 0.0;
double light_fan_duty_temp_max = 30.0;

// GrowLight_LED growLight = {LIGHT_PIN, false, MANUAL, 50};
// GrowLightInterface interface = GrowLightInterface(LIGHT_PIN, false, MANUAL, 50);
GrowLightLED growLight = GrowLightLED(LIGHT_PIN, false, MANUAL, 50, 1000);
GrowLightFAN growLightFan = GrowLightFAN(LIGHT_PIN, false, MANUAL, 70, 3000, 10);

Timer timer = Timer();

const char *PARAM_TYPE = "type";
const char *PARAM_VALUE = "value";

Scheduler runner;

void run_lightTimer();

// Replaces placeholder with LED state value
String processor(const String &var)
{
  timeClient.update();
  const double celsius = thermistor->readCelsius();

  Serial.println(var);
  if (var == "TIME")
  {
    return timeClient.getFormattedTime();
  }
  else if (var == "TEMPERATURE")
  {
    return String(celsius);
  }
  else if (var == "LIGHTSTATE")
  {
    return String(light_led_state);
  }
  else if (var == "LIGHTMODE")
  {
    return String(light_led_mode);
  }
  else if (var == "LIGHTCYCLE")
  {
    return String(light_led_duty_cycle) + String("%");
  }
  else
  {
    return "Not-Found";
  }
}

void notFound(AsyncWebServerRequest *request)
{
  request->send(404, "text/plain", "Not found");
}

void setup()
{

  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(BLINK_LED, OUTPUT);
  runner.init();
  runner.addTask(task_blink);
  runner.addTask(task_run);
  timer.init(8, 0, 24, 0);

  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  Serial.begin(115200);
  pwm_light.attachPin(LIGHT_PIN, freq, 10);
  pwm_fan.attachPin(FAN_PIN, freq, 10);

  thermistor = new NTC_Thermistor(
      SENSOR_PIN,
      REFERENCE_RESISTANCE,
      NOMINAL_RESISTANCE,
      NOMINAL_TEMPERATURE,
      B_VALUE,
      ANALOG_RESULTION);

  if (!LITTLEFS.begin(FORMAT_LITTLEFS_IF_FAILED))
  {
    Serial.println("LITTLEFS Mount Failed");
    return;
  }

  WiFiManager wifiManager;
  // reset saved settings
  // wifiManager.resetSettings();

  // set custom ip for portal
  // wifiManager.setAPStaticIPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));
  wifiManager.autoConnect("AutoConnectAP");
  Serial.println("WiFi connected...)");

  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // === GET ===
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/index.html", String(), false, processor); });
  server.on("/api.html", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/api.html", String(), false, processor); });
  // CSS
  server.on("/assets/bootstrap/css/bootstrap.min.css", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/assets/bootstrap/css/bootstrap.min.css", "text/css"); });
  server.on("/assets/css/styles.min.css", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/assets/css/styles.min.css", "text/css"); });
  // Images
  server.on("/assets/img/icon.drawio.png", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/assets/img/icon.drawio.png"); });

  // JS
  server.on("/assets/js/snackbar.mjs", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/assets/js/snackbar.mjs", "text/javascript"); });
  server.on("/assets/js/script.min.js", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/assets/js/script.min.js", "text/javascript"); });

  server.on("/api/time", HTTP_GET, [](AsyncWebServerRequest *request)
            { 
              String message;
              if (request->hasParam(PARAM_TYPE)) {
                message = request->getParam(PARAM_TYPE)->value();

                if(message == "time_start"){
                  request->send(200, "txt/plain", timer.getStart());
                }else if (message == "time_end"){
                  request->send(200, "txt/plain", timer.getEnd());
                }
              }
              else{
              request->send(200, "text/plain", timeClient.getFormattedTime()); } });

  server.on("/api/light", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              String message;
              if (request->hasParam(PARAM_TYPE)) {
                  message = request->getParam(PARAM_TYPE)->value();

                  if(message == "state"){
                    request->send(200, "text/plain", growLight.getState());
                  }else if (message == "mode"){
                    request->send(200, "text/plain", growLight.getMode());
                  }else if (message == "duty_cycle"){
                    request->send(200, "text/plain", growLight.getDutyCycle());
                  }else if (message == "frequency"){
                    request->send(200, "text/plain", growLight.getFrequency());
                  }

              } else {
                  message = "No message sent";
              } 
              request->send(200, "text/plain", "FOUND, GET: " + message); });

  server.on("/api/fan", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              String message;
              if (request->hasParam(PARAM_TYPE)) {
                  message = request->getParam(PARAM_TYPE)->value();

                  if(message == "state"){
                    request->send(200, "text/plain", growLightFan.getState());
                  }else if (message == "mode"){
                    request->send(200, "text/plain", growLightFan.getMode());
                  }else if (message == "duty_cycle"){
                    request->send(200, "text/plain", growLightFan.getDutyCycle());
                  }else if (message == "frequency"){
                    request->send(200, "text/plain", growLightFan.getFrequency());
                  }else if (message == "temp_is"){
                    request->send(200, "text/plain", growLightFan.getTemperatureIs());
                  }else if (message == "temp_max"){
                    request->send(200, "text/plain", growLightFan.getTemperatureMax());
                  }

              } else {
                  message = "No message sent";
              }
              request->send(200, "text/plain", "FOUND, GET: " + message); });

  // === POST ===
  server.on("/api/light", HTTP_POST, [](AsyncWebServerRequest *request)
            {
              String message;
              if (request->hasParam(PARAM_TYPE)) {
                message = request->getParam(PARAM_TYPE)->value();
                if (request->hasParam(PARAM_VALUE)) {
                  if(message == "state"){
                    growLight.setState(request->getParam(PARAM_VALUE)->value());
                    request->send(200, "text/plain", growLight.getState());
                  }else if (message == "mode"){
                    growLight.setMode(request->getParam(PARAM_VALUE)->value());
                    request->send(200, "text/plain", growLight.getMode());
                  }else if (message == "duty_cycle"){
                    growLight.setDutyCycle(request->getParam(PARAM_VALUE)->value());
                    request->send(200, "text/plain", growLight.getDutyCycle());
                  }else if (message == "frequency"){
                    growLight.setFerquency(request->getParam(PARAM_VALUE)->value());
                    request->send(200, "text/plain", growLight.getFrequency());
                  }
                }
                else {
                  message = "No type value";
                }
              } else {
                  message = "No type sent";
              } 
              request->send(200, "text/plain", "FOUND, POST: " + message); });

  server.on("/api/fan", HTTP_POST, [](AsyncWebServerRequest *request)
            {
              String message;
              if (request->hasParam(PARAM_TYPE)) {
                message = request->getParam(PARAM_TYPE)->value();
                if (request->hasParam(PARAM_VALUE)) {
                  if(message == "state"){
                    growLightFan.setState(request->getParam(PARAM_VALUE)->value());
                    request->send(200, "text/plain", growLightFan.getState());
                  }else if (message == "mode"){
                    growLightFan.setMode(request->getParam(PARAM_VALUE)->value());
                    request->send(200, "text/plain", growLightFan.getMode());
                  }else if (message == "duty_cycle"){
                    growLightFan.setDutyCycle(request->getParam(PARAM_VALUE)->value());
                    request->send(200, "text/plain", growLightFan.getDutyCycle());
                  }else if (message == "frequency"){
                    growLightFan.setFerquency(request->getParam(PARAM_VALUE)->value());
                    request->send(200, "text/plain", growLightFan.getFrequency());
                  }else if (message == "temp_max"){
                    growLightFan.setTemperatureMax(request->getParam(PARAM_VALUE)->value());
                    request->send(200, "text/plain", growLightFan.getTemperatureMax());
                  }
                  
                }
                else {
                  message = "No type value";
                }
              } else {
                  message = "No type sent";
              } 
              request->send(200, "text/plain", "FOUND, POST: " + message); });

  server.on("/api/time", HTTP_POST, [](AsyncWebServerRequest *request)
            { 
              String message;
              if (request->hasParam(PARAM_TYPE)) {
                message = request->getParam(PARAM_TYPE)->value();

                if(message == "time_start_h"){
                  timer.setStartHour(request->getParam(PARAM_VALUE)->value());
                  request->send(200, "txt/plain", timer.getStart());
                }else if (message == "time_start_m"){
                  timer.setStartMinute(request->getParam(PARAM_VALUE)->value());
                  request->send(200, "txt/plain", timer.getStart());
                }
                else if (message == "time_end_h"){
                  timer.setEndHour(request->getParam(PARAM_VALUE)->value());
                  request->send(200, "txt/plain", timer.getEnd());
                }
                else if (message == "time_end_m"){
                  timer.setEndMinute(request->getParam(PARAM_VALUE)->value());
                  request->send(200, "txt/plain", timer.getEnd());
                }
              }
              else{
              request->send(200, "text/plain", timeClient.getFormattedTime()); } });

  server.onNotFound(notFound);

  ArduinoOTA
      .onStart([]()
               {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";
      SPIFFS.end();
      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type); })
      .onEnd([]()
             { Serial.println("\nEnd"); })
      .onProgress([](unsigned int progress, unsigned int total)
                  { Serial.printf("Progress: %u%%\r", (progress / (total / 100))); })
      .onError([](ota_error_t error)
               {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed"); });

  ArduinoOTA.begin();

  timeClient.begin();
  timeClient.update();
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
  server.begin();
  task_blink.enable();
  task_run.enable();
}

void loop()
{
  runner.execute();
  ArduinoOTA.handle();
}

void printStatus()
{
  Serial.print("====== STATUS ======");
}

void run_lightTimer()
{
  if (growLight.mode == AUTO)
  {
    if (timeClient.getHours() >= timer.getStartHour() && timeClient.getMinutes() >= timer.getStartMinute())
    {
      if (timeClient.getHours() <= timer.getEndHour() && timeClient.getMinutes() >= timer.getEndMinute())
      {
        growLight.state = true;
      }
      else
      {
        growLight.state = false;
      }
    }
    else
    {
      growLight.state = false;
    }
  }
}

void blinkCallback()
{
  BLINK_STATE = !BLINK_STATE;
  digitalWrite(BLINK_LED, BLINK_STATE);
}

void runCallback()
{
  if (task_run.isFirstIteration())
  {
    Serial.println("Init Task");
    growLight.state = false;
    growLight.duty_cycle = 0;
    growLightFan.state = true;
    growLightFan.duty_cycle = 100;
  }

  // Get Data
  growLightFan.setTemperatureIs(String(thermistor->readCelsius()));

  // Check Tasks
  run_lightTimer();

  // === LIGHT ===
  if (growLight.state)
  {
    if ((double)growLight.frequency != pwm_light.readFreq())
      pwm_light.adjustFrequency(growLight.frequency);
    pwm_light.writeScaled(1.0 - ((float)growLight.duty_cycle / 100));
  }
  else
  {
    pwm_light.writeScaled(1.0);
  }

  // === FAN ===
  if (growLightFan.state)
  {
    if ((double)growLightFan.frequency != pwm_fan.readFreq())
      pwm_fan.adjustFrequency(growLightFan.frequency);
    pwm_fan.writeScaled((double)growLightFan.duty_cycle / 100);
  }
  else
  {
    pwm_fan.writeScaled(0.0);
  }

  if (growLightFan.temperature_is > growLightFan.temperature_max)
  {
    growLightFan.state = true;
  }
}