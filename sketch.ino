/*
 * TODO
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
InlineKeyboard lightOnKbd, lightOffKbd;

#define LIGHT_ON_NOW_CB "LightONNow"
#define LIGHT_ON_15_CB "LightON15"
#define LIGHT_ON_30_CB "LightON30"
#define LIGHT_ON_60_CB "LightON60"
#define LIGHT_ON_DURATION_CB "LightONDuration"
#define LIGHT_ON_SCHEDULE_CB "LightONSchedule"
#define LIGHT_OFF_NOW_CB "LightOFFNow"
#define LIGHT_OFF_15_CB "LightOFF15"
#define LIGHT_OFF_30_CB "LightOFF30"
#define LIGHT_OFF_60_CB "LightOFF60"
#define LIGHT_OFF_DURATION_CB "LightOFFDuratiOFF"
#define LIGHT_OFF_SCHEDULE_CB "LightOFFSchedule"

enum states {MAIN_STATE, SCHEDULE_STATE, DURATION_STATE} state;
enum type_sch {toON, toOFF} type_schedule;

const uint8_t LED = 2;
int32_t scheduleTime;
bool isKeyboardActive, isScheduled;
TBMessage scheduleMsg;

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

  lightOnKbd.addButton("NOW", LIGHT_ON_NOW_CB, KeyboardButtonQuery);
  lightOnKbd.addRow();
  lightOnKbd.addButton("15 Min", LIGHT_ON_15_CB, KeyboardButtonQuery);
  lightOnKbd.addButton("30 Min", LIGHT_ON_30_CB, KeyboardButtonQuery);
  lightOnKbd.addButton("1 Hour", LIGHT_ON_60_CB, KeyboardButtonQuery);
  lightOnKbd.addRow();
  lightOnKbd.addButton("Duration", LIGHT_ON_DURATION_CB, KeyboardButtonQuery);
  lightOnKbd.addButton("Schedule", LIGHT_ON_SCHEDULE_CB, KeyboardButtonQuery);

  lightOffKbd.addButton("NOW", LIGHT_OFF_NOW_CB, KeyboardButtonQuery);
  lightOffKbd.addRow();
  lightOffKbd.addButton("15 Min", LIGHT_OFF_15_CB, KeyboardButtonQuery);
  lightOffKbd.addButton("30 Min", LIGHT_OFF_30_CB, KeyboardButtonQuery);
  lightOffKbd.addButton("1 Hour", LIGHT_OFF_60_CB, KeyboardButtonQuery);
  lightOffKbd.addRow();
  lightOffKbd.addButton("Duration", LIGHT_OFF_DURATION_CB, KeyboardButtonQuery);
  lightOffKbd.addButton("Schedule", LIGHT_OFF_SCHEDULE_CB, KeyboardButtonQuery);

  isKeyboardActive = false;
  state = MAIN_STATE;
  type_schedule = toOFF;
  isScheduled = false;
}

void loop() {
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
      Serial.println("scheduleTime: " + (String)scheduleTime);
      Serial.println("now: " + (String)now);
      Serial.println("scheduleTime - now: " + (String) (scheduleTime - now));
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
                bot.sendMessage(msg, "Lampu sudah hidup.");
                return;
              } else {
                bot.sendMessage(msg, "Menu", lightOnKbd);
              }
            } else if (msgText.equalsIgnoreCase("/off") or msgText.equalsIgnoreCase("TURN OFF")) {
              if(!lightIsOn()) {
                bot.sendMessage(msg, "Lampu sudah mati.");
              } else {
                bot.sendMessage(msg, "Menu", lightOffKbd);
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
              bot.sendMessage(msg, "Perintah tidak tersedia!\n\nMasukkan kembali perintah atau buka bantuan /help");
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
            break;
            
        }
        break;
        
      case SCHEDULE_STATE:
        if (msgType == MessageText) {
          if (msgText.length() == 5 and msgText.indexOf(":") == 2) {
            setSchedule(msg);
          } else {
            bot.sendMessage(msg, "Format waktu salah. Silahkan masukkan ulang waktu dengan format <b>HH:MM</b>");
          }
        }
        break;
        
      case DURATION_STATE:
        if (msgType == MessageText) {
          int duration = msgText.toInt();
          if (duration > 0) {
            setDuration(msg);
          } else {
            bot.sendMessage(msg, "Tidak bisa, silahkan set durasi dengan benar!");
          }
        }
        break;
    }

    
  }
}

int32_t getToggleTime(String msg) {
  String s_hour = msg.substring(0,2);
  String s_min = msg.substring(3);
  
  int hour = s_hour.toInt();
  int min = s_min.toInt();

  time_t rawtime;
  struct tm *tm_raw;
  
  time(&rawtime);
  tm_raw = localtime(&rawtime);
  tm_raw->tm_hour = hour;
  tm_raw->tm_min = min;

  return mktime(tm_raw);
}

String getFormattedTime(time_t rawtime) {
  struct tm *tm_raw;
  tm_raw = localtime(&rawtime);
  return (String) tm_raw->tm_hour + ":" + (String) tm_raw->tm_min;
}

void setSchedule(TBMessage msg, int32_t toggleTime, type_sch type) {
//  int32_t toggleTime = getToggleTime(msg.text);
  int32_t nowTime = msg.date;

  Serial.println((String)toggleTime);
  Serial.println((String)nowTime);

  if (nowTime > toggleTime) {
    bot.sendMessage(msg, "Waktu sudah terlewat, silahkan set waktu dengan benar.");
  } else {
    bot.sendMessage(msg, "Lampu akan <b>" + (String)((lightIsOn()) ? "dimatikan" : "dihidupkan") + "</b> pada pukul " + getFormattedTime(toggleTime) +" WIB");
    
    type_schedule = type;
    isScheduled = true;
    scheduleMsg = msg;
    scheduleTime = toggleTime;
    state = MAIN_STATE;
  }

  Serial.println("\n\nDIJADWALIN BOS");
  Serial.println("type_schedule: " + (String)type_schedule);
  Serial.println("toggleTime: " + (String)toggleTime);
  Serial.println("scheduleTime: " + (String)scheduleTime);
  Serial.println("isScheduled: " + (String)isScheduled);
  Serial.println("state: " + (String)state);
}

void setSchedule(TBMessage msg) {
  setSchedule(msg, getToggleTime(msg.text), type_schedule);
}

void setDuration(TBMessage msg, int minute, type_sch type) {
  if (lightIsOn()) {
    turnOff(msg);
  } else {
    turnOn(msg);
  }
  
  int second = minute * 60;
  int32_t toggleTime = msg.date + second;
  
  setSchedule(msg, toggleTime, type);  
  
  Serial.println("second: " + (String)second);
  Serial.println("toggleTime: " + (String)toggleTime);
  Serial.println("msg.date: " + (String)msg.date);
}

void setDuration(TBMessage msg) {
  setDuration(msg, msg.text.toInt(), type_schedule);
}

void turnOn(TBMessage msg) {
  Serial.println("\nSet light ON");
  digitalWrite(LED, LOW);
  bot.sendMessage(msg, "✅ Lampu dihidupkan!");

  if (isScheduled and type_schedule == toON)
    isScheduled = false;
}

void turnOff(TBMessage msg) {  
  Serial.println("\nSet light OFF");
  digitalWrite(LED, HIGH);
  bot.sendMessage(msg, "⛔️ Lampu dimatikan!");

  if (isScheduled and type_schedule == toOFF)
    isScheduled = false;
}

String lightStatus() {
  String lightStatus;
  lightStatus = ((lightIsOn()) ? "Hidup ✅" : "Mati ⛔️");
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

bool lightIsOn() {
  return (digitalRead(LED) == LOW) ? true : false;
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

void scheduleMessage(TBMessage msg) {
  bot.sendMessage(msg, "Set waktu untuk penjadwalan: \n\n*dalam format <b>HH:MM</b>");
}

void durationMessage(TBMessage msg) {
  bot.sendMessage(msg, "Set durasi dalam <b>menit</b>: ");
}
