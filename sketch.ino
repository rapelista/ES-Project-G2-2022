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
const char* token =  "5360848599:AAHBP3d9pdkjx3jJCxb3k-JhzkRyt_01yDE";

#include "kelompok2.h"

ReplyKeyboard replyKbd;
InlineKeyboard lightOnKbd, lightOffKbd, dummy;

void setup() {
  pinMode(LED, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED, HIGH);
  digitalWrite(LED_BUILTIN, HIGH);
  
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

  setupReplyKeyboard(replyKbd);
  setupLightOnKeyboard(lightOnKbd);
  setupLightOffKeyboard(lightOffKbd);

  isKeyboardActive = false;
  state = MAIN_STATE;
  type_schedule = toOFF;
  isScheduled = false;

  lastTime = millis();
}

void loop() {
  if (millis() - lastTime >= 1000) {
    lastTime = millis();
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  }
  
  TBMessage msg;

  if (isScheduled) {
    time_t now;
    time(&now);

    if (scheduleTime - now <= 0) {
      Serial.print("GANTI BOS");
      switch (type_schedule) {
        case toON:
          turnOn(scheduleMsg);
          break;
        case toOFF:
          turnOff(scheduleMsg);
          break;
      }
      isScheduled = false;
    } else {  
      // nanti dihapus aja
      // Serial.println("scheduleTime: " + (String)scheduleTime);
      // Serial.println("now: " + (String)now);
      // Serial.println("scheduleTime - now: " + (String) (scheduleTime - now));
    }
  }

  if (bot.getNewMessage(msg)) {
    MessageType msgType = msg.messageType;
    String msgText = msg.text;
  
    switch (state) {
      case MAIN_STATE:
        switch (msgType) {
          case MessageText:
            Serial.print("Message received: ");
            Serial.println(msgText);
            
            if (msgText.equalsIgnoreCase("/start")) {
              startMessage(msg);
            } else if (msgText.equalsIgnoreCase("/on") or msgText.equalsIgnoreCase("TURN ON")) {
              if(lightIsOn()) {
                bot.sendMessage(msg, "⚠️Lampu sudah hidup");
                return;
              } else {
                bot.sendMessage(msg, "⚙️ Menu: Hidupkan Lampu", lightOnKbd);
              }
            } else if (msgText.equalsIgnoreCase("/off") or msgText.equalsIgnoreCase("TURN OFF")) {
              if(!lightIsOn()) {
                bot.sendMessage(msg, "⚠️Lampu sudah mati");
              } else {
                bot.sendMessage(msg, "⚙️ Menu: Matikan Lampu", lightOffKbd);
              }
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
            } else {
              bot.sendMessage(msg, "⚠️Perintah tidak tersedia!\n\nMasukkan kembali perintah atau buka bantuan /help");
            }
            
            break;
    
          case MessageQuery:
            msgText = msg.callbackQueryData;
            Serial.print("\nCallback query message received: ");
            Serial.println(msg.callbackQueryData);
    
            if (msgText.equalsIgnoreCase(LIGHT_ON_NOW_CB)) {
              turnOn(msg);
            } else if (msgText.equalsIgnoreCase(LIGHT_ON_SCHEDULE_CB)) {
              state = SCHEDULE_STATE;
              type_schedule = toON;
              scheduleMessage(msg);
            } else if (msgText.equalsIgnoreCase(LIGHT_ON_DURATION_CB)) {
              state = DURATION_STATE;
              type_schedule = toOFF;
              durationMessage(msg);
            } else if (msgText.equalsIgnoreCase(LIGHT_ON_15_CB  )) {
              setDuration(msg, 15, toOFF);
            } else if (msgText.equalsIgnoreCase(LIGHT_ON_30_CB  )) {
              setDuration(msg, 30, toOFF);
            } else if (msgText.equalsIgnoreCase(LIGHT_ON_60_CB  )) {
              setDuration(msg, 60, toOFF);
            } else if (msgText.equalsIgnoreCase(LIGHT_OFF_NOW_CB)) {
              turnOff(msg);
            } else if (msgText.equalsIgnoreCase(LIGHT_OFF_SCHEDULE_CB)) {
              state = SCHEDULE_STATE;
              type_schedule = toOFF;
              scheduleMessage(msg);
            } else if (msgText.equalsIgnoreCase(LIGHT_OFF_DURATION_CB)) {
              state = DURATION_STATE;
              type_schedule = toON;
              durationMessage(msg);
            } else if (msgText.equalsIgnoreCase(LIGHT_OFF_15_CB  )) {
              setDuration(msg, 15, toON);
            } else if (msgText.equalsIgnoreCase(LIGHT_OFF_30_CB  )) {
              setDuration(msg, 30, toON);
            } else if (msgText.equalsIgnoreCase(LIGHT_OFF_60_CB  )) {
              setDuration(msg, 60, toON);
            }
            
            bot.endQuery(msg, "Loading...");
            bot.editMessage(msg, "👉🏻 " + msgText, dummy);
            
            break;
            
        }
        break;
        
      case SCHEDULE_STATE:
        if (msgType == MessageText) {
          if (msgText.length() == 5 and msgText.indexOf(":") == 2) {
            setSchedule(msg);
          } else if (msgText.equalsIgnoreCase("/cancel")) {
            state = MAIN_STATE;
            bot.sendMessage(msg, "👌🏼 Perintah dibatalkan");
          } else {
            bot.sendMessage(msg, "⚠️Format waktu salah. Silahkan masukkan ulang waktu dengan format <b>HH:MM</b>");
          }
        }
        break;
        
      case DURATION_STATE:
        if (msgType == MessageText) {
          int duration = msgText.toInt();
          if (duration > 0) {
            setDuration(msg);
          } else if (msgText.equalsIgnoreCase("/cancel")) {
            state = MAIN_STATE;
            bot.sendMessage(msg, "👌🏼 Perintah dibatalkan");
          } else {
            bot.sendMessage(msg, "⚠️Tidak bisa, silahkan set durasi dengan benar!");
          }
        }
        break;
        
    }
  }
}
