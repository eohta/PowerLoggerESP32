/*
 * power_sensor_mqtt.ino
 *
 * Created by Eiji Ota
 */

#include <Arduino.h>
#include "PowerSensorMQTT.h"
#include "Parameters.h"

void setup() {

  setup_serial(baud_rate);
  setup_wifi(wifi_ssid, wifi_password);
  
  setup_topic(mqtt_topic_base, mqtt_client_id);
  setup_mqtt_params(mqtt_endpoint, mqtt_port);
  setup_mqtt_account(mqtt_user, mqtt_password);
}

void loop() {

  //
  // (Re)connect Message Broker
  //
  
  keep_connected();

  //
  // Measure Power
  //
  
  measure_power();

  //
  // Publish Message
  //

  publish_message();

  delay(1);
}
