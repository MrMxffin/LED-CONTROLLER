#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Adafruit_NeoPixel.h>
#include <Arduino.h>
#include <cmath>
using namespace std;

//Router properties
#define SSID getenv("WIFI_SSID");          //Router SSID
#define PASSWORD getenv("WIFI_PASSWORD");  //Router Passwort
//Hardware properties
#define PIN getenv(PIN)
#define LED_COUNT getenv(LED_COUNT)
//Server properties
#define PORT getenv(PORT)

Adafruit_NeoPixel ledStrip(LED_COUNT, PIN, NEO_GRB + NEO_KHZ800);
ESP8266WebServer server(PORT);  //Declare Webserver on Port


//Initialize
String hexColor = "#000000";
int statusCode = 0, offset = 0, x = 0, counter = 0;
int rainbowRGB[3][LED_COUNT];
int sexyTimeRGB[3][LED_COUNT];
int dresdenRGB[3][LED_COUNT];
int rgbColor[3] = { 0, 0, 0 };
int ornamentColors[2][3] = {{13,255,12},{255,13,12}};
bool darkMode = true;
bool rising;

////////
//HTML//
////////

String getPage() {
  String palette;
  if (darkMode) {
    palette = R"(
input {color:white;background-color:#15202b;border:2px solid #203142}
input:hover {background-color:#203142}
select {color:white;background-color:#15202b;border:2px solid #203142}
select:hover{background-color:#203142}
body {color:white;background-color:#15202b})";
  } else {
    palette = R"(
input {color:#484b6a;background-color:#fafafa;border:2px solid #484b6a}
input:hover {color:#fafafa;background-color:#484b6a}
select {color:#484b6a;background-color:#fafafa;border:2px solid #484b6a}
select:hover{color:#fafafa;background-color:#484b6a}
body {color:#484b6a;background-color:#fafafa})";
  }
  String page = R"(
<!DOCTYPE HTML>
<html>
<script>
if ( window.history.replaceState ) {window.history.replaceState( null, null, window.location.href );}
</script>
<head>
<meta name = "viewport" content = "width = device-width, initial-scale = 1.0 maximum-scale = 2.5, user-scalable=1">
<title>LED Controller</title>
<style>
)" + palette + R"(
input {text-align:center;font-size: 26px;transition-duration:0.5s; margin:auto;cursor:pointer;border-radius:2px;text-align:center;cursor:pointer;font-size: 16px; display: block; height: 50px; width: 100%}
select {text-align:center;font-size: 26px;transition-duration:0.5s; margin:auto;cursor:pointer;border-radius:2px;text-align:center;cursor:pointer;font-size: 16px; display: block; height: 50px; width: 100%}
body {text-align: center;font-family: "Century Gothic", CenturyGothic, AppleGothic, sans-serif; font-style: normal; font-variant: normal}
h1 {font-size: 40px; font-weight: 700; line-height: 26.4px; }
h2 {font-size: 28px}
p {font-size: 14px;font-weight: 400; line-height: 20px;}
</style>
</head>
<body>
<h1>LED Controller</h1>
<h2>Pick a color.</h2>
<form action="/" method="post">
<input type="color" name="colorPicker"  id="colorPicker" required="true" value=")"
                + hexColor + R"(">
<br>
<input type="submit" value="Submit">
</form>
<h2>Rainbows und Pride</h2>
<form action="/" method="post">
<select name="rainbows" id="rainbows">
<option value=rainbow1>Rainbow 1</option>
<option value=rainbow2>Rainbow 2</option>
<option value=t-balls>Trippin Balls</option>
<option value=gaypride>Gay Pride</option>
<option value=transpride>Trans Pride</option>
<option value=bipride>Bisexual Pride</option>
</select>
<br>
<input type="submit" value="Submit">
</form>
<h2>Miscellaneous Effects</h2>
<form action="/" method="post">
<select name="misc" id="misc">
<option value=dresden1>Dresden wird brennen 1</option>
<option value=dresden2>Dresden wird brennen 2</option>
<option value=sTime>Sexy Time</option>
</select>
<br>
<input type="submit" value="Submit">
</form>
<br>
<h2>Christmas</h2>
<form action="/" method="post">
<select name="xmas" id="xmas">
<option value=candyCane>Candy Cane</option>
<option value=ornaments>Christmas Ornaments</option>
<option value=burningTree>Burning Tree</option>
</select>
<br>
<input type="submit" value="Submit">
</form>
<br>
<form action="/" method="post">
<input type="submit" name="toggleDarkMode" value="Press here to toggle dark mode.">
</form>
<br>
<p>Made with love by Neo and Rostig</p>
</body>
</html>
  )";
  return page;
}

///////////
//Handler//
///////////

void handleNotFound() {
  server.send(200, "text/html", getPage());
}

void handleSubmit() {
  if (server.hasArg("colorPicker")) {
    statusCode = 0;
    hexColor = server.arg("colorPicker");

    //Extract Color Values out of string
    int number = (int)strtol(&hexColor[1], NULL, 16);
    rgbColor[0] = number >> 16;
    rgbColor[1] = number >> 8 & 0xFF;
    rgbColor[2] = number & 0xFF;

    //Change color
    changeLEDColor(rgbColor[0], rgbColor[1], rgbColor[2]);
    Serial.print("LED Mode: Static color ");
    Serial.println(hexColor);

  } else if (server.hasArg("rainbows")) {
    String rainbowType = server.arg("rainbows");
    if (rainbowType == "rainbow1") {
      Serial.println("LED Mode: Rainbow 1");
      offset = 0;
      statusCode = 1;
    } else if (rainbowType == "rainbow2") {
      Serial.println("LED Mode: Rainbow 2");
      statusCode = 2;
      rgbColor[0] = 255;
      rgbColor[1] = 0;
      rgbColor[2] = 0;
    } else if (rainbowType == "t-balls") {
      Serial.println("LED Mode: Trippin Balls");
      statusCode = 3;
    } else if (rainbowType == "gaypride") {
      Serial.println("LED Mode: Gay Pride");
      statusCode = 0;
      gayPride();
    } else if (rainbowType == "transpride") {
      Serial.println("LED Mode: Trans Pride");
      statusCode = 0;
      transPride();
    } else if (rainbowType == "bipride") {
      statusCode = 0;
      Serial.println("LED Mode: Bisexual Pride");
      biPride();
    }
  } else if (server.hasArg("misc")) {
    String miscType = server.arg("misc");
    if (miscType == "dresden1") {
      changeLEDColor(0, 0, 0);
      Serial.println("LED Mode: Dresden wird brennen 1");
      statusCode = 4;
    } else if (miscType == "dresden2") {
      Serial.println("LED Mode: Dresden wird brennen 2");
      offset = 0;
      statusCode = 5;
    } else if (miscType == "sTime") {
      Serial.println("LED Mode: Sexy Time");
      rising = true;
      rgbColor[2] = 0;
      offset = 0;
      statusCode = 6;
    }
  } else if (server.hasArg("xmas")){
    String xmasType = server.arg("xmas");
    if (xmasType=="candyCane"){
      Serial.println("LED Mode: Candy Cane");
      offset = 0;
      statusCode = 7;
    } else if (xmasType=="ornaments"){
      Serial.println("LED Mode: Ornaments");
      statusCode = 8;
    } else if (xmasType=="burningTree"){
      statusCode =  9;    
      Serial.println("LED Mode: Burning Tree");
      counter = 0;
    }
  } else if (server.hasArg("toggleDarkMode")) {
    darkMode = !darkMode;
  }
  //Response to the HTTP request
  server.send(200, "text/html", getPage());
}


void handleRoot() {
  if (server.args()) {
    handleSubmit();
  } else {
    server.send(200, "text/html", getPage());
  }
}

void handleLEDs() {
  switch (statusCode) {
    case 1:
      rainbow1();
      break;
    case 2:
      rainbow2();
      break;
    case 3:
      trippinballs();
      break;
    case 4:
      dresden1();
      break;
    case 5:
      dresden2();
      break;
    case 6:
      sexyTime();
      break;
    case 7:
      candyCane();
      break;
    case 8:
      ornaments();
      break;
    case 9:
      burningTree();
      break;
  }
}

////////////////////
//Fancy LED action//
////////////////////

void changeLEDColor(int red, int green, int blue) {
  for (int i = 0; i < LED_COUNT; i++) {
    ledStrip.setPixelColor(i, red, green, blue);
    ledStrip.show();
    delay(20);
  }
}

void rainbow1() {
  if (x < LED_COUNT) {
    if (x - offset >= 0) {
      ledStrip.setPixelColor(x, rainbowRGB[0][x - offset], rainbowRGB[1][x - offset], rainbowRGB[2][x - offset]);
    } else {
      ledStrip.setPixelColor(x, rainbowRGB[0][LED_COUNT + x - offset], rainbowRGB[1][LED_COUNT + x - offset], rainbowRGB[2][LED_COUNT + x - offset]);
    }
    ledStrip.show();
    x++;
  } else {
    x = 0;
    offset++;
    if (offset >= LED_COUNT) {
      offset = 0;
    }
  }
}

void rainbow2() {
  if (rgbColor[0] == 255 && rgbColor[1] != 255 && rgbColor[2] == 0) {
    rgbColor[1]++;
  } else if (rgbColor[1] == 255 && rgbColor[0] != 0) {
    rgbColor[0]--;
  } else if (rgbColor[1] == 255 && rgbColor[2] != 255) {
    rgbColor[2]++;
  } else if (rgbColor[2] == 255 && rgbColor[1] != 0) {
    rgbColor[1]--;
  } else if (rgbColor[2] == 255 && rgbColor[0] != 255) {
    rgbColor[0]++;
  } else if (rgbColor[0] == 255 && rgbColor[2] != 0) {
    rgbColor[2]--;
  }
  changeLEDColor(rgbColor[0], rgbColor[1], rgbColor[2]);
}

void trippinballs() {
  if (x < LED_COUNT) {
    int randomLED = random(1, LED_COUNT - 1);
    ledStrip.setPixelColor(x, rainbowRGB[0][randomLED], rainbowRGB[1][randomLED], rainbowRGB[2][randomLED]);
    ledStrip.setPixelColor(x + 1, rainbowRGB[0][randomLED], rainbowRGB[1][randomLED], rainbowRGB[2][randomLED]);
    ledStrip.setPixelColor(x - 1, rainbowRGB[0][randomLED], rainbowRGB[1][randomLED], rainbowRGB[2][randomLED]);
    ledStrip.show();
    x++;
  } else {
    x = 0;
  }
}

void dresden1() {
  int greenVal = random(0, 50) * 3;
  int randomLED = random(0, LED_COUNT);
  ledStrip.setPixelColor(randomLED, 255, greenVal, 0);
  ledStrip.show();
}

void dresden2() {
  if (x < LED_COUNT) {
    if (x - offset >= 0) {
      ledStrip.setPixelColor(x, dresdenRGB[0][x - offset], dresdenRGB[1][x - offset], dresdenRGB[2][x - offset]);
    } else {
      ledStrip.setPixelColor(x, dresdenRGB[0][LED_COUNT + x - offset], dresdenRGB[1][LED_COUNT + x - offset], dresdenRGB[2][LED_COUNT + x - offset]);
    }
    ledStrip.show();
    x++;
  } else {
    x = 0;
    offset++;
    if (offset >= LED_COUNT) {
      offset = 0;
    }
  }
}

void sexyTime() {
  if (rgbColor[2] != 255 && rising) {
    rgbColor[2]++;
  } else if (rgbColor[2] == 255 && rising) {
    rgbColor[2]--;
    rising = false;
  } else if (rgbColor != 0 && !rising) {
    rgbColor[2]--;
  } else {
    rgbColor[2]++;
    rising = true;
  }
  changeLEDColor(255, 0, rgbColor[2]);
}

void gayPride() {
  int sectionLength = LED_COUNT / 6;
  for (int i = 0; i < LED_COUNT; i++) {
    if (i < sectionLength) {
      ledStrip.setPixelColor(i, 228, 3, 3);
      ledStrip.show();
    } else if (i < sectionLength * 2) {
      ledStrip.setPixelColor(i, 255, 140, 0);
      ledStrip.show();
    } else if (i < sectionLength * 3) {
      ledStrip.setPixelColor(i, 255, 237, 0);
      ledStrip.show();
    } else if (i < sectionLength * 4) {
      ledStrip.setPixelColor(i, 0, 128, 38);
      ledStrip.show();
    } else if (i < sectionLength * 5) {
      ledStrip.setPixelColor(i, 36, 64, 142);
      ledStrip.show();
    } else {
      ledStrip.setPixelColor(i, 115, 41, 130);
      ledStrip.show();
    }
    delay(10);
  }
}

void transPride() {
  int sectionLength = LED_COUNT / 5;
  for (int i = 0; i < LED_COUNT; i++) {
    if (i < sectionLength) {
      ledStrip.setPixelColor(i, 91, 206, 250);
      ledStrip.show();
    } else if (i < sectionLength * 2) {
      ledStrip.setPixelColor(i, 245, 169, 184);
      ledStrip.show();
    } else if (i < sectionLength * 3) {
      ledStrip.setPixelColor(i, 255, 255, 255);
      ledStrip.show();
    } else if (i < sectionLength * 4) {
      ledStrip.setPixelColor(i, 245, 169, 184);
      ledStrip.show();
    } else {
      ledStrip.setPixelColor(i, 91, 206, 250);
      ledStrip.show();
    }
    delay(10);
  }
}

void biPride() {
  int sectionLength = LED_COUNT / 5;
  for (int i = 0; i < LED_COUNT; i++) {
    if (i < sectionLength * 2) {
      ledStrip.setPixelColor(i, 214, 2, 112);
      ledStrip.show();
    } else if (i < sectionLength * 3) {
      ledStrip.setPixelColor(i, 155, 79, 150);
      ledStrip.show();
    } else {
      ledStrip.setPixelColor(i, 0, 56, 168);
      ledStrip.show();
    }
    delay(10);
  }
}

void candyCane() {
  offset++;
  offset=offset%LED_COUNT;
  for (int i = 0; i < LED_COUNT; i++) {
    if (((int) ceil(i/10))%2==1){
      ledStrip.setPixelColor ((i+offset)%LED_COUNT,255,0,0);
    } else {
      ledStrip.setPixelColor ((i+offset)%LED_COUNT,255,255,255);
    }
    ledStrip.show();
  }
  delay(1000);
}

void ornaments(){
  
  int randomArray[5];

  for (int i = 0; i < 5;i++){
    randomArray[i] = random(10)%2;
  }

  for (int i = 0; i < LED_COUNT; i++) {
    int currBlock = (int) ceil(i/10);
    int color[3]; 
    for (int i=0;i<3;i++){
      color[i]=ornamentColors[randomArray[currBlock]][i];
    }
    if (((int) ceil(i/10))%2==1){
      ledStrip.setPixelColor (i%LED_COUNT,ornamentColors[0][0],ornamentColors[0][1],ornamentColors[0][2]);
    } else {
      ledStrip.setPixelColor (i%LED_COUNT,color[0],color[1],color[2]);
    }
    ledStrip.show();
  }
  delay(1000);

}

void burningTree(){
    if (counter == 0) {
    changeLEDColor(13,255,12);
  } else if (counter>100){
  ledStrip.setPixelColor(random(LED_COUNT), 255, random(150), 0);
  ledStrip.show();
  counter = counter % 10000;
  }
  counter++;
}

//////////////////
//Setup and Loop//
//////////////////

void setup() {
  ledStrip.begin();                                 //startLEDs
  ledStrip.clear();
  ledStrip.show();
  delay(1000);
  Serial.begin(115200);
  connectWifi(SSID, PASSWORD);
  startWebserver();
  initializeNeoArrays();
}

void loop() {
  server.handleClient();
  if (statusCode != 0) {
    handleLEDs();
  }
  delay(50);
}

void connectWifi(const char* ssid, const char* password) {
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);              //Connect to Wi-Fi
  while (WiFi.status() != WL_CONNECTED) {  //Print dots until connected
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP()); /*
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);*/
}

void startWebserver() {
  server.begin();
  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);
  server.begin();
  delay(500);
  Serial.print("Port: ");
  Serial.println(PORT);
}

void initializeNeoArrays() {
  int inval = 255 * 3 / LED_COUNT;
  for (int i = 0; i < LED_COUNT; i++) {
    if (i < LED_COUNT / 3) {
      rainbowRGB[0][i] = 255 - inval * i;
      rainbowRGB[1][i] = inval * i;
      rainbowRGB[2][i] = 0;
    } else if (i < 2 * LED_COUNT / 3) {
      rainbowRGB[0][i] = 0;
      rainbowRGB[1][i] = 255 - 5.1 * (i - LED_COUNT / 3);
      rainbowRGB[2][i] = inval * (i - LED_COUNT / 3);
    } else {
      rainbowRGB[0][i] = inval * (i - 2 * LED_COUNT / 3);
      rainbowRGB[1][i] = 0;
      rainbowRGB[2][i] = 255 - inval * (i - 2 * LED_COUNT / 3);
    }
  }

  for (int i = 0; i < LED_COUNT; i++) {
    if (i <= 75) {
      dresdenRGB[0][i] = 255;
      dresdenRGB[1][i] = i * 2;
      dresdenRGB[2][i] = 0;
    } else {
      dresdenRGB[0][i] = 255;
      dresdenRGB[1][i] = dresdenRGB[1][30] - (i * 2 - dresdenRGB[1][29]);
      dresdenRGB[2][i] = 0;
    }
  }
}