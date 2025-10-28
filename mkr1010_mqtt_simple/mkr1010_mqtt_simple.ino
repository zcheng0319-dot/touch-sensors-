// Cheng Zhong October 2025 - v1 - MQTT messager to vespera

// works with MKR1010

#include <SPI.h>
#include <WiFiNINA.h>
#include <PubSubClient.h>
#include "arduino_secrets.h" 
#include <utility/wifi_drv.h>   // library to drive to RGB LED on the MKR1010

const int TOUCH_PIN = A0;

/**** please enter your sensitive data in the Secret tab/arduino_secrets.h
**** using format below*/
#define SECRET_SSID "CE-Hub-Student"
#define SECRET_PASS "casa-ce-gagarin-public-service"
#define SECRET_MQTTUSER "student"
#define SECRET_MQTTPASS "ce2021-mqtt-forget-whale"

const char* ssid          = SECRET_SSID;
const char* password      = SECRET_PASS;
const char* mqtt_username = SECRET_MQTTUSER;
const char* mqtt_password = SECRET_MQTTPASS;
const char* mqtt_server   = "mqtt.cetools.org";
const int mqtt_port       = 1884;

// create wifi object and mqtt object
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

// breathing effect control
bool breathing = false;           
unsigned long lastUpdate = 0;    
int hue = 0;                      

// Make sure to update your lightid value below with the one you have been allocated
String lightId = "30"; // the topic id number or user number being used.

// Here we define the MQTT topic we will be publishing data to
String mqtt_topic = "student/CASA0014/luminaire/" + lightId;            
String clientId = ""; // will set once i have mac address so that it is unique

// NeoPixel Configuration - we need to know this to know how to send messages 
// to vespera 
const int num_leds = 72;
const int payload_size = num_leds * 3; // x3 for RGB

// Create the byte array to send in MQTT payload this stores all the colours 
// in memory so that they can be accessed in for example the rainbow function
byte RGBpayload[payload_size];

void setup() {
  Serial.begin(115200);
  //while (!Serial); // Wait for serial port to connect (useful for debugging)
  Serial.println("Vespera");

  // print your MAC address:
  byte mac[6];
  WiFi.macAddress(mac);
  Serial.print("MAC address: ");
  printMacAddress(mac);

  Serial.print("This device is Vespera ");
  Serial.println(lightId);

  // Connect to WiFi
  startWifi();

  // Connect to MQTT broker
  mqttClient.setServer(mqtt_server, mqtt_port);
  mqttClient.setBufferSize(2000);
  mqttClient.setCallback(callback);
  
  Serial.println("Set-up complete");
  pinMode(LED_BUILTIN, OUTPUT);

}
 
void loop() {
  if (!mqttClient.connected()) {
    reconnectMQTT();
  }
  
  if (WiFi.status() != WL_CONNECTED){
    startWifi();
  }
  mqttClient.loop();

  static bool lastPressed = false;   
  int raw = analogRead(TOUCH_PIN);  
  bool pressed = (raw > 300);           

  // detect changes in touch status
  if (pressed != lastPressed) {
    lastPressed = pressed;

    if (pressed) {
      breathing = true;
      Serial.println("TOUCH PRESSED -> start breathing");
    } else {
      breathing = false;
      fadeToColor(0, 0, 0, 20, 20);  // slowly fade out after release
      Serial.println("TOUCH RELEASED -> fade out");
    }
  }

  // if breathing mode on, run effect
  if (breathing) {
    breathingEffect();
  }

  delay(20);
}

// Function to update the R, G, B values of a single LED pixel
// RGB can a value between 0-254, pixel is 0-71 for a 72 neopixel strip
void send_RGB_to_pixel(int r, int g, int b, int pixel) {
  // Check if the mqttClient is connected before publishing
  if (mqttClient.connected()) {
    // Update the byte array with the specified RGB color pattern
    RGBpayload[pixel * 3 + 0] = (byte)r; // Red
    RGBpayload[pixel * 3 + 1] = (byte)g; // Green
    RGBpayload[pixel * 3 + 2] = (byte)b; // Blue

    // Publish the byte array
    mqttClient.publish(mqtt_topic.c_str(), RGBpayload, payload_size);
    
    Serial.println("Published whole byte array after updating a single pixel.");
  } else {
    Serial.println("MQTT mqttClient not connected, cannot publish from *send_RGB_to_pixel*.");
  }
}

