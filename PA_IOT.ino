//Libraru
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <Servo.h>
#include <Ultrasonic.h> 

// Wifi
#define WIFI_SSID "P"
#define WIFI_PASSWORD "12345678"

//Token Telegram
#define BOT_TOKEN "7147328073:AAEhdHncjraAlcRjATS1OX4AQY6mR2hSPmc"
String chat_id = "1284485513";
const unsigned long BOT_MTBS = 1000;

X509List cert(TELEGRAM_CERTIFICATE_ROOT);
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);
unsigned long bot_lasttime; 

// Servo Pin
#define SERVO_PIN D4
Servo servo;

// Buzzer Pin
#define BUZZER_PIN D5

// Ultrasonic sensor pins
#define TRIGGER_PIN D8
#define ECHO_PIN D8
Ultrasonic ultrasonic(TRIGGER_PIN, ECHO_PIN); //buat inisialisasi

bool isBuzzerOn = false; // Status buzzer

void handleNewMessages(int numNewMessages) {
  Serial.print("Pesan Masuk Tele: ");
  Serial.println(numNewMessages);

  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = bot.messages[i].chat_id;
    String text = bot.messages[i].text;
    String from_name = bot.messages[i].from_name;
    if (from_name == "")
      from_name = "Guest";

    // kondisi if untuk membuka pakan 
    if (text.startsWith("/KasiMakan")) {
      int pos = text.substring(11).toInt(); 
      if (pos >= 0 && pos <= 600) {

        // Bunyikan buzzer 
        tone(BUZZER_PIN, 3000, 3000);
        servo.write(pos); 

        String response = "Pakan telah dibuka";
        bot.sendMessage(chat_id, response, "");
        

        tone(BUZZER_PIN, 2000, 100); 

        // kasi delay 0.1 detik
        delay(100);
        servo.write(90); // Putar servo ke 90 derajat biar nutup

        delay(50); // kasi dlay tutup servo
        bot.sendMessage(chat_id, "Pakan ditutup", "");

  
        tone(BUZZER_PIN, 2500, 100); 
      } else {
        bot.sendMessage(chat_id, "Error !!!", "");
      }
    }

    // Kondisi untuuk mengecek makanan
    if (text == "/CekIsipakan") {
      float distance_cm = ultrasonic.distanceRead(CM); // Membaca jarak dalam centimeter
      String response = "";
      if (distance_cm >= 12) {
        response = "Isi pakan: Habis";

        // Bunyikan buzzer jika pakan habis
        if (!isBuzzerOn) {
          tone(BUZZER_PIN, 4000);
          isBuzzerOn = true;
        }
      } else if (distance_cm < 12 && distance_cm >= 6) {
        response = "Isi pakan: Setengah";

        // Matikan buzzer jika pakan tidak habis
        if (isBuzzerOn) {
          noTone(BUZZER_PIN);
          isBuzzerOn = false;
        }
      } else {
        response = "Isi pakan: Banyak";
        // Matikan buzzer jika pakan tidak habis
        if (isBuzzerOn) {
          noTone(BUZZER_PIN);
          isBuzzerOn = false;
        }
      }
      bot.sendMessage(chat_id, response, "");
    }

    // Menampilkan pesan selamat datang ketfood dan daftar perintah di request telenya
    if (text == "/start") {
      String welcome = "Selamat Datang di Bot Smart Cat Feeder, " + from_name + ".\n";
      welcome += "List Perintah:\n\n";
      welcome += "/KasiMakan : Membuka tutup kontainer \n";
      welcome += "/CekIsipakan : Mengecek isi makanan yg tersedia di kontainer\n";
      bot.sendMessage(chat_id, welcome, "Markdown");
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println();

  // Servo Pin Mode
  servo.attach(SERVO_PIN, 0, 2000); 

  // Buzzer Pin Mode
  pinMode(BUZZER_PIN, OUTPUT);

  // Ultrasonic sensor pin mode
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  configTime(0, 0, "pool.ntp.org");    
  secured_client.setTrustAnchors(&cert); 
  Serial.print("Connecting to Wifi SSID ");
  Serial.print(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.print("\nWiFi connected. IP address: ");
  Serial.println(WiFi.localIP());

  Serial.print("Retrieving time: ");
  time_t now = time(nullptr);
  while (now < 24 * 3600)
  {
    Serial.print(".");
    delay(100);
    now = time(nullptr);
  }
  Serial.println(now);
}

void loop() {
  if (millis() - bot_lasttime > BOT_MTBS) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    
    while (numNewMessages) {
      Serial.println("Mendapatkan Respon");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    bot_lasttime = millis();
  }
}
