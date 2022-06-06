/*
 * DHT11 CON OPENHAB
 * por: Alexander Arroyo Núñez
 * Fecha: 03 de junio de 2022
 * 
 * Este programa envía los datos del DHT11 por Internet a través del protocolo MQTT. Para poder
 * comprobar el funcionamiento de este programa, es necesario conectarse a un broker
 * y usar OPENHAB para visualzar que la información se está recibiendo correctamente.
 * Este programa no requiere componentes adicionales.
 * 
 * Componente     PinESP32CAM     Estados lógicos
 * ledStatus------GPIO 33---------On=>LOW, Off=>HIGH
 * ledFlash-------GPIO 4----------On=>HIGH, Off=>LOW
 */

//Bibliotecas
#include <WiFi.h>  // Biblioteca para el control de WiFi
#include <PubSubClient.h> //Biblioteca para conexion MQTT

#include "DHT.h"          // AGREGADO PARA EL DHT11

//Constantes
#define DHTPIN 12   
#define DHTTYPE DHT11



//Datos de WiFi
//const char* ssid = "Ubee74C8-2.4G";  //ESTO Aquí debes poner el nombre de tu red
//const char* password = "5F99F574C8";  //ESTO Aquí debes poner la contraseña de tu red

const char* ssid = "MECA-ALEX";
const char* password = "M3C44L3X2022";

//const char* ssid = "INFINITUMAA9C_2.4";
//const char* password = "2cERga8H4V";

//const char* ssid = "TP-LINK_AP_641FF0";
//const char* password = "";

//Datos del broker MQTT

const char* mqtt_server = "172.17.51.115"; //ESTO Si estas en una red local, coloca la IP asignada, en caso contrario, coloca la IP publica
IPAddress server(172,17,51,115);  //ESTO

//const char* mqtt_server = "192.168.1.196"; //ESTO Si estas en una red local, coloca la IP asignada, en caso contrario, coloca la IP publica
//IPAddress server(192,168,1,196);  //ESTO

//const char* mqtt_server = "172.17.51.165"; //ESTO Si estas en una red local, coloca la IP asignada, en caso contrario, coloca la IP publica
//IPAddress server(172,17,51,165);  //ESTO

// Objetos
WiFiClient espClient; // Este objeto maneja los datos de conexion WiFi
PubSubClient client(espClient); // Este objeto maneja los datos de conexion al broker

DHT dht(DHTPIN, DHTTYPE); // AGREGADO PARA EL DHT11

// Variables
int flashLedPin = 4;  // Para indicar el estatus de conexión
int statusLedPin = 33; // Para ser controlado por MQTT
long timeNow, timeLast; // Variables de control de tiempo no bloqueante
int data = 0; // Contador
int wait = 10000;  // Indica la espera cada 5 segundos para envío de mensajes MQTT

// Inicialización del programa
void setup() 
{
  // Iniciar comunicación serial
  Serial.begin (115200);
  pinMode (flashLedPin, OUTPUT);
  pinMode (statusLedPin, OUTPUT);
  digitalWrite (flashLedPin, LOW);
  digitalWrite (statusLedPin, HIGH);

  dht.begin();    // AGREGADO PARA EL DHT11

  Serial.println();
  Serial.println();
  Serial.print("Conectar a ");
  Serial.println(ssid);
 
  WiFi.begin(ssid, password); // Esta es la función que realiz la conexión a WiFi
 
  while (WiFi.status() != WL_CONNECTED) { // Este bucle espera a que se realice la conexión
    digitalWrite (statusLedPin, HIGH);
    delay(500); //dado que es de suma importancia esperar a la conexión, debe usarse espera bloqueante
    digitalWrite (statusLedPin, LOW);
    Serial.print(".");  // Indicador de progreso
    delay (5);
  }
  
  // Cuando se haya logrado la conexión, el programa avanzará, por lo tanto, puede informarse lo siguiente
  Serial.println();
  Serial.println("WiFi conectado");
  Serial.println("Direccion IP: ");
  Serial.println(WiFi.localIP());

  // Si se logro la conexión, encender led
  if (WiFi.status () > 0){
  digitalWrite (statusLedPin, LOW);
  }
  
  delay (1000); // Esta espera es solo una formalidad antes de iniciar la comunicación con el broker

  // Conexión con el broker MQTT
  client.setServer(mqtt_server, 1883); // Conectarse a la IP del broker en el puerto indicado
  client.setCallback(callback); // Activar función de CallBack, permite recibir mensajes MQTT y ejecutar funciones a partir de ellos
  delay(1500);  // Esta espera es preventiva, espera a la conexión para no perder información

  timeLast = millis (); // Inicia el control de tiempo
}// fin del void setup ()

