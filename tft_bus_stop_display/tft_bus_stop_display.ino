#include <Arduino.h>
#include <TFT_eSPI.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include <vector>
#include <algorithm>
#include <Freenove_IR_Lib_for_ESP32.h>

// pins
const uint16_t IR_PIN     = 13;
const uint8_t  BUZZER_PIN = 12;
const uint8_t  BUTTON_PIN = 5;

// ir remote codes
const uint32_t IR_DOWN = 0xFF9867;
const uint32_t IR_UP   = 0xFF02FD;
const uint32_t IR_OK   = 0xFFA857;

// wifi
const char* ssid     = "ssid";
const char* password = "password";

// api
const char* apiKey   = "api-key";
const char* baseUrl  = "https://api.um.warszawa.pl/api/action";

// api res ids
const char* RES_LINES     = "88cd555f-6f31-43ca-9de4-66c479ad5942";
const char* RES_TIMETABLE = "e923fa0e-d96c-43f9-ae6e-60518c9f3238";

// tft screen
const int SCREEN_W        = 240;
const int SCREEN_H        = 320;
const int MAX_BUSES       = 4;
const int MENU_ITEMS_PAGE = 5;
const int MAX_STOPS       = 25;

// colors
#define RGB565(r,g,b) ((uint16_t)(((r&0xF8)<<8)|((g&0xFC)<<3)|(b>>3)))

const uint16_t COL_BG_A     = RGB565(0x18,0x18,0x18);
const uint16_t COL_BG_B     = RGB565(0x26,0x26,0x2E);
const uint16_t COL_SEP      = RGB565(0x3A,0x3A,0x3A);
const uint16_t COL_DIR_A    = RGB565(0xAA,0xAA,0xAA);
const uint16_t COL_DIR_B    = RGB565(0xBB,0xBB,0xCC);
const uint16_t COL_SELECTED = RGB565(0x00,0x55,0xAA);
const uint16_t COL_HEADER   = TFT_RED;
const uint16_t COL_MENU_BG  = RGB565(0x10,0x10,0x10);
const uint16_t COL_MUTED_IC = RGB565(0xC0,0x50,0x00);

struct StopEntry { String id, nr, name; };

struct BusDeparture {
  String line;
  String direction;
  String scheduledTime;
  int    secondsUntil;
  bool   valid;
  bool   mutedForToday;
};

struct TimeNow { int hour, minute, second; bool valid; };

struct AlarmOption {
  String scheduledTime;
  String direction;
  String label;
};

// alarmMode == -1  - off 
// alarmMode ==  0  - every depart
// alarmMode >   0  - target time
int    alarmMode       = -1;
String alarmTargetTime = "";
bool   globalMute      = false;

enum AppState { STATE_MENU_STOP, STATE_MENU_LINE, STATE_MENU_ALARM, STATE_DEPARTURES };

TFT_eSPI tft = TFT_eSPI();
Freenove_ESP32_IR_Recv ir(IR_PIN);

AppState appState = STATE_MENU_STOP;

std::vector<StopEntry>    allStops;
std::vector<String>       stopLines;
std::vector<BusDeparture> currentDepartures;
std::vector<AlarmOption>  alarmOptions;

String selectedStopId   = "";
String selectedStopNr   = "";
String selectedStopName = "";
String selectedLine     = "";

int menuCursor = 0;
int menuScroll = 0;

unsigned long lastClockRefresh = 0;
const unsigned long CLOCK_INTERVAL = 10000;
int lastMinute = -1;

unsigned long lastBuzzAt = 0;
int           buzzPhase  = 0;
const int     ALARM_THRESHOLD_SEC = 10 * 60;

unsigned long btnPressedAt = 0;
bool          btnHeld      = false;
const unsigned long HOLD_MS = 800;

