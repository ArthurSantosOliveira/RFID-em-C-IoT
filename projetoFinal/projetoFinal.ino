#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <MFRC522.h>
#include <time.h>  
#include <WiFiUdp.h>  
#include <TimeLib.h>

#define SS_PIN D8
#define RST_PIN D0

// Pacote do NTP  
typedef struct {  
  uint8_t ctrl;       // LI/VN/Mode  
  uint8_t stratum;  
  uint8_t poll;  
  uint8_t precision;  
  uint8_t rootDelay[4];  
  uint8_t rootDispersion[4];  
  uint8_t refIdent[4];  
  uint8_t refTimestamp[8];  
  uint8_t orgTimestamp[8];  
  uint8_t recTimestamp[8];  
  uint8_t txmTimestamp[8];  
} PKT_NTP;  
  
PKT_NTP pktTx;  // pacote enviado ao servidor  
PKT_NTP pktRx;  // pacore recebido do servidor  
  
// Controle dos tempos da tentativa  
const uint32_t MIN_TENTATIVA = 30000;  
const uint32_t MAX_TENTATIVA = 180000;  
uint32_t intervTentativa = MIN_TENTATIVA;  
uint32_t proxTentativa = 0;  
  
// Controle dos tempos de atualização  
uint32_t ultAtualizacao = 0;  
const uint32_t intervAtualizacao = 60000; // 1 minuto em milisegundos  
  
// Timestamp local  
uint32_t timestamp = 0;   

//Conexão Hive e Wifi
const char* ssid = "";
const char* password = "";
const char* mqtt_server = "";
const char* mqtt_username = "";
const char* mqtt_password = "";
const int mqtt_port = 8883;
const char* mqtt_topic = "rfid_tags";
static const char *root_ca PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc
oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=
-----END CERTIFICATE-----
)EOF";
MFRC522 rfid(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;

WiFiClientSecure espClient;
PubSubClient client(espClient);

// Servidor NTP  
const char* servidorNTP = "a.ntp.br";   
const int NTP_PORT = 123;  
const int LOCAL_PORT = 1234;  
  
 // Ajuste para o fuso horário (UTC-3)  
const uint32_t epochUnix = 2208988800UL;  
const int fusoHorario = -10800;           // 12080em segundos  
  
// Acesso ao UDP  
WiFiUDP udp;  
  
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];

void setup() {
  Serial.begin(115200);
  SPI.begin();
  rfid.PCD_Init();
  sntpInit();  

  setup_wifi();

#ifdef ESP8266
  espClient.setInsecure();
#endif

  client.setServer(mqtt_server, mqtt_port);
}

