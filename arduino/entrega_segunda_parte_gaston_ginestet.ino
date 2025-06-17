#ifdef ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif

#include <PubSubClient.h>

#define LED_PIN 2


// --- Datos WiFi ---
// const char* ssid = "Personal 289 2.4GHz"; //cargar con credenciales del wifi propio
// const char* password = "00439801553"; //cargar con credenciales del wifi propio

const char* ssid = "G-LINK"; //cargar con credenciales del wifi propio
const char* password = "a5b4c3d2e1"; //cargar con credenciales del wifi propio

// Datos del Broker 
const char* MQTT_BROKER_ADRESS = "broker.mqtt.cool";// Otra opcion "test.mosquitto.org";
const uint16_t MQTT_PORT = 1883;
const char* MQTT_CLIENT_NAME = "ESP32Client_TP2";
const char* MQTT_USER = "user";
const char* MQTT_PASS = "password";


// --- Estados del sistema ---
bool windowsOpen = false;
int temperature = 0;
int humidity = 0;

// Crear y configurar cliente MQTT
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

unsigned long lastTempRead = 0;
const unsigned long tempInterval = 5000; // por ejemplo, 5 segundos


void setup() {
  Serial.begin(115200);
  
  // Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  WiFi.setSleep(false);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());

  // Set pin modes
  pinMode(LED_PIN, OUTPUT);

  // MQTT
  client.setServer(MQTT_BROKER_ADRESS, MQTT_PORT);
  client.setCallback(callback);  // Para recibir mensajes
}

void loop() {
  unsigned long currentTime = millis();

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // lectura de temperatura y acción
  if (currentTime - lastTempRead > tempInterval) {
    lastTempRead = currentTime;
    readTemperatureAndHumidity();
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print(" MQTT connection...");
    
    if (client.connect(MQTT_CLIENT_NAME)) { //OJO credenciales MOSQUITTO MQTT_USER, MQTT_PASS
      Serial.println("connected");
      // Subscribe
      client.subscribe("esp32/ventana");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off". 
  // Changes the output state according to the message
  if (String(topic) == "esp32/ventana") {
    if (messageTemp == "abrir" && !windowsOpen) {
      openWindows();
    } else if (messageTemp == "cerrar" && windowsOpen) {
      closeWindows();
    }
  }
}

void readTemperatureAndHumidity() {
  temperature = random(15, 36);
  humidity = random(30, 90); // Humedad simulada (30% a 90%)

  Serial.print("Temperatura: ");
  Serial.println(temperature);
  Serial.print("Humedad: ");
  Serial.println(humidity);

  String payload = 
    "{\"temperatura\": " + String(temperature) +
    ", \"humedad\": " + String(humidity) +
    ", \"estado\": \"" + (windowsOpen ? "abrir" : "cerrar") +
    "\"}";

  client.publish("esp32/mediciones", payload.c_str());
  Serial.println("Publicando en esp32/mediciones: " + payload);
}


void openWindows() {
  Serial.println("Abriendo ventanas...");
  activateMotor();
  windowsOpen = true;
}

void closeWindows() {
  Serial.println("Cerrando ventanas...");
  activateMotor();
  windowsOpen = false;
}

void activateMotor() {
  Serial.println("Activando motor (LED encendido 5 segundos)...");
  digitalWrite(LED_PIN, HIGH);  // Encender LED
  delay(3000);
  digitalWrite(LED_PIN, LOW);   // Apagar LED
}