// list of stops
struct BuiltInStop { const char* id; const char* nr; const char* name; };
const BuiltInStop builtInStops[] PROGMEM = {
  {"4222","02","Rybnicka"},
  {"4213","01","Wa\u0142owicka"},
  {"4212","01","Parowcowa"},
  {"4228","02","Wojciechowskiego"},
  {"4225","01","Sympatyczna"},
  {"4049","03","\u0141opusza\u0144ska"},
  {"4048","04","Popularna"},
  {"4046","01","\u015amig\u0142owca"},
  {"4052","06","Pl. Starynkiewicza"},
  {"4052","02","Pl. Starynkiewicza"},
  {"4109","02","Siemie\u0144skiego"},
  {"4112","02","Mo\u0142dawska"},
  {"4117","01","Rokosowska"},
  {"4119","07","Szcz\u0119\u015bliwice"},
  {"3096","02","Wr\u00f3bla"},
  {"3098","02","Nowoursynowska"},
  {"3106","01","Madali\u0144skiego-Szpital"},
  {"3113","03","Smoluchowskiego"},
  {"3115","02","Telewizja Polska"},
  {"3208","02","P\u0142askowickiej"},
  {"3222","01","Rozjazd Oborski"},
  {"3227","02","Markowskiego"},
  {"3230","01","Metro Rac\u0142awicka"},
  {"3238","01","Wo\u0142oska-Szpital"},
  {"3241","01","Konstruktorska"},
};

// utf-8 to ascii conv
String utf8ToAscii(const String& utf8) {
  String result; result.reserve(utf8.length());
  const uint8_t* s = (const uint8_t*)utf8.c_str();
  size_t len = utf8.length();
  for (size_t i = 0; i < len;) {
    uint8_t b = s[i];
    if ((b&0xE0)==0xC0 && i+1<len) {
      uint16_t cp=((b&0x1F)<<6)|(s[i+1]&0x3F); i+=2;
      switch(cp){
        case 0x0105:result+='a';break; case 0x0107:result+='c';break;
        case 0x0119:result+='e';break; case 0x0142:result+='l';break;
        case 0x0144:result+='n';break; case 0x00F3:result+='o';break;
        case 0x015B:result+='s';break; case 0x017A:result+='z';break;
        case 0x017C:result+='z';break; case 0x0104:result+='A';break;
        case 0x0106:result+='C';break; case 0x0118:result+='E';break;
        case 0x0141:result+='L';break; case 0x0143:result+='N';break;
        case 0x00D3:result+='O';break; case 0x015A:result+='S';break;
        case 0x0179:result+='Z';break; case 0x017B:result+='Z';break;
        default:result+='?';break;
      }
    } else if((b&0xF0)==0xE0&&i+2<len){i+=3;result+='?';}
      else if((b&0xF8)==0xF0&&i+3<len){i+=4;result+='?';}
      else{result+=(char)b;i++;}
  }
  return result;
}

// standardization of nighttime hours
bool normalizeNightBusTime(String& t) {
  if (t.length()<8) return false;
  int h=t.substring(0,2).toInt();
  if (h<24) return false;
  char buf[9]; snprintf(buf,sizeof(buf),"%02d%s",h-24,t.substring(2).c_str());
  t=String(buf); return true;
}

// time
void setupTime() {
  configTime(3600,3600,"pool.ntp.org","time.nist.gov");
  struct tm ti; int att=0;
  while (!getLocalTime(&ti)){delay(1000);if(++att>20)return;}
}

TimeNow getTimeNow() {
  struct tm ti;
  if (!getLocalTime(&ti)) return {0,0,0,false};
  return {ti.tm_hour,ti.tm_min,ti.tm_sec,true};
}

int totalSeconds(const TimeNow& t){ return t.hour*3600+t.minute*60+t.second; }

int calcSecondsUntil(const String& sched, int nowSec) {
  if (sched.length()<8) return 9999;
  int s=sched.substring(0,2).toInt()*3600+
        sched.substring(3,5).toInt()*60+
        sched.substring(6,8).toInt()-nowSec;
  if (s<-3600) s+=86400;
  return s;
}

String fetchAPI(const char* resId, const String& params) {
  HTTPClient http;
  String url=String(baseUrl)+"/dbtimetable_get/?id="+resId+"&apikey="+apiKey+params;
  http.begin(url); http.setTimeout(10000);
  http.addHeader("Accept-Charset","utf-8");
  int code=http.GET();
  String body=(code==HTTP_CODE_OK)?http.getString():"";
  http.end(); return body;
}

