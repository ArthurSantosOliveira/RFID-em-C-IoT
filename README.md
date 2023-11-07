# RFID-em-C-IoT
Projeto desenvolvido usando um ESP8266 e um módulo RC522 para ler o cartão.
Leitor de Tag RFID que utilizam o protocolo Mifare e enviar o Id das tags juntamente com o horário de leitura, para o site da Hive usando o protocolo de comunicação MQTT. 
O processo de utilização do Node é feito como padrão utilizada em qualquer outro tutorial na internet.
##

Lado esquerdo RC522, lado direito ESP8266
Vcc <-> 3V3 

RST (Reset) <-> D0

GND (Terra) <-> GND

MISO (Master Input Slave Output) <-> D6

MOSI (Master Output Slave Input) <-> D7

SCK (Serial Clock) <-> D5

SS/SDA (Slave select) <-> D8

Bibliotecas instaladas pela IDE: 
NTPClient by Fabrice, Versão: 3.2.1 - 
MRFC522 by GithubCommunity, Versão: 1.4.10 - 
PubSubClient by Nick O'Leary , Versão: 2.8 - 

As bibliotecas .zip estão na pasta bibliotecas.
Algumas dessas bibliotecas podem não estar sendo utilizada na versão final do código, entratando foram usadas no seu desenvolvimento.

Estarei disponibilando também o drive CH34X que utilizei para resolver o problema que tive com relação a porta UBS -> https://sparks.gogo.co.nz/ch340.html
