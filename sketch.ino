/*
 * TODO
 *  Option durasi lampu dinyalakan (lampu akan dimatikan setelah sekian menit)
 *  Option jadwal lampu dinyalakan/dimatikan
 *  Menghitung berapa lama lampu nyala/mati
*/

#include <AsyncTelegram2.h>

#include <time.h>
#define MYTZ "WIB-7"

#include <ESP8266WiFi.h>
#define USE_CLIENTSSL false

BearSSL::WiFiClientSecure client;
BearSSL::Session   session;
BearSSL::X509List  certificate(telegram_cert);

AsyncTelegram2 bot(client);
const char* ssid  =  "NINGRUM77";
const char* pass  =  "090021452";
const char* token =  "5360848599:AAHaYjtCDGX7UXiCFRxWevD5D0zrdox45G4";

ReplyKeyboard replyKbd;

#define LIGHT_ON_CB  "LightON"
#define LIGHT_OFF_CB "LightOFF"
#define STATUS_CB "Status"

const uint8_t LED = 2;
bool isKeyboardActive;
int state = 0;

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

  replyKbd.addButton("TURN ON");
  replyKbd.addButton("TURN OFF");
  replyKbd.addRow();
  replyKbd.addButton("STATUS");
  replyKbd.addRow();
  replyKbd.addButton("HELP");
  replyKbd.addButton("CLOSE");
  replyKbd.enableResize();

  isKeyboardActive = false;
}

void loop() {
  TBMessage msg;

  switch(state) {
    case 0:
      if (bot.getNewMessage(msg)) {
        MessageType msgType = msg.messageType;
        String msgText = msg.text;
    
        switch (msgType) {
          case MessageText:
            Serial.print("Message received: ");
            Serial.println(msgText);
            
            if (msgText.equalsIgnoreCase("/start")) {
              startMessage(msg);
            } else if (msgText.equalsIgnoreCase("/on") or msgText.equalsIgnoreCase("TURN ON")) {
              turnOn(msg);
            } else if (msgText.equalsIgnoreCase("/off") or msgText.equalsIgnoreCase("TURN OFF")) {
              turnOff(msg);
            } else if (msgText.equalsIgnoreCase("/status") or msgText.equalsIgnoreCase("STATUS")) {
              checkStatus(msg);
            } else if (msgText.equalsIgnoreCase("/menu") or msgText.equalsIgnoreCase("CLOSE")) {
              if (isKeyboardActive) {
                bot.removeReplyKeyboard(msg, "💡 Sembunyikan Menu");
                isKeyboardActive = false;
              } else {
                bot.sendMessage(msg, "💡 Tampilkan Menu", replyKbd);
                isKeyboardActive = true;
              }
            } else if (msgText.equalsIgnoreCase("/help") or msgText.equalsIgnoreCase("HELP")) {
              helpMessage(msg);
            } else if (msgText.equalsIgnoreCase("/state1")) {
              state = 1;
            } else {
            }
              bot.sendMessage(msg, "Perintah tidak tersedia!\n\nMasukkan kembali perintah atau buka bantuan /help");
            }
            
            break;
            
          default:
            break;
        }
      }
      break;
      
    case 1:
      bot.sendMessage(msg, "Ada di state 1");
      break;
  }
}

void turnOn(TBMessage msg) {
  if(!digitalRead(LED)) {
    bot.sendMessage(msg, "Lampu sudah hidup.");
    return;
  }
  
  Serial.println("\nSet light ON");
  digitalWrite(LED, LOW);
  bot.sendMessage(msg, "✅ Lampu dihidupkan!");
}

void turnOff(TBMessage msg) {
  if(digitalRead(LED)) {
    bot.sendMessage(msg, "Lampu sudah mati.");
    return;
  }
  
  Serial.println("\nSet light OFF");
  digitalWrite(LED, HIGH);
  bot.sendMessage(msg, "⛔️ Lampu dimatikan!");
}

String lightStatus() {
  String lightStatus;
  lightStatus = ((digitalRead(LED) == LOW) ? "Hidup ✅" : "Mati ⛔️");
  return lightStatus;
}

String helpLight() {
  String helpL;
  helpL = "/on - menghidupkan lampu\n"
          "/off - mematikan lampu\n"
          "/status - cek keadaan lampu\n\n";
  return helpL;
}

String helpBot() {
  String helpB;
  helpB = "/menu - tampilkan/sembunyikan menu\n"
          "/help - bantuan bot";
  return helpB;
}

void startMessage(TBMessage msg) {
  String startMsg;
  startMsg = "Hi. \n\n"
             "Status lampu sekarang dalam keadaan: " 
             + lightStatus() +
             "\n\nSilahkan gunakan bot ini dengan mengirimkan perintah berikut:\n\n"
             + helpLight() + 
             "Atau silahkan menggunakan perintah berikut:\n\n"
             + helpBot();
  bot.sendMessage(msg, startMsg);
}

void helpMessage(TBMessage msg) {
  String helpMsg;
  helpMsg = "📖 Bantuan\n\n" + helpLight() + helpBot();
  bot.sendMessage(msg, helpMsg);
}

void checkStatus(TBMessage msg) {
  String statusMsg;
  statusMsg = "Status Lampu: " + lightStatus();
  bot.sendMessage(msg, statusMsg);
}