void loadBuiltInStops() {
  allStops.clear();
  const int n=sizeof(builtInStops)/sizeof(BuiltInStop);
  for (int i=0;i<n&&i<MAX_STOPS;i++){
    StopEntry e; e.id=builtInStops[i].id; e.nr=builtInStops[i].nr; e.name=builtInStops[i].name;
    allStops.push_back(e);
  }
}

void fetchLines() {
  stopLines.clear();
  String r=fetchAPI(RES_LINES,"&busstopId="+selectedStopId+"&busstopNr="+selectedStopNr);
  if (!r.length()) return;
  DynamicJsonDocument doc(4096);
  if (deserializeJson(doc,r)) return;
  for (JsonObject e:doc["result"].as<JsonArray>())
    for (JsonObject v:e["values"].as<JsonArray>())
      if (String(v["key"].as<const char*>())=="linia")
        stopLines.push_back(v["value"].as<const char*>());
}

void fetchAlarmOptions() {
  alarmOptions.clear();
  AlarmOption every; every.scheduledTime=""; every.label="Kazdy odjazd";
  alarmOptions.push_back(every);

  String r=fetchAPI(RES_TIMETABLE,
    "&busstopId="+selectedStopId+"&busstopNr="+selectedStopNr+"&line="+selectedLine);
  if (!r.length()) return;

  DynamicJsonDocument doc(32768);
  if (deserializeJson(doc,r)) return;

  std::vector<AlarmOption> raw;
  for (JsonArray dep:doc["result"].as<JsonArray>()) {
    AlarmOption o;
    for (JsonObject f:dep) {
      String k=f["key"].as<const char*>();
      if      (k=="czas")     o.scheduledTime=f["value"].as<const char*>();
      else if (k=="kierunek") o.direction    =f["value"].as<const char*>();
    }
    normalizeNightBusTime(o.scheduledTime);
    String d=utf8ToAscii(o.direction);
    if (d.length()>11) d=d.substring(0,28);
    o.label=o.scheduledTime.substring(0,5)+" "+d;
    raw.push_back(o);
  }
  std::sort(raw.begin(),raw.end(),[](const AlarmOption&a,const AlarmOption&b){
    return a.scheduledTime<b.scheduledTime;
  });
  for (auto& o:raw) alarmOptions.push_back(o);
}

// departs for a specific line
void updateDepartures() {
  std::vector<BusDeparture> all;

  String r=fetchAPI(RES_TIMETABLE,
    "&busstopId="+selectedStopId+"&busstopNr="+selectedStopNr+"&line="+selectedLine);
  if (!r.length()) return;

  DynamicJsonDocument doc(32768);
  if (deserializeJson(doc,r)) return;

  for (JsonArray dep:doc["result"].as<JsonArray>()) {
    BusDeparture d; d.valid=true; d.line=selectedLine; d.mutedForToday=false;
    for (JsonObject f:dep) {
      String k=f["key"].as<const char*>();
      if      (k=="kierunek") d.direction    =f["value"].as<const char*>();
      else if (k=="czas")     d.scheduledTime=f["value"].as<const char*>();
    }
    normalizeNightBusTime(d.scheduledTime);
    all.push_back(d);
  }

  TimeNow t=getTimeNow(); if (!t.valid) return;
  int nowSec=totalSeconds(t);
  for (auto& d:all){d.secondsUntil=calcSecondsUntil(d.scheduledTime,nowSec);if(d.secondsUntil<=0)d.valid=false;}
  std::sort(all.begin(),all.end(),[](const BusDeparture&a,const BusDeparture&b){
    if(!a.valid)return false;if(!b.valid)return true;return a.secondsUntil<b.secondsUntil;});

  std::vector<BusDeparture> prev=currentDepartures;
  currentDepartures.clear();
  for (const auto& d:all){
    if (!d.valid||(int)currentDepartures.size()>=MAX_BUSES) continue;
    BusDeparture nd=d;
    for (const auto& p:prev)
      if (p.scheduledTime==nd.scheduledTime){nd.mutedForToday=p.mutedForToday;break;}
    currentDepartures.push_back(nd);
  }
}

