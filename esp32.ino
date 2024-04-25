/*
 * Date: 20240403
   Version: 0.4

   Logs:
   1. Add json decoder and wifi feature
   2. Optimize the wifi and parser script

 * Date: 20240328
 * Version: 0.2
 *
 * Logs:
 * 1. Add the wifi scanner and reconnect function
 *
 *
 * Date: 20240324
 * Version: 0.1
 *
 * Logs:
 * 1. Build the framework
 * 2. Finish the 1st light effect
 *
 */

#include <Adafruit_NeoPixel.h>
#include <Metro.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
// 用于OTA
#include <ESPmDNS.h>
#include <Update.h>

/* Put your HOST SSID & Password */

const char *host = "esp32";

const char *ssid = "ZimaCube";      // Enter SSID here
const char *password = "homecloud"; // Enter Password here

/* Put IP Address details */
IPAddress local_ip(172, 16, 1, 1);
IPAddress gateway(172, 16, 1, 1);
IPAddress subnet(255, 255, 255, 0);

WebServer server(80);

/*
 * Server Index Page
 */

const char *serverIndex =
    "<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
    "<script src='https://cdn.tailwindcss.com'></script>"
    "<link href='https://cdn.jsdelivr.net/npm/daisyui@4.10.1/dist/full.min.css' rel='stylesheet' type='text/css' />"
    "<script src='https://cdn.tailwindcss.com'></script>"
    "<div class='flex flex-col justify-center items-center space-y-4 h-full bg-[rgba(81,_81,_84,_1)]' >"
    "<h1 class='text-2xl font-bold text-[rgba(250,_183,_57,_1)]'>"
    "Update Your File"
    "</h1>"

    "<form "
    " method='POST'"
    " action='#'"
    " enctype='multipart/form-data'"
    " id='upload_form'"
    " class='h-30 space-x-4 flex justify-center items-center rounded-lg p-4'"
    ">"
    "<input "
    " type='file'"
    " name='update'"
    " class='text-white file:cursor-pointer file:bg-[rgba(0,_122,_229,_1)] file:mr-4 file:py-2 file:px-4 file:rounded-full file:border-0 file:text-sm file:font-semibold file:text-white hover:file:text-black hover:file:bg-violet-100'"
    "/>"
    "<input "
    " type='submit'"
    " value='Update'"
    " class='cursor-pointer mr-4 py-2 px-4 border-0 text-white hover:text-black bg-[rgba(0,_122,_229,_1)] hover:bg-white rounded-full text-sm font-semibold'"
    "/>"
    " </form>"
    "<div id='prg'>progress: 0%</div>"
    "</div>"
    "<script>"
    "$('form').submit(function (e) {"
    "e.preventDefault();"
    "var form = $('#upload_form')[0];"
    "var data = new FormData(form);"
    "$.ajax({"
    "url: '/update',"
    "type: 'POST',"
    "data: data,"
    "contentType: false,"
    "processData: false,"
    "xhr: function () {"
    "var xhr = new window.XMLHttpRequest();"
    "xhr.upload.addEventListener("
    "'progress',"
    "function (evt) {"
    "if (evt.lengthComputable) {"
    "var per = evt.loaded / evt.total;"
    "$('#prg').html('progress: ' + Math.round(per * 100) + '%');"
    "}"
    "},"
    "false"
    ");"
    "return xhr;"
    "},"
    "success: function (d, s) {"
    "console.log('success!');"
    "},"
    "error: function (a, b, c) {},"
    "});"
    "});"
    "</script>";

DynamicJsonDocument doc(1024);

boolean cmdEvent = false;

// Pin to use to send signals to WS2812B
/* WS2812 Configuration info */
#define LED_PIN 2
#define NUM_PIXELS 10
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_PIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

// Timer controllers

Metro colorAlgorithmTimer = Metro(15);
Metro lightUpdater = Metro(10);
Metro debugTimer = Metro(100);

/*light_controller_parameter*/
int effectId = 2;    // 1 ~ 5
int effectSpeed = 1; // 1 ~ 5

// Color parameter
int hue = 17;       // 0 ~ 359
int saturation = 0; // 0 ~ 255
int lightness = 0;  // 0 ~ 255

uint32_t colors[10] = {};

void overTheAir_server()
{

  /*return index page which is stored in serverIndex */
  server.on("/", HTTP_GET, []()
            {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex); });
  server.on("/version", HTTP_GET, []()
            { server.send(200, "application/json", "{\"version\":\"0.5\"}"); });
  /*handling uploading firmware file */
  server.on(
      "/update", HTTP_POST, []()
      {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart(); },
      []()
      {
        HTTPUpload &upload = server.upload();
        if (upload.status == UPLOAD_FILE_START)
        {
          Serial.printf("Update: %s\n", upload.filename.c_str());
          if (!Update.begin(UPDATE_SIZE_UNKNOWN))
          { // start with max available size
            Update.printError(Serial);
          }
        }
        else if (upload.status == UPLOAD_FILE_WRITE)
        {
          /* flashing firmware to ESP*/
          if (Update.write(upload.buf, upload.currentSize) != upload.currentSize)
          {
            Update.printError(Serial);
          }
        }
        else if (upload.status == UPLOAD_FILE_END)
        {
          if (Update.end(true))
          { // true to set the size to the current progress
            Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
          }
          else
          {
            Update.printError(Serial);
          }
        }
      });
}