void send_all_off() {
  // Check if the mqttClient is connected before publishing
  if (mqttClient.connected()) {
    // Fill the byte array with the specified RGB color pattern
    for(int pixel=0; pixel < num_leds; pixel++){
      RGBpayload[pixel * 3 + 0] = (byte)0; // Red
      RGBpayload[pixel * 3 + 1] = (byte)0; // Green
      RGBpayload[pixel * 3 + 2] = (byte)0; // Blue
    }
    // Publish the byte array
    mqttClient.publish(mqtt_topic.c_str(), RGBpayload, payload_size);
    
    Serial.println("Published an all zero (off) byte array.");
  } else {
    Serial.println("MQTT mqttClient not connected, cannot publish from *send_all_off*.");
  }
}

void send_all_random() {
  // Check if the mqttClient is connected before publishing
  if (mqttClient.connected()) {
    // Fill the byte array with the specified RGB color pattern
    for(int pixel=0; pixel < num_leds; pixel++){
      RGBpayload[pixel * 3 + 0] = (byte)random(50,256); // Red 
      RGBpayload[pixel * 3 + 1] = (byte)random(50,256); // Green
      RGBpayload[pixel * 3 + 2] = (byte)random(50,256); // Blue
    }
    // Publish the byte array
    mqttClient.publish(mqtt_topic.c_str(), RGBpayload, payload_size);
    
    Serial.println("Published an all random byte array.");
  } else {
    Serial.println("MQTT mqttClient not connected, cannot publish from *send_all_random*.");
  }
}

void printMacAddress(byte mac[]) {
  for (int i = 5; i >= 0; i--) {
    if (mac[i] < 16) {
      Serial.print("0");
    }
    Serial.print(mac[i], HEX);
    if (i > 0) {
      Serial.print(":");
    }
  }
  Serial.println();
}

// hue to rgb, for blue-purple-pink loop
void hueToRGB(int hue, byte &r, byte &g, byte &b) {
  float h = (hue % 360) / 60.0;
  float x = 1 - fabs(fmod(h, 2) - 1);
  float R, G, B;

  if (h < 1) { R = 1; G = x; B = 0; }
  else if (h < 2) { R = x; G = 1; B = 0; }
  else if (h < 3) { R = 0; G = 1; B = x; }
  else if (h < 4) { R = 0; G = x; B = 1; }
  else if (h < 5) { R = x; G = 0; B = 1; }
  else { R = 1; G = 0; B = x; }

  r = (byte)(R * 255);
  g = (byte)(G * 255);
  b = (byte)(B * 255);
}

// breathing effect: blue > purple > pink > blue
void breathingEffect() {
  unsigned long now = millis();
  if (now - lastUpdate > 200) { // update color every ~ 0.2s
    lastUpdate = now;

    // Cycle the hue within the range of 200 to 320 degrees
    hue += 3;
    if (hue > 320) hue = 200;

    byte r, g, b;
    hueToRGB(hue, r, g, b);

    //  breathing brightness curve (sin shape)
    float brightness = (sin(millis() / 1000.0 * 3.14159) + 1.0) / 2.0; // 0~1
    r = (byte)(r * brightness);
    g = (byte)(g * brightness);
    b = (byte)(b * brightness);

    // Update the entire light strip
    for (int p = 0; p < num_leds; p++) {
      RGBpayload[p*3+0] = r;
      RGBpayload[p*3+1] = g;
      RGBpayload[p*3+2] = b;
    }

    if (mqttClient.connected()) {
      mqttClient.publish(mqtt_topic.c_str(), RGBpayload, payload_size);
    }

    Serial.print("breathing hue=");
    Serial.print(hue);
    Serial.print(" RGB=");
    Serial.print(r); Serial.print(",");
    Serial.print(g); Serial.print(",");
    Serial.println(b);
  }
}