void updateDeparturesAllLines() {
  currentDepartures.clear();
  std::vector<BusDeparture> all;
  for (const String& ln:stopLines){
    String r=fetchAPI(RES_TIMETABLE,
      "&busstopId="+selectedStopId+"&busstopNr="+selectedStopNr+"&line="+ln);
    if (!r.length()) continue;
    DynamicJsonDocument doc(16384);
    if (deserializeJson(doc,r)) continue;
    for (JsonArray dep:doc["result"].as<JsonArray>()){
      BusDeparture d; d.valid=true; d.line=ln; d.mutedForToday=false;
      for (JsonObject f:dep){
        String k=f["key"].as<const char*>();
        if      (k=="kierunek") d.direction    =f["value"].as<const char*>();
        else if (k=="czas")     d.scheduledTime=f["value"].as<const char*>();
      }
      normalizeNightBusTime(d.scheduledTime);
      all.push_back(d);
    }
  }
  TimeNow t=getTimeNow(); if (!t.valid) return;
  int nowSec=totalSeconds(t);
  for (auto& d:all){d.secondsUntil=calcSecondsUntil(d.scheduledTime,nowSec);if(d.secondsUntil<=0)d.valid=false;}
  std::sort(all.begin(),all.end(),[](const BusDeparture&a,const BusDeparture&b){
    if(!a.valid)return false;if(!b.valid)return true;return a.secondsUntil<b.secondsUntil;});
  for (const auto& d:all) if(d.valid&&(int)currentDepartures.size()<MAX_BUSES) currentDepartures.push_back(d);
}

void refreshCountdowns() {
  TimeNow t=getTimeNow(); if (!t.valid) return;
  int nowSec=totalSeconds(t);
  for (auto& d:currentDepartures){d.secondsUntil=calcSecondsUntil(d.scheduledTime,nowSec);if(d.secondsUntil<=0)d.valid=false;}
  currentDepartures.erase(
    std::remove_if(currentDepartures.begin(),currentDepartures.end(),[](const BusDeparture&d){return !d.valid;}),
    currentDepartures.end());
}

// current depart (alarm)
int alarmingIdx() {
  if (alarmMode<0 || globalMute) return -1;
  for (int i=0;i<(int)currentDepartures.size();i++){
    const BusDeparture& d=currentDepartures[i];
    if (!d.valid||d.mutedForToday) continue;
    if (d.secondsUntil<=0||d.secondsUntil>ALARM_THRESHOLD_SEC) continue;
    if (alarmMode==0) return i;
    if (d.scheduledTime==alarmTargetTime) return i;
  }
  return -1;
}

// buzzer
void updateBuzzer() {
  if (alarmingIdx()<0){digitalWrite(BUZZER_PIN,LOW);return;}
  unsigned long now=millis();
  if (buzzPhase==0){
    if (now-lastBuzzAt>=1500){buzzPhase=1;lastBuzzAt=now;}
    digitalWrite(BUZZER_PIN,LOW);
  } else {
    unsigned long inS=now-lastBuzzAt;
    int bi=inS/200,hb=inS%200;
    if (bi>=6){buzzPhase=0;lastBuzzAt=now;digitalWrite(BUZZER_PIN,LOW);}
    else       {digitalWrite(BUZZER_PIN,(bi%2==0&&hb<150)?HIGH:LOW);}
  }
}

// display
const int DEP_START_Y  = 44;
const int DEP_ROW_H    = 64;
const int DEP_TILE_H   = 63;
const int DEP_CIRCLE_X = 210;
const int DEP_CIRCLE_R = 18;
const int MUTE_IC_X    = DEP_CIRCLE_X - DEP_CIRCLE_R - 10;
const int MUTE_IC_R    = 8;

void drawStatusBar() {
  TimeNow t=getTimeNow();
  tft.fillRect(0,302,SCREEN_W,18,TFT_BLACK);
  if (t.valid){
    char ts[10]; sprintf(ts,"%02d:%02d:%02d",t.hour,t.minute,t.second);
    tft.setTextColor(TFT_CYAN,TFT_BLACK);
    tft.drawString(ts,4,303,2);
  }
  if (alarmMode>=0){
    if (globalMute){
      tft.setTextColor(RGB565(0x60,0x60,0x60),TFT_BLACK);
      tft.drawString("ALARM OFF",140,303,2);
    } else {
      tft.setTextColor(TFT_YELLOW,TFT_BLACK);
      tft.drawString("ALARM ON ",140,303,2);
    }
  }
}