void setup()
{
  Serial.begin(115200);
  Serial.println("Initial the ZimaCube light effect system!");

  strip.begin();
  wifiOn();
}

void wifiOn()
{
  WiFi.softAPConfig(local_ip, gateway, subnet);
  WiFi.softAP(ssid, password);
  //  delay(100);

  server.on("/post", HTTP_POST, handle_OnConnect);
  server.onNotFound(handle_NotFound);
  // OTA websever
  overTheAir_server();

  server.begin();
  Serial.println("HTTP server started");
}

void wifiRestart()
{
}

void loop()
{

  server.handleClient();
  if (cmdEvent)
    jsonParser();

  if (colorAlgorithmTimer.check())
  {
    switch (effectId)
    {
    case 1:
      fadeEffect(); // a simple algorithm for ID 1 effect
      break;
    case 2:
      lightOnEffect();
      break;

    default:
      break;
    }
  }

  if (lightUpdater.check())
    render();

  if (debugTimer.check())
    debugger();
}

void debugger()
{

  //  Serial.print("Color Parameter: ");
  //  Serial.println(lightness);
}

// According to the protocol, decode the json package to the global values

void jsonParser()
{

  effectId = doc["id"];
  effectSpeed = constrain(doc["speed"], 0, 5);     // filter noise, control the data value in a reasonable range
  lightness = constrain(doc["lightness"], 0, 255); // filter noise, control the data value in a reasonable range
  JsonArray jsonArray = doc["data"];

  switch (effectId)
  {
  case 1:
    hue = jsonArray[0]["h"];
    saturation = jsonArray[0]["s"];

    //      Serial.print("hue: ");
    //      Serial.print(hue);
    //      Serial.print(", sat: ");
    //      Serial.println(saturation);

    break;

  case 2:
    hue = jsonArray[0]["h"];
    saturation = jsonArray[0]["s"];

    //      Serial.print("hue: ");
    //      Serial.print(hue);
    //      Serial.print(", sat: ");
    //      Serial.println(saturation);

    break;

  case 5:
    for (int i = 0; i < 10; i++)
    {

      unsigned int h = map(jsonArray[i]["h"], 0, 255, 0, 65537);
      unsigned int s = jsonArray[i]["s"];
      unsigned int v = jsonArray[i]["v"];

      colors[i] = strip.gamma32(strip.ColorHSV(h, s, v));
    }
    break;

  default:
    break;
  }

  cmdEvent = false;
  //  Serial.println("update the json package to global parameters!");
}

void handle_OnConnect()
{

  DeserializationError error = deserializeJson(doc, server.arg("plain"));

  if (error)
  {
    Serial.print("Failed to parse JSON: ");
    Serial.println(error.c_str());
    server.send(400, "text/plain", "Bad Request");
    return;
  }

  cmdEvent = true;

  server.send(200, "application/json", "{\"response\":\"Received\"}");
}

void handle_NotFound()
{
  server.send(404, "text/plain", "Not found");
}

void lightOnEffect()
{

  lightness = 255;

  for (int i = 0; i < 10; i++)
  {

    unsigned int h = map(hue, 0, 255, 0, 65537);
    unsigned int s = saturation;
    unsigned int v = lightness;

    colors[i] = strip.gamma32(strip.ColorHSV(h, s, v));
  }
}

void fadeEffect()
{

  lightness = getBrightness(lightness, effectSpeed);

  if (lightness < 1)
    colorAlgorithmTimer.interval(2000); // enhance the interval
  else if (lightness > 254)
    colorAlgorithmTimer.interval(1500); // enhance the interval
  else
    colorAlgorithmTimer.interval(15);

  for (int i = 0; i < 10; i++)
  {

    unsigned int h = map(hue, 0, 255, 0, 65537);
    unsigned int s = saturation;
    unsigned int v = lightness;

    colors[i] = strip.gamma32(strip.ColorHSV(h, s, v));
  }
}

void render()
{

  for (int i = 0; i < 10; i++)
  {
    strip.setPixelColor(i, colors[i]);
  }
  strip.show(); // push the data pack to ws2812b
}

// 定义一个函数，根据输入的step（步长）计算亮度
int getBrightness(int brightness, int delta)
{

  static boolean increasing = true;

  brightness += increasing ? delta : -delta; // 根据方向增加或减少亮度

  if (brightness >= 255 || brightness <= 0)
  {
    increasing = !increasing;                 // 到达边界时反转方向
    brightness = brightness >= 255 ? 255 : 0; // 防止超出范围
  }

  return brightness;
}
