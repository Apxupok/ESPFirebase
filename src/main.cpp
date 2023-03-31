//Подключаем необходимые библиотеки
#include <Arduino.h>
#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFiUdp.h>
#include <NTPClient.h>


#define ONE_WIRE_BUS D2         // Порт для датчика
#define TEMPERATURE_PRECISION 9 // Какая-то точность (не понятно)

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");


//идем по wire
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
//Массив для датчиков
DeviceAddress thermo1, thermo2, thermo3, thermo4, thermo5, thermo6;

//Подключаем дополнительные файлы
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

// Указываем учетные данные для доступа к Wi-Fi сети
#define WIFI_SSID "Snab"
#define WIFI_PASSWORD "tehnopark"

// Вставляем проектный API ключ Firebase
#define API_KEY "AIzaSyBrVBPxBNhN4IYJgCcyWlWznsGIuZFefdY"

// Вставляем URL базы данных Realtime
#define DATABASE_URL "https://esp-firebase-b51db-default-rtdb.europe-west1.firebasedatabase.app/" 

// Создаем объект FirebaseData
FirebaseData fbdo;

// Авторизация и создание объектов auth и config
FirebaseAuth auth;
FirebaseConfig config;

// Таймер для отправки данных
unsigned long sendDataPrevMillis = 0;

// Счетчик для отслеживания отправленных данных
int count = 0;

// Переменная для отслеживания успешной регистрации
bool signupOK = false;

int sensor1 = 0;
int sensor2 = 0;
int sensor3 = 0;
int sensor4 = 0;
int sensor5 = 0;
int sensor6 = 0;



void setup(){
  while (!Serial)
  {
    ;
  }
  
  Serial.begin(115200);
  //sensors.begin();
  Serial.print("Поиск девайсов...");
  Serial.print("Найдено ");
  Serial.print(sensors.getDeviceCount(), DEC);
  Serial.println(" устройства.");

  if (!sensors.getAddress(thermo1, 0)) Serial.println("Unable to find address for Device 1");
  if (!sensors.getAddress(thermo2, 1)) Serial.println("Unable to find address for Device 2");
  if (!sensors.getAddress(thermo3, 2)) Serial.println("Unable to find address for Device 3");
  if (!sensors.getAddress(thermo4, 3)) Serial.println("Unable to find address for Device 4");
  if (!sensors.getAddress(thermo5, 4)) Serial.println("Unable to find address for Device 5");
  if (!sensors.getAddress(thermo6, 5)) Serial.println("Unable to find address for Device 6");

  sensors.setResolution(thermo1, TEMPERATURE_PRECISION);
  sensors.setResolution(thermo2, TEMPERATURE_PRECISION);
  sensors.setResolution(thermo3, TEMPERATURE_PRECISION);
  sensors.setResolution(thermo4, TEMPERATURE_PRECISION);
  sensors.setResolution(thermo5, TEMPERATURE_PRECISION);
  sensors.setResolution(thermo6, TEMPERATURE_PRECISION);

  sensors.requestTemperatures();
  sensor1 = sensors.getTempC(thermo1);
  sensor2 = sensors.getTempC(thermo2);
  sensor3 = sensors.getTempC(thermo3);
  sensor4 = sensors.getTempC(thermo4);
  sensor5 = sensors.getTempC(thermo5);
  sensor6 = sensors.getTempC(thermo6);
  
  //Подключаемся к Wi-Fi сети
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  //Задаем ключ API
  config.api_key = API_KEY;

  //Задаем URL базы данных
  config.database_url = DATABASE_URL;

  //Регистрируемся на Firebase 
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  // Задаем функцию обратного вызова для генерации токенов
  config.token_status_callback = tokenStatusCallback; //см. addons/TokenHelper.h
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  timeClient.setTimeOffset(18000); // adjust the time zone as required
  timeClient.forceUpdate();
}

void loop(){
  timeClient.update();
  time_t now = timeClient.getEpochTime();
  String timeStamp = String(ctime(&now));
  String pathToCurrent = "лаборатория/Текущии показатели/";

  //Отправляем данные каждые 15 секунд
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 60000 || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();
    
    // Записываем целочисленное значение в базу данных
    if (Firebase.RTDB.setInt(&fbdo, "лаборатория/Текущии показатели/прошло времени с момента запуска", count)){
      Serial.println("PASSED");
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    count++;
    
    // Записываем значения в базу данных
    if (Firebase.RTDB.setFloat(&fbdo, pathToCurrent+"датчик 1",sensor1)){
      //Текущие показатели
      Firebase.RTDB.setFloat(&fbdo, pathToCurrent+"датчик 2", sensor2);
      Firebase.RTDB.setFloat(&fbdo, pathToCurrent+"датчик 3", sensor3);
      Firebase.RTDB.setFloat(&fbdo, pathToCurrent+"датчик 4", sensor4);
      Firebase.RTDB.setFloat(&fbdo, pathToCurrent+"датчик 5", sensor5);
      Firebase.RTDB.setFloat(&fbdo, pathToCurrent+"датчик 6", sensor6);
      //Все показания
      Firebase.RTDB.setFloat(&fbdo, "лаборатория/"+std::to_string(count)+"/датчик 1", sensor1);
      Firebase.RTDB.setFloat(&fbdo, "лаборатория/"+std::to_string(count)+"/датчик 2", sensor2);
      Firebase.RTDB.setFloat(&fbdo, "лаборатория/"+std::to_string(count)+"/датчик 3", sensor3);
      Firebase.RTDB.setFloat(&fbdo, "лаборатория/"+std::to_string(count)+"/датчик 4", sensor4);
      Firebase.RTDB.setFloat(&fbdo, "лаборатория/"+std::to_string(count)+"/датчик 5", sensor5);
      Firebase.RTDB.setFloat(&fbdo, "лаборатория/"+std::to_string(count)+"/датчик 6", sensor6);

      Serial.println("PASSED");
      Serial.println(timeStamp);
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
  }
}