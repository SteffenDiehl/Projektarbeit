#include <Arduino.h>
#ifdef ESP32
  #include <WiFi.h>
  #include <AsyncTCP.h>
  #include <SPIFFS.h>
#else
  #include <ESP8266WiFi.h>
  #include <ESPAsyncTCP.h>
  #include <Hash.h>
  #include <FS.h>
#endif
#include <ESPAsyncWebServer.h>

AsyncWebServer server(80);
const char* ssid = "DiehlWithIt";
const char* password = "DiehlWithIt";

IPAddress staticIP(192, 168, 43, 68); // Die gewünschte IP-Adresse
IPAddress gateway(192, 168, 43, 1);    // Das Gateway
IPAddress subnet(255, 255, 255, 0);   // Die Subnetzmaske
IPAddress dns(8, 8, 8, 8);

const char* start_now_timer = "start_now_timer";
int now_timer = 0;
const char* left_now_timer = "left_now_timer";
int left_timer = 0;
const char* start_timer1 = "start_timer1";
int start1 = 0;
const char* end_timer1 = "end_timer1";
int end1 = 0;
const char* start_timer2 = "start_timer2";
int start2 = 0;
const char* end_timer2 = "end_timer2";
int end2 = 0;

const char* PARAM_CURRENTTIME = "currentTime";

String Date = "";
const char* ptr_Date = Date.c_str();
int *web_year = nullptr;
int *web_month = nullptr;
int *web_day = nullptr;

String Time = "";
const char* ptr_Time = Time.c_str();
int *web_hour = nullptr;
int *web_min = nullptr;
int *web_sec = nullptr;

int *web_status = nullptr;
String web_status_color[3] = {"DarkGreen", "DarkOrange", "FireBrick"};

unsigned long _timer[5] = {0, 0, 0, 0, 0};

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html><body style="background-color: lightgreen;">
  <head>
    <meta http-equiv="refresh" content="10">
    <script>
    function submitMessage() {
      alert("Saved value to ESP SPIFFS");
      setTimeout(function(){ document.location.reload(false); }, 500);   
    }
    function startMessage() {
      alert("new Timer started");
      setTimeout(function(){ document.location.reload(false); }, 500);   
    }
    function startnow() {
      alert("Maerkel starts");
      setTimeout(function(){ document.location.reload(false); }, 500);   
    }
    function stopnow() {
      alert("Maerkel stops");
      setTimeout(function(){ document.location.reload(false); }, 500);   
    }
    function gohome() {
      alert("Maerkel goes home");
      setTimeout(function(){ document.location.reload(false); }, 500);   
    }
    </script>
    <title>Maehrkel</title>
    <link rel="icon" type="image/jpg" href="/images/Merkel.jpg">
  </head>
  <style>
  /* Define the CSS style for the colored square */
  .colored-square {
    width: 100px; /* Adjust the width as needed */
    height: 100px; /* Adjust the height as needed */
    background-color: %statuscolor%; /* Change the color using a hexadecimal code */
  }
  </style>
  <body>
    <form target= "hidden-form">
      <h1>Maehrkel<small><small><small> by Daniel Becher, Benedikt Buettner, Steffen Diehl </small></small></small></h1>
      <h2>Date: %currentDate%</h2>
      <h2>Time: %currentTime%</h2>
  <p><img src="https://www.bundeskanzler.de/resource/image/1860510/16x9/1023/575/1f8092e18cf8a8a122753b918d3e7016/aH/bundeskanzlerin-angela-merkel-portraet-2.jpg" alt="Smiley face" style="float:right;width:800px;height:450px;"></p>
    </form>
    <br>
    <form action="/stopnow" target="hidden-form">
      <input type="submit" style = background-color:FireBrick value="Stop now" onclick="stopnow()">
    </form>
    <form action="/gohome" target="hidden-form">
      <input type="submit" style = background-color:DarkOrange value="Go home" onclick="gohome()">
    </form>
    <br>
    <br>
    <form target= "hidden-form">
      Timer: %left_now_timer% seconds / %StartNow% seconds left
    </form>
    <form action="/startnow" target="hidden-form">
      <input type="submit" style = background-color:DarkGreen value="Start now" onclick="startnow()">
    </form>
    <br>
    <body>
      <!-- Create the colored square div element -->
      <div class="colored-square"></div>
    </body>
    <br>
    <form target= "hidden-form">
      <strong>Timer 1</strong>
      <br>
      Start: %timer1value% seconds
      &nbsp; &nbsp; &nbsp;
      Time till start: %timer4left% seconds
    <br>
      End: %timer2value% seconds
      &nbsp; &nbsp; &nbsp;
      Time till end: %timer4left% seconds
    </form>
    <br>
    <br>
    <form target= "hidden-form">
      <strong>Timer 2</strong>
      <br>
      Start: %timer3value% seconds
      &nbsp; &nbsp; &nbsp;
      Time till start: %timer4left% seconds
    <br>
      End: %timer4value% seconds
      &nbsp; &nbsp; &nbsp;
      Time till end: %timer4left% seconds
    </form>
    <br>
    <form action="/get" target="hidden-form">
      Start now Timer (current value %StartNow%): <input type="text" name="start_now_timer">
      <br>
      <br>
      Start Time 1 (current value %Start1%): <input type="text" name="start_timer1">
      End Time 1 (current value %End1% ms): <input type="number " name="end_timer1">
      <br>
      Start Time 2 (current value %Start2%): <input type="text" name="start_timer2">
      End Time 2 (current value %End2% ms): <input type="number " name="end_timer2">
      <br>
      <input type="submit" value="Submit" onclick="submitMessage()">
    </form>
    <br>
    <br>
  <iframe style="display:none" name="hidden-form"></iframe>
  </body>
