# Power Logger for ESP32

## 概要

このレポジトリは、クランプ式電流センサーのためのコード一式をまとめたものです。エッジ側の ESP32 のための Arduino のコードとサーバ側の Python のコードの両方を含んでいます。データの通信には MQTT を利用しますので、Mosquitto 等の Message Broker の準備が必要になります。なお、こちらのコードは以下のブログ記事とリンクしたものになります。

https://note.com/ds_kotaro/n/nfaf4f6f1344e

## ディレクトリ

主なディレクトリは、以下の通りです。コードはサーバ側かエッジ側か、あるいは分析用のノートブックかでディレクトリを分けています。

### [power_sensor_mqtt]
このディレクトリには、エッジ側のコードを配置しています。基本的には Arduino IDE で power_sensor_mqtt.ino を開いて、コンパイルすれば OK です。WiFi や MQTT等の各種の設定は、同じディレクトリにある [Parameters.h] にて行います。Arduino IDE のインストール等については、他に詳しいサイトがありますので、適宜参照して下さい。

### [python]
このディレクトリには、サーバ側のコードを配置しています。 基本的には Python のコードで、次のようにして実行することで、エッジ側からのデータを CSV へロギングすることができます。

```
./recorder.py config.ini
```

あるいは

```
python3 recorder.py config.ini
```

MQTTに関する設定は、同じディレクトリにある [config.ini] にて行います。Python の環境構築に関しても、幾つもブログ記事等がありますので、ぜひそちらを参照して頂ければと思います。基本的には、Python をインストールした後で、次のような感じにすれば OK なはずです。


Linux の場合：
```
python3 -m venv venv
source ./venv/bin/activate
pip install -r requirements.txt
```

Windows の場合：
```
python3 -m venv venv
./venv/scripts/activate
pip install -r requirements.txt
```

### [notebooks]

こちらのディレクトリには、Jupyter Notebook を配置しています。

- [フィルタ設計]
- [データ可視化]

ESP32 のコードで使用しているローパスフィルタの計算や取得したデータの可視化に使った Notebook を置きました。


## パラメータの設定方法

プログラムを動かす際には、まず次のパラメータを決めて下さい。

- mqtt_client_id
- mqtt_topic_base

client_id は、デバイスの名前で複数のデバイスを区別するために使います。基本的には何でもかまわないですが、仮にここではデバイスが２つあるとして、"x1", "x2" という名前をそれぞれ付けることにします。

また、mqtt_topic_base を仮に "home/data/power" としておくことにすると、ESP32 の方の設定ファイル（[Parameters.h]）は次のような感じになります（"x1" として名前を付けた方）。

```
const char* wifi_ssid = "xxxxx";
const char* wifi_password = "xxxxx";

const char* mqtt_client_id = "x1";
const char* mqtt_topic_base = "home/data/power";
const char* mqtt_endpoint = "xxx.xxx.xxx.xxx";
const int mqtt_port = 1883;

const char* mqtt_user = "xxxxx";
const char* mqtt_password = "xxxxx";

```

このように設定すると、実際に ESP32 が MQTT でデータの送信するトピックは自動的に "home/data/power/x1" という風に設定されるようになります。また、データを受信する側の設定ファイル（[config.ini]）は、次のような感じになります。

```
[info]
client_id = x1

[mqtt]
topic_base = home/data/power
endpoint = xxx.xxx.xxx.xxx
port = 1883
user = xxxxx
password = xxxxx
```

このように設定したとき、recorder.py は "x1" として名前を付けたデバイスから送信されたデータのみを受信するようになります。その他の MQTT のパラメータについては、一般的なものと一緒ですので、詳細は省略します。


## 注意点

設定ファイルで設定できるのは、ソフトウェア周りのものだけですので、ハードウェアの構成を変更する際には、[PowerSensorMQTT.h] の以下の辺りを適宜修正して下さい。

```
const int baud_rate = 115200;
const int pin_ref = 2;
const int pin_ch1 = 3;
const int pin_ch2 = 4;

const float R = 51.0;
const float I_ratio = 2000.0;   // Ratio of currents
const float V_mean = 100.0;     // AC voltage in Japan
```

例えば、同じクランプ式の電流センサーでも検出する際の電流比が異なる場合がありますので、ご注意下さい。

- https://www.switch-science.com/products/840
- https://akizukidenshi.com/catalog/g/gP-08960/


## 免責事項

基本的にこちらのコードは自由にお使い頂くことができますが、直接・間接を問わず、こちらのコードに起因する事故・損害等については一切責任を負うことができませんので、各自の責任の範囲内でご利用下さい（感電の可能性のある電圧ですので、十分に注意して作業して下さい）。

[python]:https://github.com/eohta/PowerLoggerESP32/tree/main/python
[power_sensor_mqtt]:https://github.com/eohta/PowerLoggerESP32/tree/main/power_sensor_mqtt
[notebooks]:https://github.com/eohta/PowerLoggerESP32/tree/main/notebooks

[フィルタ設計]:https://github.com/eohta/PowerLoggerESP32/blob/main/notebooks/filter_design.ipynb
[データ可視化]:https://github.com/eohta/PowerLoggerESP32/blob/main/notebooks/power_analysis.ipynb

[power_sensor_mqtt.ino]:https://github.com/eohta/PowerLoggerESP32/blob/main/power_sensor_mqtt/power_sensor_mqtt.ino
[Parameters.h]:https://github.com/eohta/PowerLoggerESP32/blob/main/power_sensor_mqtt/Parameters.h
[config.ini]:https://github.com/eohta/PowerLoggerESP32/blob/main/python/config.ini
[PowerSensorMQTT.h]:https://github.com/eohta/PowerLoggerESP32/blob/main/power_sensor_mqtt/PowerSensorMQTT.h
