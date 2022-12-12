#! /usr/bin/env python
"""
Data Recorder
Created by Eiji Ota
"""

import paho.mqtt.client as mqtt
import datetime
import configparser
import sys

#
# Recorder Class
#

class Recorder:
    
    def __init__(self, config_file):

        self.setup_params(config_file)
        self.setup_mqtt()


    def setup_params(self, config_file):

        config = configparser.ConfigParser()
        config.read(config_file)

        self.client_id = config['info']['client_id']
        self.mqtt_topic_base = config['mqtt']['topic_base']
        self.mqtt_endpoint = config['mqtt']['endpoint']
        self.mqtt_port = int(config['mqtt']['port'])
        self.mqtt_user = config['mqtt']['user']
        self.mqtt_password = config['mqtt']['password']
        self.mqtt_sub_topic = self.mqtt_topic_base + '/' + self.client_id

        now = datetime.datetime.now()
        self.logfile = './log/log_' + now.strftime('%Y%m%d_%H%M%S') + '.csv'  


    def setup_mqtt(self):
        
        self.client = mqtt.Client()
        self.client.username_pw_set(self.mqtt_user, self.mqtt_password)
                
        self.client.on_connect = self.mqtt_on_connect
        self.client.on_disconnect = self.mqtt_on_disconnect
        self.client.on_message = self.mqtt_on_message
        
        self.client.connect(self.mqtt_endpoint, self.mqtt_port)
        
        
    def start(self):
        
        self.client.connect(self.mqtt_endpoint, self.mqtt_port)
        self.client.loop_forever()
        
        
    def mqtt_on_connect(self, client, userdata, flag, rc):
        
        if rc == 0:
        
            print('Successfully connected to server')
            client.subscribe(self.mqtt_sub_topic)
        
        else:
        
            print('Connection failed...')
            
            
    def mqtt_on_disconnect(self, client, userdata, rc):
        
        if rc != 0:
        
            print('Unexpected disconnection')


    def mqtt_on_message(self, client, userdata, msg):

        with open(self.logfile, mode='a') as f:

            now = datetime.datetime.now()
            line = str(now) + ', ' + msg.payload.decode() + '\n'

            f.write(line)


#
# Main
#

def main(config_file='./config.ini'):
    
    recorder = Recorder(config_file)
    recorder.start()    


if __name__ == '__main__':
    
    if len(sys.argv) > 1:
        
        main(sys.argv[1])
    
    else:
        
        main()
