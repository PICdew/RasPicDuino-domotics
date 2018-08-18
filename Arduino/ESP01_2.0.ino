#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <stdlib.h>

//DECLARACIO DE VARIABLES
//WiFi
char *wifi_ssid = "CASA COSTABELLANOGUERA"; //your WiFi Name
char *wifi_password = "0123456789ABCDEF";   //Your Wifi Password

//MQTT
const char *mqtt_server = "192.168.0.2";
const char *mqtt_topic = "modul";
const char *mqtt_username = "username";
const char *mqtt_password = "esp8266";
const char *clientID = "ESP8266_1"; //identificador del client

//variables
int temps = 0;     //temps de conexio
int SensorLDR;     //Sensor de llum
int SensorTMP;     //dades Temperatura
int SensorPIR;     //dades sensor pir
int APosicioP1;    //posicio actual persiana 1
int APosicioP2;    //posicio actual persiana 2
int DPosicioP1;    //posicio desitjada persiana 1
int DPosicioP2;    //posicio desitjada persiana 2
int DvalorL1;      //llum 1 valor desitjat
int DvalorL2;      //llum 2 valor desitjat
int DvalorL3;      //llum 3 valor desitjat
int RvalorL1;      //llum 1 valor real
int RvalorL2;      //llum 2 valor real
int RvalorL3;      //llum 3 valor real
String DvalorOUT1; //sortida 1 valor desitjat
String DvalorOUT2; //sortida 2 valor desitjat
int RvalorOUT1;    //sortida 1 valor real
int RvalorOUT2;    //sortida 2 valor real

//Inicialitzacio del WiFi i MQTT Client
WiFiServer server(80); //obre port 80
WiFiClient wifiClient;
PubSubClient client(mqtt_server, 1883, wifiClient); //obre el port 1883 per utilitzarlo pel MQTT

//----------------------------------
//FUNCIONS
//canvi de ssid i de password
int wifi_change_ssid_pass()
{
  if (wifi_ssid == "CASA COSTABELLANOGUERA")
  {
    wifi_ssid = "UdGeeks-Conceptronic";
    wifi_password = "lamuambela";
    return 0;
  }
  else if (wifi_ssid == "UdGeeks-Conceptronic")
  {
    wifi_ssid = "CostistarFusion";
    wifi_password = "QwKX3NRP";
    return 0;
  }
  return 1;
}

//funcio per connectar-se a un wifi
int wifi_connection()
{
  int retorn = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    bool wifi = LOW;
    Serial.print("Connecting to ");
    Serial.println(wifi_ssid);
    WiFi.begin(wifi_ssid, wifi_password);
    int temps0 = millis();
    Serial.println(temps0);
    while (WiFi.status() != WL_CONNECTED and wifi == LOW)
    {
      delay(500);
      Serial.print(".");
      int temps1 = millis();
      int diftemps = temps1 - temps0;
      Serial.println(diftemps);
      if (diftemps > 5000)
      {
        retorn = wifi_change_ssid_pass();
        wifi = HIGH;
        if (retorn == 1)
        {
          return 1;
        }
      }
    }
  }
  return 0;
}

//funcions per la comunicacio entra PIC i ESP-01
int EUSART_connection_R(const char *tipos, const char *numero)
{
  int Retorn;
  Serial.println(tipos);
  for (int i = 0; i < 100; i++)
  {
    delay(10);
    if (Serial.available() > 0)
    {
      if (Serial.readStringUntil('\n') == "Quin")
      {
        Serial.println(numero);
        for (int n = 0; n < 100; n++)
        {
          delay(10);
          if (Serial.available() > 0)
          {
            if (Serial.readStringUntil('\n') == "RorW")
            {
              Serial.println("R");
              for (int o = 0; o < 100; o++)
              {
                delay(10);
                if (Serial.available() > 0)
                {
                  Retorn = Serial.parseInt();
                  o = 100;
                  n = 100;
                  i = 100;
                  Serial.println("rebut");
                  return Retorn;
                }
              }
            }
          }
        }
      }
    }
  }
}

int EUSART_connection_W(const char *tipos, const char *numero, const int dades)
{
  int Retorn;
  Serial.println(tipos);
  for (int i = 0; i < 100; i++)
  {
    delay(10);
    if (Serial.available() > 0)
    {
      if (Serial.readStringUntil('\n') == "Quin")
      {
        Serial.println(numero);
        for (int n = 0; n < 100; n++)
        {
          delay(10);
          if (Serial.available() > 0)
          {
            if (Serial.readStringUntil('\n') == "RorW")
            {
              Serial.println("W");
              Serial.write(dades);
              n = 100;
              i = 100;
              return 0;
            }
          }
        }
      }
    }
  }
  return 1;
}