// Cuerpo del programa, bucle principal
void loop() 
{
  float t = dht.readTemperature();    // AGREGADO PARA EL DHT11
  float h = dht.readHumidity();
  
  if ( isnan(t) || isnan(h)) {
    Serial.println(F("No hay conexion"));
    return;
  } 
  Serial.print(F("Temperatura en °C: "));
  Serial.println(t);
  Serial.print(F("Humedad en %: "));
  Serial.println(h);                  // AGREGADO PARA EL DHT11

  
  //Verificar siempre que haya conexión al broker
  if (!client.connected()) {
    reconnect();  // En caso de que no haya conexión, ejecutar la función de reconexión, definida despues del void setup ()
  }// fin del if (!client.connected())
  client.loop(); // Esta función es muy importante, ejecuta de manera no bloqueante las funciones necesarias para la comunicación con el broker
  
  //****************************
  timeNow = millis(); // Control de tiempo para esperas no bloqueantes
  if (timeNow - timeLast > wait) { // Manda un mensaje por MQTT cada cinco segundos
    timeLast = timeNow; // Actualización de seguimiento de tiempo

    //Obtener las lecturas de temperatura y humedad del sensor
    float t = dht.readTemperature();
    float h = dht.readHumidity();

    //Comprobar que la conexion con el sensor sea exitosa
    if ( isnan(t) || isnan(h)) {
    Serial.println(F("No hay conexion"));
    return;
    }
    
    char dataString[8]; // Define una arreglo de caracteres para enviarlos por MQTT, especifica la longitud del mensaje en 8 caracteres
    dtostrf(t, 1, 2, dataString);  // Esta es una función nativa de leguaje AVR que convierte un arreglo de caracteres en una variable String
    Serial.print("Temperatura: "); // Se imprime en monitor solo para poder visualizar que el evento sucede
    Serial.println(dataString);
    client.publish("codigoIoT/SIC/G5/temp", dataString); // ESTO Esta es la función que envía los datos por MQTT, especifica el tema y el valor

    delay(100);
    dtostrf(h, 1, 2, dataString);  // Esta es una función nativa de leguaje AVR que convierte un arreglo de caracteres en una variable String
    Serial.print("Humedad: "); // Se imprime en monitor solo para poder visualizar que el evento sucede
    Serial.println(dataString);
    client.publish("codigoIoT/SIC/G5/hum", dataString); // ESTO Esta es la función que envía los datos por MQTT, especifica el tema y el valor


    
  }// fin del if (timeNow - timeLast > wait)
}// fin del void loop ()

// Funciones de usuario

// Esta función permite tomar acciones en caso de que se reciba un mensaje correspondiente a un tema al cual se hará una suscripción
void callback(char* topic, byte* message, unsigned int length) {

  // Indicar por serial que llegó un mensaje
  Serial.print("Llegó un mensaje en el tema: ");
  Serial.print(topic);

  // Concatenar los mensajes recibidos para conformarlos como una varialbe String
  String messageTemp; // Se declara la variable en la cual se generará el mensaje completo  
  for (int i = 0; i < length; i++) {  // Se imprime y concatena el mensaje
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }

  // Se comprueba que el mensaje se haya concatenado correctamente
  Serial.println();
  Serial.print ("Mensaje concatenado en una sola variable: ");
  Serial.println (messageTemp);

  // En esta parte puedes agregar las funciones que requieras para actuar segun lo necesites al recibir un mensaje MQTT

  // Ejemplo, en caso de recibir el mensaje true - false, se cambiará el estado del led soldado en la placa.
  // El ESP323CAM está suscrito al tema esp/output
  //if (String(topic) == "codigoIoT/SIC/G5/#")//NOTA:El # incluye todos los temas que se quieran suscribir {  // En caso de recibirse mensaje en el tema esp32/output
  if (String(topic) == "codigoIoT/SIC/G5/led") {  // En caso de recibirse mensaje en el tema esp32/output
    if(messageTemp == "ON"){
      Serial.println("Led encendido");
      digitalWrite(flashLedPin, HIGH);
//      delay(100);
//      digitalWrite(flashLedPin, LOW);
//      delay(100);
//      digitalWrite(flashLedPin, HIGH);
    }// fin del if (String(topic) == "esp32/output")
    else if(messageTemp == "OFF"){
      Serial.println("Led apagado");
      digitalWrite(flashLedPin, LOW);
//      delay(100);
//      digitalWrite(flashLedPin, HIGH);
//      delay(100);
//      digitalWrite(flashLedPin, LOW);
    }// fin del else if(messageTemp == "false")
  }// fin del if (String(topic) == "esp32/output")
}// fin del void callback

// Función para reconectarse
void reconnect() {
  // Bucle hasta lograr conexión
  while (!client.connected()) { // Pregunta si hay conexión
    Serial.print("Tratando de contectarse...");
    // Intentar reconexión
    if (client.connect("ESP32CAMClient")) { //Pregunta por el resultado del intento de conexión
      Serial.println("Conectado");
      client.subscribe("codigoIoT/SIC/G5/led"); // Esta función realiza la suscripción al tema
    }// fin del  if (client.connect("ESP32CAMClient"))
    else {  //en caso de que la conexión no se logre
      Serial.print("Conexion fallida, Error rc=");
      Serial.print(client.state()); // Muestra el codigo de error
      Serial.println(" Volviendo a intentar en 5 segundos");
      // Espera de 5 segundos bloqueante
      delay(5000);
      Serial.println (client.connected ()); // Muestra estatus de conexión
    }// fin del else
  }// fin del bucle while (!client.connected())
}// fin de void reconnect(
