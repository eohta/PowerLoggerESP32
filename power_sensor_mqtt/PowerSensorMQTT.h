/*
 * PowerSensorMQTT.h
 *
 * Created by Eiji Ota
 */

#ifndef POWER_SENSOR_MQTT_H
#define POWER_SENSOR_MQTT_H

#include <deque>
#include <numeric>
#include <string>
#include <sstream>

#include <WiFi.h>
#include <PubSubClient.h>

//
// Parameters
//

const int baud_rate = 115200;
const int pin_ref = 2;
const int pin_ch1 = 3;
const int pin_ch2 = 4;

const float R = 51.0;
const float I_ratio = 2000.0;   // Ratio of currents
const float V_mean = 100.0;     // AC voltage in Japan

const int report_period = 5000; // Report Period
const int buffer_size = 5000;   // Buffer Size

const float tap[45] = {\
 0.001100, 0.000754,-0.000000,-0.001197,-0.002573,
-0.003424,-0.002791, 0.000000, 0.004661, 0.009554,
 0.011968, 0.009182,-0.000000,-0.013916,-0.027657,
-0.034098,-0.026224, 0.000000, 0.043345, 0.096685,
 0.148498, 0.186163, 0.199937, 0.186163, 0.148498,
 0.096685, 0.043345, 0.000000,-0.026224,-0.034098,
-0.027657,-0.013916,-0.000000, 0.009182, 0.011968,
 0.009554, 0.004661, 0.000000,-0.002791,-0.003424,
-0.002573,-0.001197,-0.000000, 0.000754, 0.001100};

const int num_taps = 45;

//
// Variables
//

String _mqtt_client_id;
String _mqtt_pub_topic;
String _mqtt_user;
String _mqtt_password;

unsigned long _connection_last_tried = 0;
unsigned long _millis_message_published = 0;

//
// Buffers
//

std::deque<float> q_I1;
std::deque<float> q_I2;
std::deque<float> q_U1;
std::deque<float> q_U2;

//
// Objects
//

WiFiClient client_wifi;
PubSubClient client;


// ************************************************************* //
//
//    Sensor Functions : Setup / Measuring
//
// ************************************************************* //

void measure_power(void){

  int V_raw_ref = analogRead(pin_ref);
  int V_raw_ch1 = analogRead(pin_ch1);
  int V_raw_ch2 = analogRead(pin_ch2);

  float V_diff_ch1 = (float)(V_raw_ch1 - V_raw_ref) / 4096.0 * 3.3;
  float V_diff_ch2 = (float)(V_raw_ch2 - V_raw_ref) / 4096.0 * 3.3;

  float I1 = V_diff_ch1 / R * I_ratio;
  float I2 = V_diff_ch2 / R * I_ratio;

  q_I1.push_back(I1);
  q_I2.push_back(I2);

  if (q_I1.size() > num_taps){

    q_I1.pop_front();
    q_I2.pop_front();

    float I1_filtered = 0.0;
    float I2_filtered = 0.0;

    for (int k=0; k<num_taps; k++){

      I1_filtered += q_I1[k] * tap[k];
      I2_filtered += q_I2[k] * tap[k];
    }

    q_U1.push_back(I1_filtered * I1_filtered);
    q_U2.push_back(I2_filtered * I2_filtered);    
    
    if (q_U1.size() > buffer_size){

      q_U1.pop_front();
      q_U2.pop_front();
    }
  }
}


// ************************************************************* //
//
//      Communication Functions : MQTT / WiFi / Serial
//
// ************************************************************* //

//
// Setup TOPIC
//

void setup_topic(const String mqtt_topic_base, const String mqtt_client_id){

  _mqtt_client_id = mqtt_client_id;
  _mqtt_pub_topic = mqtt_topic_base + "/" + mqtt_client_id;
}

//
// Setup Serial
//

void setup_serial(const unsigned int baud_rate){

  Serial.begin(baud_rate);
  Serial.flush();
  delay(100);
}

//
// Connection to WiFi network
//

void setup_wifi(const char* wifi_ssid, const char* wifi_password){

  Serial.print("\nConnecting to : ");
  Serial.print(wifi_ssid);

  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) delay(1000);

  Serial.println(" ---> WiFi Connected!");
}

//
// Setup MQTT
//

void setup_mqtt_params(const char* mqtt_endpoint, const int mqtt_port, const int mqtt_max_packet_size = 100, const int mqtt_keep_alive = 60){

  Serial.print("MQTT endpoint is set to : ");
  Serial.println(mqtt_endpoint);

  client.setClient(client_wifi);

  client.setBufferSize(mqtt_max_packet_size);
  client.setKeepAlive(mqtt_keep_alive);
  client.setServer(mqtt_endpoint, mqtt_port);
}

void setup_mqtt_account(const char* mqtt_user, const char* mqtt_password){

  _mqtt_user = String(mqtt_user);
  _mqtt_password = String(mqtt_password);
}

//
// (Re)connection to Message Broker (MQTT)
//

void connect_message_broker(void){
  
  unsigned long now = millis();
  
  if (now - _connection_last_tried > 5000) {

    bool is_connected;

    if (_mqtt_user.length() > 0){
      
      is_connected = client.connect(_mqtt_client_id.c_str(), _mqtt_user.c_str(), _mqtt_password.c_str());
    }
    else {

      is_connected = client.connect(_mqtt_client_id.c_str());
    }

    Serial.print("Connecting Message Broker");

    if (is_connected) {

      Serial.println(" ---> Server Connected!");
      _connection_last_tried = 0;
    }
    else {

      Serial.println(" ---> Connection failed!");
      _connection_last_tried = now;
    }
  }
}

//
// Connection Check
//

void keep_connected(void){

  if (!client.connected()) {

    connect_message_broker();
  }
  else {

    client.loop();
  }
}

//
// Publish Message
//

void publish_message(void){

  bool cond = (millis() - _millis_message_published) > report_period;

  if (cond) {
    
    float U1_mean = std::accumulate(q_U1.begin(), q_U1.end(), 0.0) / (float)q_U1.size();
    float U2_mean = std::accumulate(q_U2.begin(), q_U2.end(), 0.0) / (float)q_U2.size();

    float P1_mean = sqrt(U1_mean) * V_mean;
    float P2_mean = sqrt(U2_mean) * V_mean;

    std::ostringstream oss;
    oss << P1_mean << ", " << P2_mean;

    client.publish(_mqtt_pub_topic.c_str(), oss.str().c_str());

    _millis_message_published = millis();
  }
}

#endif