//funcio recepcio de missatges via wifi
void ReceivedMessage(char *topic, byte *payload, unsigned int length)
{
  // Output the first character of the message to serial (debug)
  char message;
  char value;
  int i = 0;

  //Obtencio de valor
  do
  {
    Serial.println((char)payload[i]);
    message = message + (char)payload[i];
    i++;
  } while ((char)payload != '\\');
  i++;

  do
  {
    Serial.print((char)payload[i]);
    value = value + (char)payload[i];
    i++;
  } while ((char)payload != '\\');

  if (message == "LLUML1")
  {
    DvalorL1 = value.toInt();
    EUSART_connection_W("LLUM", "1", DvalorL1);
  }
  else if (message == "LLUML2")
  {
    DvalorL2 = value.toInt();
    EUSART_connection_W("LLUM", "2", DvalorL2);
  }
  else if (message == "LLUML3")
  {
    DvalorL3 = value.toInt();
    EUSART_connection_W("LLUM", "3", DvalorL3);
  }
  else if (message == "PERSIANA1")
  {
    DPosicioP1 = value.toInt();
    EUSART_connection_W("PERSIANA", "1", DPosicioP1);
  }
  else if (message == "PERSIANA2")
  {
    DPosicioP2 = value.toInt();
    EUSART_connection_W("PERSIANA", "2", DPosicioP2);
  }
  else if (message == "OUT1")
  {
    EUSART_connection_W("OUT", "1", DvalorOUT1);
  }
  else if (message == "OUT2")
  {
    EUSART_connection_W("OUT", "2", DvalorOUT2);
  }
  else if (message == "DADES"){
    client.publish(mqtt_topic, "LDR:");
    client.publish(mqtt_topic, SensorLDR);
    client.publish(mqtt_topic, "TMP:");
    client.publish(mqtt_topic, SensorTMP);
    client.publish(mqtt_topic, "PIR:");
    client.publish(mqtt_topic, SensorPIR);
    client.publish(mqtt_topic, "AposicioP1:");
    client.publish(mqtt_topic, AposicioP1);
    client.publish(mqtt_topic, "AposicioP2:");
    client.publish(mqtt_topic, AposicioP2);
    client.publish(mqtt_topic, "RvalorL1:");
    client.publish(mqtt_topic, RvalorL1);
    client.publish(mqtt_topic, "RvalorL2:");
    client.publish(mqtt_topic, RvalorL2);
    client.publish(mqtt_topic, "RvalorL3:");
    client.publish(mqtt_topic, RvalorL3);
    client.publish(mqtt_topic, "RvalorOUT1:");
    client.publish(mqtt_topic, RvalorOUT1);
    client.publish(mqtt_topic, "RvalorOUT2:");
    client.publish(mqtt_topic, RvalorOUT2);
}

//Connectem amb el MQTT Server
bool Connect() {
  //Connect to MQTT Server and subscribe to the topic
  if (client.connect(clientID, mqtt_username, mqtt_password)) {
    client.subscribe(mqtt_topic);
    return true;
  }
  else {
    return false;
  }
}
//-----------------------------------------------------
//execucio del principi del programa
void setup()
{
  //Inicialitzacio de variables
  int retorn = 0;

  //inicialitzacio del port serial
  Serial.begin(115200);
  Serial.setTimeout(10);
  delay(10);
  Serial.print("NOCONNECTION"); //definim per seguretat ja que en al pic determina totes les sortides "OUT" off
  //conectem a un wifi
  retorn = wifi_connection();

  if (retorn == 0)
  {
    Serial.println("");
    Serial.println("WiFi connected");
    server.begin();
    Serial.println("Server started");
    Serial.print("Use this URL to connect: ");
    Serial.print("http://");
    Serial.print(WiFi.localIP());
    Serial.println("/");
  }
  else
  {
    Serial.print("NOCONNECTION");
  }
  //Connectem amb el canal MQTT
  //setCallback determinar que la funcio ReceivedMessage serà cridada cada cop que es revi un missatge
  client.setCallback(ReceivedMessage);
  Connect();
}

//programa loop
void loop()
{
  //Si perdem la conexió tornem a intentar la conexio
  if (!client.connected()) {
    Connect();
  }
  //Executa tot el que ha de fer, enviar missatges, rebrels, etc.
  client.loop();
  //Una vegada ha fet tot el cycle, comprova si segueix connectat.
  //comprovacio de la connexio
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.print("NOCONNECTION");
    wifi_ssid = "CASA COSTABELLANOGUERA";
    wifi_password = "0123456789ABCDEF";
    wifi_connection();
  }

  //---------------------------------------------
  //demanar valors sensors
  Serial.println("SensorLDR");
  for (int i = 0; i < 100; i++)
  {
    delay(10);
    if (Serial.available() > 0)
    {
      //lectura SensorLDR
      SensorLDR = Serial.parseInt();
      Serial.println("rebut");
      i = 100;
    }
  }

  Serial.println("SensorTMP");
  for (int i = 0; i < 10; i++)
  {
    delay(1000);
    if (Serial.available() > 0)
    {
      //lectura SensorTMP
      SensorTMP = Serial.parseInt();
      i = 100;
      Serial.println("rebut");
    }
  }

  Serial.println("SensorPIR");
  for (int i = 0; i < 10; i++)
  {
    delay(1000);
    if (Serial.available() > 0)
    {
      //lectura PIR
      SensorPIR = Serial.parseInt();
      i = 10;
      Serial.println("rebut");
    }
  }

  //Situacio de les persianes
  //llegeix el valor actual de la persiana 1
  APosicioP1 = EUSART_connection_R("PERSIANA", "1");
  //llegeix del PIC la posicio actual de la persiana 2
  APosicioP2 = EUSART_connection_R("PERSIANA", "2");

  //Situacio dels llums
  //llegeix el valor actual del llum L1
  RvalorL1 = EUSART_connection_R("LLUM", "1");
  //llegeix el valor actual de la llum L2
  RvalorL2 = EUSART_connection_R("LLUM", "2");
  //llegeix el valor actual del llum L3
  RvalorL3 = EUSART_connection_R("LLUM", "3");

  //Sitaucio dels OUT
  //llegeix el valor OUT1
  RvalorOUT1 = EUSART_connection_R("OUT", "1");
  //llegeix el valor OUT2
  RvalorOUT2 = EUSART_connection_R("OUT", "2");
}
