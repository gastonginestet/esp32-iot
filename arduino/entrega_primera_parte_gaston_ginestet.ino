#ifdef ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif

#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>   // https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
#include <ArduinoJson.h>

#define LED_PIN 2


// --- WiFi y Bot ---
const char* ssid = "*********"; //cargar con credenciales del wifi propio
const char* password = "*********"; //cargar con credenciales del wifi propio

// Claves Bot telegram
#define BOTtoken "*********"  // your Bot Token (Get from Botfather)
#define CHAT_ID "*********" //grupo (se toma de la URL), si fuera individuo se le pide al IDBot con /getid 


WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// --- Intervalos de tiempo ---
unsigned long lastNotificationTime = 0; // ultima notificacion
const unsigned long notificationInterval = 5000; // Tiempo de intervalo entre notificaciones

const unsigned long tempInterval = 5000;
const unsigned long statusInterval = 300000;  // Enviar estado cada 5 min
unsigned long lastTempRead;
// Checks for new messages every 1 second.
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;
unsigned long lastStatusMsg;

// --- Estados del sistema ---
bool windowsOpen = false;
bool autoMode = false;
int temperature = 0;


void setup() {
  Serial.begin(115200);
  
  // Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
  WiFi.setSleep(false);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());

  // Set pin modes
  pinMode(LED_PIN, OUTPUT);
  bot.sendMessage(CHAT_ID, "Bot iniciado correctamente", "");
}

void loop() {
  unsigned long currentTime = millis();

  // check de nuevos mensajes
  if (currentTime - lastTimeBotRan > botRequestDelay) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = currentTime;
  }

  // lectura de temperatura y acción
  if ((currentTime - lastTempRead > tempInterval) && autoMode) {
    lastTempRead = currentTime;
    readTemperatureAndAct();
  }

  // envio de status
  if (currentTime - lastStatusMsg > statusInterval) {
    lastStatusMsg = currentTime;
    prettyPrintStatus();
  }
}

void prettyPrintStatus() {
  String msg = "📊 Reporte periódico:\n";
  msg += "Temperatura: " + String(temperature) + "°C\n";
  msg += "Ventanas: " + String(windowsOpen ? "Abiertas" : "Cerradas") + "\n";
  msg += "Modo: " + String(autoMode ? "Automático" : "Manual");
  bot.sendMessage(CHAT_ID, msg, "");
}

void readTemperatureAndAct() {
  temperature = random(15, 36);
  Serial.print("Temperatura: ");
  Serial.println(temperature);

  if (!autoMode) return;

  if (temperature > 30 && !windowsOpen) {
    openWindows();
  } else if (temperature < 20 && windowsOpen) {
    closeWindows();
  } else {
    Serial.println("Estado sin cambios.");
  }
}


// Funcion para manejar los msg entrantes

void handleNewMessages(int numNewMessages) {

  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));


  for (int i = 0; i < numNewMessages; i++) {
    // Chat id of the requester
    String chat_id = String(bot.messages[i].chat_id);

    if (chat_id != CHAT_ID){
      bot.sendMessage(chat_id, "Usuario no autorizado", "");
      continue;
    }

    // Print the received message
    String text = bot.messages[i].text;
    Serial.println(text);
    String from_name = bot.messages[i].from_name;

    if (text == "/start") {
      autoMode = true;
      String welcome = "Hola, " + from_name + ".\n";
      welcome += "Usar los siguientes comandos para controlar.\n\n";
      welcome += "/abrir para desactivar modo automático y abrir ventanas \n";
      welcome += "/cerrar para desactivar modo automático y cerrar ventanas \n";
      welcome += "/auto activar modo automático \n";
      welcome += "/manual desactivar modo automático \n";
      welcome += "/estado para recibir el estado de la temperatura y el dispositivo. \n";
      bot.sendMessage(chat_id, welcome, "");
    }

    if (text == "/abrir") {
      autoMode = false;
      bot.sendMessage(CHAT_ID, "Ok, mandando a abrir las ventanas, desactivando modo automático..", "");
      openWindows();
    } else if (text == "/cerrar") {
      autoMode = false;
      bot.sendMessage(CHAT_ID, "Ok, mandando a cerrar las ventanas, desactivando modo automático..", "");
      closeWindows();
    } else if (text == "/auto") {
      autoMode = true;
      bot.sendMessage(CHAT_ID, "Modo automático ACTIVADO", "");
    } else if (text == "/manual") {
      autoMode = false;
      bot.sendMessage(CHAT_ID, "Modo automático DESACTIVADO", "");
    } else if (text == "/estado") {
      prettyPrintStatus();
    }
  }
}

void openWindows() {
  Serial.println("Abriendo ventanas...");
  activateMotor();
  windowsOpen = true;
  bot.sendMessage(CHAT_ID, "Ventanas ABIERTAS", "");
}

void closeWindows() {
  Serial.println("Cerrando ventanas...");
  activateMotor();
  windowsOpen = false;
  bot.sendMessage(CHAT_ID, "Ventanas CERRADAS", "");
}

void activateMotor() {
  Serial.println("Activando motor (LED encendido 5 segundos)...");
  digitalWrite(LED_PIN, HIGH);  // Encender LED
  delay(5000);           // Esperar 5 segundos
  digitalWrite(LED_PIN, LOW);   // Apagar LED
  Serial.println("Motor desactivado (LED apagado).");
}