void drawMuteIcon(int rowIdx, bool muted) {
  int y  = DEP_START_Y + rowIdx * DEP_ROW_H;
  int cx = MUTE_IC_X;
  int cy = y + 14;

  uint16_t bg=(rowIdx%2==0)?COL_BG_A:COL_BG_B;
  if (!muted){
    tft.fillCircle(cx,cy,MUTE_IC_R,bg);
    return;
  }
  tft.fillCircle(cx,cy,MUTE_IC_R,COL_MUTED_IC);
  tft.setTextColor(TFT_BLACK,COL_MUTED_IC);
  tft.drawCentreString("X",cx+1,cy-3,1);
}

void drawDepartureHeader(){
  tft.fillRect(0,0,SCREEN_W,DEP_START_Y,COL_HEADER);
  tft.setTextColor(TFT_WHITE,COL_HEADER);
  tft.drawCentreString(utf8ToAscii(selectedStopName),SCREEN_W/2,4,2);
  String sub="Przystanek "+selectedStopNr;
  if (selectedLine.length()) sub+="  Linia "+selectedLine;
  tft.drawCentreString(sub,SCREEN_W/2,26,2);
}

void drawDepartureRow(int i){
  int y   = DEP_START_Y+i*DEP_ROW_H;
  int ccy = y+DEP_ROW_H/2;
  uint16_t bg =(i%2==0)?COL_BG_A:COL_BG_B;
  uint16_t dir=(i%2==0)?COL_DIR_A:COL_DIR_B;

  tft.fillRect(0,y,SCREEN_W,DEP_TILE_H,bg);
  tft.drawFastHLine(0,y+DEP_TILE_H,SCREEN_W,COL_SEP);

  if (i>=(int)currentDepartures.size()) return;
  BusDeparture& dep=currentDepartures[i];
  if (dep.secondsUntil<=0) return;

  tft.fillRoundRect(4,y+10,36,28,4,TFT_ORANGE);
  tft.setTextColor(TFT_BLACK,TFT_ORANGE);
  tft.drawCentreString(dep.line,22,y+15,2);

  tft.setTextColor(TFT_WHITE,bg);
  tft.drawString(dep.scheduledTime.substring(0,5),46,y+6,4);

  String d=utf8ToAscii(dep.direction);
  if (d.length()>15) d=d.substring(0,15);
  tft.setTextColor(dir,bg);
  tft.drawString(d,46,y+38,2);

  int    mins  =dep.secondsUntil/60;
  bool   subMin=(dep.secondsUntil<=60);
  String lbl   =subMin?"<1'":(String(mins)+"'");
  uint16_t col =(subMin||mins<=2)?TFT_RED:TFT_GREEN;
  tft.fillCircle(DEP_CIRCLE_X,ccy,DEP_CIRCLE_R,col);
  tft.setTextColor(TFT_BLACK,col);
  tft.drawCentreString(lbl,DEP_CIRCLE_X,ccy-7,2);

  drawMuteIcon(i,dep.mutedForToday);
}

void displayDepartures(){
  TimeNow t=getTimeNow();
  if (t.valid){
    int nowSec=totalSeconds(t);
    for (auto& d:currentDepartures)
      d.secondsUntil=calcSecondsUntil(d.scheduledTime,nowSec);
  }
  for (int i=0;i<MAX_BUSES;i++) drawDepartureRow(i);
  drawStatusBar();
}

void handleButton(){
  static bool lastState=HIGH;
  bool cur=(bool)digitalRead(BUTTON_PIN);

  if (lastState==HIGH && cur==LOW){
    btnPressedAt=millis();
    btnHeld=false;
  }

  if (cur==LOW && !btnHeld && (millis()-btnPressedAt)>=HOLD_MS){
    btnHeld=true;
    globalMute=!globalMute;
    buzzPhase=0; digitalWrite(BUZZER_PIN,LOW);
    drawStatusBar();
  }

  if (lastState==LOW && cur==HIGH){
    unsigned long held=millis()-btnPressedAt;
    if (!btnHeld && held<HOLD_MS){
      int idx=alarmingIdx();
      if (idx>=0){
        currentDepartures[idx].mutedForToday=true;
        buzzPhase=0; digitalWrite(BUZZER_PIN,LOW);
        drawDepartureRow(idx);
        drawStatusBar();
      }
    }
    btnHeld=false;
  }

  lastState=cur;
}

