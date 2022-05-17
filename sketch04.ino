/*
 * TODO
  Hi, akmal!

  Status lampu sekarang dalam keadaan: **Mati**[emoji]
  
  Silahkan gunakan bot ini dengan mengirimkan perintah berikut:
  
  /on - menyalakan lampu
  /off - mematikan lampu
  /status - cek keadaan lampu
  
  Atau silahkan menggunakan perintah berikut:
  
  /menu - membuat tampilan menu
  /help - bantuan bot
*/

#include <AsyncTelegram2.h>

#include <time.h>
#define MYTZ "WIB-7"

#include <ESP8266WiFi.h>
#define USE_CLIENTSSL false

WiFiClientSecure client;
Session   session;
X509List  certificate(telegram_cert);

AsyncTelegram2 bot(client);
const char* ssid  =  "NINGRUM77";
const char* pass  =  "090021452";
const char* token =  "5360848599:AAFJv21I9Glgl1qFYnDBAtn5wx-YI3-Cjyw";

InlineKeyboard inlineKbd;

#define LIGHT_ON_CB  "LightON"
#define LIGHT_OFF_CB "LightOFF"
#define STATUS_CB "Status"

const uint8_t LED = 2;

void setup() {
  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH);
  
  Serial.begin(115200);
  Serial.println("Memulai program bot...");
  
  WiFi.begin(ssid, pass);
  delay(500);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(250);
  }

  // Sync time with NTP, to check properly Telegram certificate
  configTime(MYTZ, "time.google.com", "time.windows.com", "pool.ntp.org");
  //Set certficate, session and some other base client properies
  client.setSession(&session);
  client.setTrustAnchors(&certificate);
  client.setBufferSizes(1024, 1024);

  bot.setUpdateTime(1000);
  bot.setTelegramToken(token);

  Serial.println("\nTest Telegram connection...");
  bot.begin() ? Serial.println("OK") : Serial.print("NOK");
  Serial.print("Bot name: @");
  Serial.print(bot.getBotName());

  inlineKbd.addButton("TURN ON", LIGHT_ON_CB, KeyboardButtonQuery);
  inlineKbd.addButton("TURN OFF", LIGHT_OFF_CB, KeyboardButtonQuery);
  inlineKbd.addRow();
  inlineKbd.addButton("Check Light Status", STATUS_CB, KeyboardButtonQuery);
}

void loop() {
  TBMessage msg;

  if (bot.getNewMessage(msg)) {
    MessageType msgType = msg.messageType;
    String msgText = msg.text;

    switch (msgType) {
      case MessageText:
        Serial.print("Message received: ");
        Serial.println(msgText);
        
        if (msgText.equalsIgnoreCase("/start")) {
          bot.sendMessage(msg, "Hi!\nGunakan /menu untuk menampilkan menu agar lampu dapat diatur.");
        } else if (msgText.equalsIgnoreCase("/menu")) {
          bot.sendMessage(msg, "💡Menu Lampu", inlineKbd);
        } else {
          bot.sendMessage(msg, "Gunakan menu yang ada!");
        }
        
        break;

      case MessageQuery:
        msgText = msg.callbackQueryData;
        
        Serial.print("\nCallback query message received: ");
        Serial.println(msg.callbackQueryData);
        
        if (msgText.equalsIgnoreCase(LIGHT_ON_CB)) {
          // pushed "LIGHT ON" button...
          Serial.println("\nSet light ON");
          digitalWrite(LED, LOW);
          // terminate the callback with an alert message
          bot.endQuery(msg, "Lampu Hidup", true);
        }
        else if (msgText.equalsIgnoreCase(LIGHT_OFF_CB)) {
          // pushed "LIGHT OFF" button...
          Serial.println("\nSet light OFF");
          digitalWrite(LED, HIGH);
          // terminate the callback with a popup message
          bot.endQuery(msg, "Lampu Mati", true);
        }
        else if (msgText.equalsIgnoreCase(STATUS_CB)) {
          Serial.println("\nCheck status");
          statusMessage(msg);
          bot.endQuery(msg, "Cek Status");
        }
        break;
        
      default:
        break;
    }
  }
}

void statusMessage(TBMessage msg) {
  String statusMsg;
  statusMsg = "Status Lampu: ";
  statusMsg += ((digitalRead(LED) == LOW) ? "Hidup ✅" : "Mati ⛔️");
  bot.sendMessage(msg, statusMsg);
}
