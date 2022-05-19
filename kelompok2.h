#include <AsyncTelegram2.h>

#define LIGHT_ON_NOW_CB "Hidupkan lampu sekarang"
#define LIGHT_ON_15_CB "Hidupkan lampu selama 15 menit"
#define LIGHT_ON_30_CB "Hidupkan lampu selama 30 menit"
#define LIGHT_ON_60_CB "Hidupkan lampu selama 1 jam"
#define LIGHT_ON_DURATION_CB "Hidupkan lampu dengan mengatur durasi"
#define LIGHT_ON_SCHEDULE_CB "Hidupkan lampu dengan mengatur jadwal"
#define LIGHT_OFF_NOW_CB "Matikan lampu sekarang"
#define LIGHT_OFF_15_CB "Matikan lampu selama 15 menit"
#define LIGHT_OFF_30_CB "Matikan lampu selama 30 menit"
#define LIGHT_OFF_60_CB "Matikan lampu selama 1 jam"
#define LIGHT_OFF_DURATION_CB "Matikan lampu dengan mengatur durasi"
#define LIGHT_OFF_SCHEDULE_CB "Matikan lampu dengan mengatur jadwal"

enum states {MAIN_STATE, SCHEDULE_STATE, DURATION_STATE} state;
enum type_sch {toON, toOFF} type_schedule;

const uint8_t LED = 4;
int32_t scheduleTime;
bool isKeyboardActive, isScheduled;
TBMessage scheduleMsg;

void setupReplyKeyboard(ReplyKeyboard &kbd) {
  kbd.addButton("TURN ON");
  kbd.addButton("TURN OFF");
  kbd.addRow();
  kbd.addButton("STATUS");
  kbd.addRow();
  kbd.addButton("HELP");
  kbd.addButton("CLOSE");
  kbd.enableResize();
}

void setupLightOnKeyboard(InlineKeyboard &kbd) {
  kbd.addButton("NOW", LIGHT_ON_NOW_CB, KeyboardButtonQuery);
  kbd.addRow();
  kbd.addButton("15 Min", LIGHT_ON_15_CB, KeyboardButtonQuery);
  kbd.addButton("30 Min", LIGHT_ON_30_CB, KeyboardButtonQuery);
  kbd.addButton("1 Hour", LIGHT_ON_60_CB, KeyboardButtonQuery);
  kbd.addRow();
  kbd.addButton("Duration", LIGHT_ON_DURATION_CB, KeyboardButtonQuery);
  kbd.addButton("Schedule", LIGHT_ON_SCHEDULE_CB, KeyboardButtonQuery);
}

void setupLightOffKeyboard(InlineKeyboard &kbd) {
  kbd.addButton("NOW", LIGHT_OFF_NOW_CB, KeyboardButtonQuery);
  kbd.addRow();
  kbd.addButton("15 Min", LIGHT_OFF_15_CB, KeyboardButtonQuery);
  kbd.addButton("30 Min", LIGHT_OFF_30_CB, KeyboardButtonQuery);
  kbd.addButton("1 Hour", LIGHT_OFF_60_CB, KeyboardButtonQuery);
  kbd.addRow();
  kbd.addButton("Duration", LIGHT_OFF_DURATION_CB, KeyboardButtonQuery);
  kbd.addButton("Schedule", LIGHT_OFF_SCHEDULE_CB, KeyboardButtonQuery);
}

bool lightIsOn() {
  return (digitalRead(LED) == LOW) ? true : false;
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

String gtf(int n) {
  String s = (String)n;
  if (n < 10) {
    return "0" + s;
  }
  return s;
}

String getFormattedTime(time_t rawtime) {
  struct tm *tm_raw;
  tm_raw = localtime(&rawtime);

  String hour;
  String min;
  
  
  return gtf(tm_raw->tm_hour) + ":" + gtf(tm_raw->tm_min);
}

void setSchedule(TBMessage msg, int32_t toggleTime, type_sch type) {
  int32_t nowTime = msg.date;

  Serial.println((String)toggleTime);
  Serial.println((String)nowTime);

  if (nowTime > toggleTime) {
    bot.sendMessage(msg, "⚠️Waktu sudah terlewat, silahkan set waktu dengan benar!");
  } else {
    bot.sendMessage(msg, "⏱ Lampu akan <b>" + (String)((lightIsOn()) ? "dimatikan" : "dihidupkan") + "</b> pada pukul " + getFormattedTime(toggleTime) +" WIB");
    
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
  startMsg = "Hi, " + msg.sender.firstName + "!\n\n"
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

void scheduleMessage(TBMessage msg) {
  bot.sendMessage(msg, "⚙ Set waktu untuk penjadwalan: \n\n*dalam format <b>HH:MM</b>");
}

void durationMessage(TBMessage msg) {
  bot.sendMessage(msg, "⚙ Set durasi dalam <b>menit</b>: ");
}