</html>)rawliteral";
  // <form action="/start" target="hidden-form">
  //   Timer %currentTimer%:
  //   <label>festeTimerName:</label>
  //   <select name="festeTimerNameSelect">
  //   <option value="%festerTimer1%">%festerTimerName1%</option>
  //   <option value="%festerTimer2%">%festerTimerName2%</option>
  //   <option value="%festerTimer3%">%festerTimerName3%</option>
  //   <option value="%festerTimer4%">%festerTimerName4%</option>
  //   <option value="%festerTimer5%">%festerTimerName5%</option>
  //   <option value="%festerTimer6%">%festerTimerName6%</option>
  //   <option value="%festerTimer7%">%festerTimerName7%</option>
  //   <option value="%festerTimer8%">%festerTimerName8%</option>
  //   <option value="%festerTimer9%">%festerTimerName9%</option>
  //   <option value="%festerTimer10%">%festerTimerName10%</option>
  //   </select>
  //   <input type="submit" value="Start a new Timer" onclick="startMessage()">
  // </form>

// Replaces placeholder with stored values
void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

// Replaces placeholder with stored values
String processor(const String& var){
  if(var == "StartNow"){
    return String(now_timer);
  }
  else if(var == "Start1"){
    return String(start1);
  }
  else if(var == "End1"){
    return String(end1);
  }
  else if(var == "Start2"){
    return String(start2);
  }
  else if(var == "End2"){
    return String(end2);
  }
  else if(var == "currentTime"){
    return Time;
  }
  else if(var == "currentDate"){
    return Date;
  }
  else if(var== "left_now_timer") {
      return String(left_timer);
  }
  else if(var== "currentTimer") {
      return String(1);
  }
  else if(var== "statuscolor") {
      return web_status_color[*web_status];
  }
  return String();
}

void setup_webbrwoser(int *c_hour, int *c_min, int *c_sec, int *c_day, int *c_month, int *c_year, int *c_status) {
  web_hour = c_hour;
  web_min = c_min;
  web_sec = c_sec;
  web_day = c_day;
  web_month = c_month;
  web_year = c_year;
  web_status = c_status;
  // Initialize SPIFFS
  #ifdef ESP32
    if(!SPIFFS.begin(true)){
      Serial.println("An Error has occurred while mounting SPIFFS");
      return;
    }
  #else
    if(!SPIFFS.begin()){
      Serial.println("An Error has occurred while mounting SPIFFS");
      return;
    }
  #endif

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("WiFi Failed!");
    return;
  }
  WiFi.config(staticIP, gateway, subnet, dns);
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Send web page with input fields to client
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  server.on("/gohome", HTTP_GET, [](AsyncWebServerRequest *request){
    *web_status = 1;
  });

  server.on("/stopnow", HTTP_GET, [](AsyncWebServerRequest *request){
    *web_status = 2;
  });

  server.on("/startnow", HTTP_GET, [](AsyncWebServerRequest *request){
    *web_status = 0;
  });

  server.on("/Date", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", ptr_Date);
  });

  server.on("/Time", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", ptr_Time);
  });
  server.on("/Status", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", (String(*web_status)).c_str());
  });

  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    unsigned long changetimer;
    String inputMessage;
    if (request->hasParam(start_now_timer)) {
      inputMessage = request->getParam(start_now_timer)->value();
      if(inputMessage != ""){
        changetimer = (inputMessage.toInt())*1000;
        now_timer = changetimer;
      }
    }
    if (request->hasParam(start_timer1)) {
      inputMessage = request->getParam(start_timer1)->value();
      if(inputMessage != ""){
        changetimer = (inputMessage.toInt())*1000;
        start1 = changetimer;
      }
    }
    if (request->hasParam(end_timer1)) {
      inputMessage = request->getParam(end_timer1)->value();
      if(inputMessage != ""){
        changetimer = (inputMessage.toInt())*1000;
        end1 = changetimer;
      }
    }
    if (request->hasParam(start_timer2)) {
      inputMessage = request->getParam(start_timer2)->value();
      if(inputMessage != ""){
        changetimer = (inputMessage.toInt())*1000;
        start2 = changetimer;
      }
    }
    if (request->hasParam(end_timer2)) {
      inputMessage = request->getParam(end_timer2)->value();
      if(inputMessage != ""){
        changetimer = (inputMessage.toInt())*1000;
        end2 = changetimer;
      }
    }
    request->send(200, "text/text", inputMessage);
  });
  server.on("/start", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("start");
    });
  server.onNotFound(notFound);
  server.begin();
}

void web_browser_end(){
  server.end();
}
String formatDigits(int number) {
  if (number < 10) {
    return "0" + String(number);
  } else {
    return String(number);
  }
}

void set_date_time() {
  String d = formatDigits(*web_day);
  String m = formatDigits(*web_month);
  Date = d + "." + m + "." + String(*web_year);
  Time = formatDigits(*web_hour) + ":" + formatDigits(*web_min) + ":" + formatDigits(*web_sec);
// Function to format a number as a two-digit string
}