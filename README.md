# RFID-em-C-IoT
Projeto desenvolvido usando um ESP8266 e um módulo RC522 para ler o cartão.
Leitor de Tag RFID que utilizam o protocolo Mifare e enviar o Id das tags juntamente com o horário de leitura, para o site da Hive usando o protocolo de comunicação MQTT. 
##########################################################################################################################################################################
lado esquerdo RC522, lado direito ESP8266
Vcc <-> 3V3 
RST (Reset) <-> D0
GND (Terra) <-> GND
MISO (Master Input Slave Output) <-> D6
MOSI (Master Output Slave Input) <-> D7
SCK (Serial Clock) <-> D5
SS/SDA (Slave select) <-> D8

Bibliotecas instaladas pela IDE:
NTPClient by Fabrice, Versão: 3.2.1
MRFC522 by GithubCommunity, Versão: 1.4.10
PubSubClient by Nick O'Leary , Versão: 2.8 