void setup_wifi() {
  delay(10);
  Serial.print("\nConnecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());
  Serial.println("\nWiFi connected\nIP Address: ");
  Serial.println(WiFi.localIP());
}

// Iniciação do acesso ao SNTP  
void sntpInit () {  
  memset (&pktTx, 0, sizeof(pktTx));  
  pktTx.ctrl = (4 << 3) | 3;  // Versão 4, Modo 3 (client)  
  if (!udp.begin (LOCAL_PORT)) {  
    Serial.println ("Erro ao iniciar UDP");  
  }  
}  

// Trata atualização periódica do timestamp local  
void sntpUpdate() {  
  if (millis() > proxTentativa) {  
    uint32_t tempoDesdeAtualizacao = (millis() - ultAtualizacao) / 1000UL;  
    Serial.print (timestamp+ tempoDesdeAtualizacao);  
    // Envia a solicitação  
    putUInt32 (pktTx.txmTimestamp, timestamp+tempoDesdeAtualizacao);  
    udp.beginPacket(servidorNTP, NTP_PORT);  
    udp.write((uint8_t *)&pktTx, sizeof(pktTx));  
    udp.endPacket();  
    // Espera a resposta  
    int timeout = 0;  
    int cb = 0;  
    do {  
      delay ( 10 );  
      cb = udp.parsePacket();  
      if (timeout > 100) {  
        Serial.println (" - Sem resposta");  
        proxTentativa = millis() + intervTentativa;  
        if (intervTentativa < MAX_TENTATIVA) {  
          intervTentativa += intervTentativa;  
        }  
        return; // timeout de um segundo  
      }  
      timeout++;  
    } while (cb == 0);  
    intervTentativa = MIN_TENTATIVA;  
    // Le a resposta  
    udp.read((uint8_t *) &pktRx, sizeof(pktRx));  
    // Consistência básica  
    if (((pktRx.ctrl & 0x3F) != ( (4 << 3) | 4)) ||  
        ((pktRx.ctrl & 0xC0) == (3 << 6)) ||  
        (pktRx.stratum == 0) ||  
        (memcmp(pktRx.orgTimestamp, pktTx.txmTimestamp, 4) != 0)) {  
      Serial.print (" - Resposta invalida");  
      proxTentativa = millis() + intervTentativa;  
      if (intervTentativa < MAX_TENTATIVA) {  
        intervTentativa += intervTentativa;  
      }  
      return;  
    }  
    // Pega o resultado  
    ultAtualizacao = millis();  
    proxTentativa = ultAtualizacao + intervAtualizacao;  
    timestamp = getUInt32 (pktRx.txmTimestamp);  
    time_t hora = sntpTime();  
    Serial.print (" - ");  
    Serial.print (timestamp);  
    Serial.print (" - ");  
    struct tm *ptm = gmtime(&hora);  
    Serial.print (ptm->tm_mday);  
    Serial.print ("/");  
    Serial.print (ptm->tm_mon + 1);  
    Serial.print ("/");  
    Serial.print (ptm->tm_year + 1900);  
    Serial.print (" ");  
    Serial.print (ptm->tm_hour);  
    Serial.print (":");  
    Serial.print (ptm->tm_min);  
    Serial.print (":");  
    Serial.println (ptm->tm_sec);  
  }  
}  

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);

    if (client.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("Connected");
      client.subscribe("led_state");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" Retrying in 5 seconds");
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    Serial.println(F("Card detected"));

    // Read the UID of the card
    String cardUid = "";
    for (byte i = 0; i < rfid.uid.size; i++) {
      cardUid += String(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
      cardUid += String(rfid.uid.uidByte[i], HEX);
    }
    Serial.print(F("Card UID: "));
    Serial.println(cardUid);


  if (millis() > proxTentativa) {  
    uint32_t tempoDesdeAtualizacao = (millis() - ultAtualizacao) / 1000UL;  
    Serial.print (timestamp+ tempoDesdeAtualizacao);  
    // Envia a solicitação  
    putUInt32 (pktTx.txmTimestamp, timestamp+tempoDesdeAtualizacao);  
    udp.beginPacket(servidorNTP, NTP_PORT);  
    udp.write((uint8_t *)&pktTx, sizeof(pktTx));  
    udp.endPacket();  
    // Espera a resposta  
    int timeout = 0;  
    int cb = 0;  
    do {  
      delay ( 10 );  
      cb = udp.parsePacket();  
      if (timeout > 100) {  
        Serial.println (" - Sem resposta");  
        proxTentativa = millis() + intervTentativa;  
        if (intervTentativa < MAX_TENTATIVA) {  
          intervTentativa += intervTentativa;  
        }  
        return; // timeout de um segundo  
      }  
      timeout++;  
    } while (cb == 0);  
    intervTentativa = MIN_TENTATIVA;  
    // Le a resposta  
    udp.read((uint8_t *) &pktRx, sizeof(pktRx));  
    // Consistência básica  
    if (((pktRx.ctrl & 0x3F) != ( (4 << 3) | 4)) ||  
        ((pktRx.ctrl & 0xC0) == (3 << 6)) ||  
        (pktRx.stratum == 0) ||  
        (memcmp(pktRx.orgTimestamp, pktTx.txmTimestamp, 4) != 0)) {  
      Serial.print (" - Resposta invalida");  
      proxTentativa = millis() + intervTentativa;  
      if (intervTentativa < MAX_TENTATIVA) {  
        intervTentativa += intervTentativa;  
      }  
      return;  
    }  
    // Pega o resultado  
    ultAtualizacao = millis();  
    proxTentativa = ultAtualizacao + intervAtualizacao;  
    timestamp = getUInt32 (pktRx.txmTimestamp);  
    time_t hora = sntpTime();  
    Serial.print (" - ");  
    Serial.print (timestamp);  
    Serial.print (" - ");  
    struct tm *ptm = gmtime(&hora);  
    Serial.print (ptm->tm_mday);  
    Serial.print ("/");  
    Serial.print (ptm->tm_mon + 1);  
    Serial.print ("/");  
    Serial.print (ptm->tm_year + 1900);  
    Serial.print (" ");  
    Serial.print (ptm->tm_hour);  
    Serial.print (":");  
    Serial.print (ptm->tm_min);  
    Serial.print (":");  
    Serial.println (ptm->tm_sec);  
  }  
    time_t timestampLocal = sntpTime();

    char formattedTime[30];
    strftime(formattedTime, sizeof(formattedTime), "%d/%m/%Y %H:%M:%S", localtime(&timestampLocal));


    // Publish card UID and formatted timestamp to the MQTT topic

    String payload = "ID: " + cardUid + " Data e hora: " + formattedTime ;
    client.publish(mqtt_topic, payload.c_str());

    // Halt PICC and stop crypto1
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
  }

  delay(1000);
}


time_t sntpTime() {
    uint32_t tempoDesdeAtualizacao = millis() - ultAtualizacao;  
    uint32_t tempoUTC = timestamp + tempoDesdeAtualizacao/1000;  
    return (time_t) (tempoUTC - epochUnix + fusoHorario);  
  return 0;
}
// Rotinas para mover uint32_t de/para os pacotes  
void putUInt32 (uint8_t *p, uint32_t val) {  
  p[0] = (uint8_t) ((val >> 24) & 0xFF);  
  p[1] = (uint8_t) ((val >> 16) & 0xFF);  
  p[2] = (uint8_t) ((val >> 8) & 0xFF);  
  p[3] = (uint8_t) (val & 0xFF);  
}  
uint32_t getUInt32 (uint8_t *p) {  
  return (((uint32_t) p[0]) << 24) |  
         (((uint32_t) p[1]) << 16) |  
         (((uint32_t) p[2]) << 8) |  
         ((uint32_t) p[3]);  
}  