// menu
const int MENU_HEADER_H = 40;
const int MENU_ITEM_H   = 51;
const int MENU_HINT_Y   = 296;

void drawMenuHeader(const String& title, const String& sub=""){
  tft.fillRect(0,0,SCREEN_W,MENU_HEADER_H,COL_HEADER);
  tft.setTextColor(TFT_WHITE,COL_HEADER);
  tft.drawCentreString(title,SCREEN_W/2,6,2);
  if (sub.length()) tft.drawCentreString(sub,SCREEN_W/2,24,2);
}

void drawMenuHint(){
  tft.fillRect(0,MENU_HINT_Y,SCREEN_W,SCREEN_H-MENU_HINT_Y,RGB565(0x1A,0x1A,0x1A));
  tft.setTextColor(RGB565(0x88,0x88,0x88),RGB565(0x1A,0x1A,0x1A));
  tft.drawCentreString("GORA:+  DOL:-  OK:>",SCREEN_W/2,MENU_HINT_Y+6,1);
}

// menu items
void drawMenuItem(int idx, int row, const String& label, bool selected) {
  int y = MENU_HEADER_H + row * MENU_ITEM_H;
  uint16_t bg  = selected ? COL_SELECTED : ((row%2==0) ? COL_BG_A : COL_BG_B);
  uint16_t dim = selected ? RGB565(0xCC,0xCC,0xFF) : RGB565(0xCC,0xCC,0xCC);

  tft.fillRect(0, y, SCREEN_W, MENU_ITEM_H-1, bg);
  tft.drawFastHLine(0, y+MENU_ITEM_H-1, SCREEN_W, COL_SEP);

  tft.setTextColor(RGB565(0x80,0x80,0x80), bg);
  tft.drawString(String(idx+1)+".", 6, y+8, 2);

  if (label.length() > 32) {
    int cut = 31;
    while (cut > 8 && label[cut] != ' ') cut--;

    if (label[cut] == ' ') {
      tft.setTextColor(TFT_WHITE, bg);
      tft.drawString(label.substring(0, cut), 28, y+6, 2);
      String rest = label.substring(cut+1);
      if (rest.length() > 32) rest = rest.substring(0, 32);
      tft.setTextColor(dim, bg);
      tft.drawString(rest, 28, y+26, 2);
    } else {
      tft.setTextColor(TFT_WHITE, bg);
      tft.drawString(label.substring(0, 32), 28, y+16, 2);
    }
  } else {
    tft.setTextColor(TFT_WHITE, bg);
    tft.drawString(label, 28, y+16, 2);
  }
}

// menu labels
String stopLabel(int i) {
  return utf8ToAscii(allStops[i].name) + " " + allStops[i].nr;
}

String lineLabel(int i) { return (i==0) ? "Wszystkie linie" : ("Linia "+stopLines[i-1]); }
String alarmLabel(int i) { return alarmOptions[i].label; }

void drawStopMenu(){
  tft.fillScreen(TFT_BLACK);
  drawMenuHeader("Wybierz przystanek","("+String(allStops.size())+" dostepnych)");
  for (int row=0; row<MENU_ITEMS_PAGE; row++){
    int idx = menuScroll+row;
    if (idx>=(int)allStops.size()){
      tft.fillRect(0, MENU_HEADER_H+row*MENU_ITEM_H, SCREEN_W, MENU_ITEM_H-1, COL_MENU_BG);
      tft.drawFastHLine(0, MENU_HEADER_H+row*MENU_ITEM_H+MENU_ITEM_H-1, SCREEN_W, COL_SEP);
      continue;
    }
    drawMenuItem(idx, row, stopLabel(idx), idx==menuCursor);
  }
  drawMenuHint();
}

