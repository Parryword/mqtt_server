//This example code is in the Public Domain (or CC0 licensed, at your option.)
//By Evandro Copercini - 2018
//
//This example creates a bridge between Serial and Classical Bluetooth (SPP)
//and also demonstrate that SerialBT have the same functionalities of a normal Serial

#include "BluetoothSerial.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "DHT.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

#define DHTPIN 15
#define DHTTYPE 11

using namespace std;


const char* wifi_network_ssid = "Ekonet-Student";
const char* wifi_network_password = "";

const char* soft_ap_ssid = "ESP32-Access-Point Efe";
const char* soft_ap_password = "123456789";
int channel = 1;
int ssid_hidden = 0;
int max_connection = 9;
bool ftm_responder = false;

BluetoothSerial SerialBT;
WiFiServer server(80);

struct ClientData {
  double temp;
  double humidity;
  double windSpeed;
  double roomTemp;
};

struct ClientData clientData;
String header;
String query = "https://api.openweathermap.org/data/2.5/weather?q=Izmir&appid=660aad8d134eb678f8445709216b8361";
String selectedCity = "Izmir";

DHT dht(DHTPIN, DHTTYPE);

String bluetoothMessage = "";
bool notSent = true;

void setup() {
  Serial.begin(115200);

  // Connect to the WiFi
  WiFi.begin(wifi_network_ssid);
  Serial.println("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

  //weatherRequest();

  dht.begin();

  delay(2000);

  float t = dht.readTemperature();

  if (isnan(t)) {
    Serial.println("Failed to read from sensor");
    return;
  }

  Serial.println(t);
  SerialBT.begin("ESP32 - Efe");  //Bluetooth device name
}

void loop() {
  if (Serial.available()) {
    SerialBT.write(Serial.read());
  }
  
  if (SerialBT.available()) {
    bluetoothMessage = SerialBT.readString();
    Serial.println(bluetoothMessage);
    // executeCommand(String());
  }

  if (bluetoothMessage != "") {
    executeCommand(bluetoothMessage);

    Serial.println("Replying to client.\n\n");
    // DO NOT DELETE '\n' FROM THE END OF THE MESSAGE, OTHERWISE IT WILL NOT TRANSMIT
    String a = "Message received.\n";
    uint8_t buf[a.length()];
    memcpy(buf, a.c_str(), a.length());
    SerialBT.write(buf, a.length());
    bluetoothMessage = "";
  }
    
  delay(20);
}

String executeCommand(String command) {
  String s = command;
  String fetch = "FETCH";
  String cred = "CRED";

  int start_f = s.indexOf(fetch);
  int start_c = s.indexOf(cred);

  if (start_f == -1 && start_c == -1) {
    Serial.println("Invalid command.");
    return "";
  }

  if (start_f != -1) {
    Serial.println("Executing FETCH.");
  }

  if (start_c != -1) {
    Serial.println("Executing CRED.");

    String ssid;
    String password;

    String param_s = "-s ";
    String param_p = "-p ";

    int start = s.indexOf(param_s);
    start += param_s.length(); // Skip the command itself
    int end = s.indexOf(param_p, start);
    ssid = s.substring(start, end);
    
    Serial.print("The ssid of the command is: ");
    Serial.println(ssid);

    start = s.indexOf(param_p);
    start += param_p.length(); // Skip the command itself
    end = s.indexOf("\r\n", start);
    password = s.substring(start, end);
    
    Serial.print("The password of the command is: ");
    Serial.println(password);

    Serial.print("Setting AP (Access Point)…");
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(ssid, soft_ap_password, channel, ssid_hidden, max_connection, ftm_responder);

    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);

    server.begin();
  }
  /*
    start += query.length(); // Skip the query itself
    int end = s.indexOf(" HTTP/1.1", start);
    String value = s.substring(start, end);
    
    Serial.print("The value of the query is: ");
    Serial.println(value);*/
  return "";
}