void drawLineMenu(){
  tft.fillScreen(TFT_BLACK);
  drawMenuHeader("Wybierz linie", utf8ToAscii(selectedStopName)+" "+selectedStopNr);
  int total = (int)stopLines.size()+1;
  for (int row=0; row<MENU_ITEMS_PAGE; row++){
    int idx = menuScroll+row;
    if (idx>=total){
      tft.fillRect(0, MENU_HEADER_H+row*MENU_ITEM_H, SCREEN_W, MENU_ITEM_H-1, COL_MENU_BG);
      tft.drawFastHLine(0, MENU_HEADER_H+row*MENU_ITEM_H+MENU_ITEM_H-1, SCREEN_W, COL_SEP);
      continue;
    }
    drawMenuItem(idx, row, lineLabel(idx), idx==menuCursor);
  }
  drawMenuHint();
}

void drawAlarmMenu(){
  tft.fillScreen(TFT_BLACK);
  drawMenuHeader("Alarm - kiedy?","Linia "+selectedLine+" "+utf8ToAscii(selectedStopName));
  int total = (int)alarmOptions.size();
  for (int row=0; row<MENU_ITEMS_PAGE; row++){
    int idx = menuScroll+row;
    if (idx>=total){
      tft.fillRect(0, MENU_HEADER_H+row*MENU_ITEM_H, SCREEN_W, MENU_ITEM_H-1, COL_MENU_BG);
      tft.drawFastHLine(0, MENU_HEADER_H+row*MENU_ITEM_H+MENU_ITEM_H-1, SCREEN_W, COL_SEP);
      continue;
    }
    drawMenuItem(idx, row, alarmLabel(idx), idx==menuCursor);
  }
  drawMenuHint();
}

void refreshMenuRow(int idx, const String& label, bool selected) {
  int row = idx-menuScroll;
  if (row<0 || row>=MENU_ITEMS_PAGE) return;
  drawMenuItem(idx, row, label, selected);
}

void menuNavigate(int delta){
  int old = menuCursor;
  int total = 0;
  switch(appState){
    case STATE_MENU_STOP:  total=(int)allStops.size();      break;
    case STATE_MENU_LINE:  total=(int)stopLines.size()+1;   break;
    case STATE_MENU_ALARM: total=(int)alarmOptions.size();  break;
    default: return;
  }
  menuCursor = constrain(menuCursor+delta, 0, total-1);

  bool redraw = false;
  if (menuCursor >= menuScroll+MENU_ITEMS_PAGE){ menuScroll=menuCursor-MENU_ITEMS_PAGE+1; redraw=true; }
  if (menuCursor <  menuScroll){ menuScroll=menuCursor; redraw=true; }

  if (redraw){
    if      (appState==STATE_MENU_STOP)  drawStopMenu();
    else if (appState==STATE_MENU_LINE)  drawLineMenu();
    else if (appState==STATE_MENU_ALARM) drawAlarmMenu();
    return;
  }

  if (old != menuCursor){
    if (appState==STATE_MENU_STOP){
      refreshMenuRow(old, stopLabel(old), false);
      refreshMenuRow(menuCursor, stopLabel(menuCursor), true);
    } else if (appState==STATE_MENU_LINE){
      refreshMenuRow(old, lineLabel(old), false);
      refreshMenuRow(menuCursor, lineLabel(menuCursor), true);
    } else if (appState==STATE_MENU_ALARM){
      refreshMenuRow(old, alarmLabel(old), false);
      refreshMenuRow(menuCursor, alarmLabel(menuCursor), true);
    }
  }
}

bool ensureWiFi(){
  if (WiFi.status()==WL_CONNECTED) return true;
  WiFi.reconnect();
  for (int i=0;i<10;i++){delay(1000);if(WiFi.status()==WL_CONNECTED)return true;}
  return false;
}

void setup(){
  Serial.begin(115200);

  pinMode(BUZZER_PIN,OUTPUT); digitalWrite(BUZZER_PIN,LOW);
  pinMode(BUTTON_PIN,INPUT_PULLUP);

  tft.init(); tft.setRotation(0); tft.fillScreen(TFT_BLACK);

  tft.setTextColor(TFT_WHITE,TFT_BLACK);
  tft.drawCentreString("Laczenie z WiFi...",SCREEN_W/2,140,2);
  WiFi.begin(ssid,password);
  while(WiFi.status()!=WL_CONNECTED) delay(500);
  tft.fillScreen(TFT_BLACK); tft.setTextColor(TFT_GREEN,TFT_BLACK);
  tft.drawCentreString("Polaczono!",SCREEN_W/2,130,2); delay(500);

  setupTime();

  tft.fillScreen(TFT_BLACK); tft.setTextColor(TFT_WHITE,TFT_BLACK);
  tft.drawCentreString("Ladowanie przystankow...",SCREEN_W/2,140,2);
  loadBuiltInStops();
  if (allStops.empty()){
    tft.drawCentreString("Blad!",SCREEN_W/2,160,2);
    delay(3000); ESP.restart();
  }

  appState=STATE_MENU_STOP; menuCursor=0; menuScroll=0;
  drawStopMenu();
}

void loop(){
  unsigned long now=millis();

  if (appState!=STATE_DEPARTURES){
    ir.task();
    if (ir.nec_available()){
      uint32_t code=ir.data();

      if      (code==IR_DOWN) menuNavigate(+1);
      else if (code==IR_UP)   menuNavigate(-1);
      else if (code==IR_OK){

        if (appState==STATE_MENU_STOP){
          StopEntry& s=allStops[menuCursor];
          selectedStopId=s.id; selectedStopNr=s.nr; selectedStopName=s.name;
          tft.fillScreen(TFT_BLACK); tft.setTextColor(TFT_WHITE,TFT_BLACK);
          tft.drawCentreString("Pobieranie linii...",SCREEN_W/2,140,2);
          fetchLines();
          appState=STATE_MENU_LINE; menuCursor=0; menuScroll=0;
          drawLineMenu();
        }

        else if (appState==STATE_MENU_LINE){
          if (menuCursor==0){
            selectedLine=""; alarmMode=-1; globalMute=false;
            tft.fillScreen(TFT_BLACK); tft.setTextColor(TFT_WHITE,TFT_BLACK);
            tft.drawCentreString("Pobieranie odjazdow...",SCREEN_W/2,140,2);
            if (stopLines.empty()) fetchLines();
            updateDeparturesAllLines();
            appState=STATE_DEPARTURES;
            TimeNow t=getTimeNow(); lastMinute=t.valid?t.minute:-1;
            lastClockRefresh=now; buzzPhase=0; lastBuzzAt=now;
            drawDepartureHeader(); displayDepartures();
          } else {
            selectedLine=stopLines[menuCursor-1];
            tft.fillScreen(TFT_BLACK); tft.setTextColor(TFT_WHITE,TFT_BLACK);
            tft.drawCentreString("Pobieranie rozkladu...",SCREEN_W/2,140,2);
            fetchAlarmOptions();
            appState=STATE_MENU_ALARM; menuCursor=0; menuScroll=0;
            drawAlarmMenu();
          }
        }

        else if (appState==STATE_MENU_ALARM){
          if (menuCursor==0){
            alarmMode=0; alarmTargetTime="";
          } else {
            alarmMode=menuCursor;
            alarmTargetTime=alarmOptions[menuCursor].scheduledTime;
          }
          globalMute=false;
          tft.fillScreen(TFT_BLACK); tft.setTextColor(TFT_WHITE,TFT_BLACK);
          tft.drawCentreString("Pobieranie odjazdow...",SCREEN_W/2,140,2);
          updateDepartures();
          appState=STATE_DEPARTURES;
          TimeNow t=getTimeNow(); lastMinute=t.valid?t.minute:-1;
          lastClockRefresh=now; buzzPhase=0; lastBuzzAt=now;
          drawDepartureHeader(); displayDepartures();
        }
      }
    }
    delay(20);
    return;
  }

  handleButton();

  TimeNow t=getTimeNow();
  if (!t.valid){updateBuzzer();delay(20);return;}

  if (t.minute!=lastMinute){
    lastMinute=t.minute;

    if (t.hour==0 && t.minute==0){
      for (auto& d:currentDepartures) d.mutedForToday=false;
      globalMute=false;
    }

    if (ensureWiFi()){
      if (alarmMode>=0) updateDepartures();
      else              updateDeparturesAllLines();
      drawDepartureHeader(); displayDepartures();
      lastClockRefresh=now;
    }
  }
  else if (now-lastClockRefresh>=CLOCK_INTERVAL){
    lastClockRefresh=now;
    refreshCountdowns();
    displayDepartures();
  }

  updateBuzzer();
  delay(20);
}
