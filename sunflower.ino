//#include <ETH.h>
//#include <WiFi.h>
//#include <WiFiAP.h>
#include <WiFiClient.h>
//#include <WiFiGeneric.h>
//#include <WiFiMulti.h>
//#include <WiFiScan.h>
#include <WiFiServer.h>
//#include <WiFiSTA.h>
//#include <WiFiType.h>
//#include <WiFiUdp.h>

/*------------------------------------------------------------------------------
  11/03/2020
  Author: Szymanski Works LLC
  Platforms: ESP8266
  Language: C++/Arduino
  File: sunflower.ino
  ------------------------------------------------------------------------------
  Description: 
  Code for Sunflower neopixels that allows controller to control lighting

  Websocket programming based on this tutorial:  
  
                  https://www.youtube.com/watch?v=ROeT-gyYZfw
                  
------------------------------------------------------------------------------*/
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <FastLED.h>

#define DATA_PIN  5
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define NUM_LEDS    71
#define LLEAF_ST 0
#define LLEAF_SZ 8
#define RLEAF_ST 8
#define RLEAF_SZ 8
#define ALL_LEAF_ST 0
#define ALL_LEAF_SZ 16
#define PETALS_ST 16
#define PETALS_SZ 24
#define CENTER_ST 40
#define CENTER_SZ 31

CRGB leds[NUM_LEDS];
#define BRIGHTNESS          96
#define FRAMES_PER_SECOND  120

#define DEBUG

ESP8266WebServer server;
WebSocketsServer webSocket = WebSocketsServer(81);

uint8_t pin_led = 2;
char* ssid = "Rainbow";
char* password = "Un1c0rn!";

char webpage[] PROGMEM = R"=====(
<html>
<head>
<style>
.pollengroup{
  ;
}
.petalgroup{
  ;
}
.leavesgroup{
  ;
}

.slidecontainer {
  width: 100%;
}

.slider {
  -webkit-appearance: none;
  width: 80%;
  height: 15px;
  border-radius: 5px;
  background: #d3d3d3;
  outline: none;
  opacity: 0.7;
  -webkit-transition: .2s;
  transition: opacity .2s;
}

.slider:hover {
  opacity: 1;
}

.slider::-webkit-slider-thumb {
  -webkit-appearance: none;
  appearance: none;
  width: 25px;
  height: 25px;
  border-radius: 50%; 
  background: #4CAF50;
  cursor: pointer;
}

.slider::-moz-range-thumb {
  width: 25px;
  height: 25px;
  border-radius: 50%;
  background: #4CAF50;
  cursor: pointer;
}
</style>

</head>
<body onload="javascript:init()">
<div class="pollengroup">
<h1>Pollen</h1>
<div>
<div class="slidecontainer">
 <p><input oninput="changeAll()" onchange="changeAll()" type="range" min="0" max="255" value="127" class="slider" id="myRangeR">
  RED: <span id="demoR"></span></p>
</div>

<div class="slidecontainer">
 <p><input oninput="changeAll()" onchange="changeAll()" type="range" min="0" max="255" value="127" class="slider" id="myRangeG">
  GREEN: <span id="demoG"></span></p>
</div>

<div class="slidecontainer">
 <p><input oninput="changeAll()" onchange="changeAll()" type="range" min="0" max="255" value="127" class="slider" id="myRangeB">
  BLUE: <span id="demoB"></span></p>
</div>
</div>

<div style="width:80px; height: 80px; border: 5px outset black; background: #7F7F7F;"id="demoBack">
</div>

</div>
<hr/>

<div class="petalsgroup">
<h1>Petals</h1>
<div>
<div class="slidecontainer">
 <p><input oninput="changeAllPetals()" onchange="changeAllPetals()" type="range" min="0" max="255" value="127" class="slider" id="petalRangeR">
  RED: <span id="petalR"></span></p>
</div>

<div class="slidecontainer">
 <p><input oninput="changeAllPetals()" onchange="changeAllPetals()" type="range" min="0" max="255" value="127" class="slider" id="petalRangeG">
  GREEN: <span id="petalG"></span></p>
</div>

<div class="slidecontainer">
 <p><input oninput="changeAllPetals()" onchange="changeAllPetals()" type="range" min="0" max="255" value="127" class="slider" id="petalRangeB">
  BLUE: <span id="petalB"></span></p>
</div>
</div>

<div style="width:80px; height: 80px; border: 5px outset black; background: #7F7F7F;"id="petalBack">
</div>

</div>
<hr/>

<div class="leavesgroup">
<h1>Leaves</h1>
<div>
<div class="slidecontainer">
 <p><input oninput="changeAllLeaves()" onchange="changeAllLeaves()" type="range" min="0" max="255" value="127" class="slider" id="leavesRangeR">
  RED: <span id="leavesR"></span></p>
</div>

<div class="slidecontainer">
 <p><input oninput="changeAllLeaves()" onchange="changeAllLeaves()" type="range" min="0" max="255" value="127" class="slider" id="leavesRangeG">
  GREEN: <span id="leavesG"></span></p>
</div>

<div class="slidecontainer">
 <p><input oninput="changeAllLeaves()" onchange="changeAllLeaves()" type="range" min="0" max="255" value="127" class="slider" id="leavesRangeB">
  BLUE: <span id="leavesB"></span></p>
</div>
</div>

<div style="width:80px; height: 80px; border: 5px outset black; background: #7F7F7F;"id="leavesBack">
</div>

</div>
<hr/>


</body>
<script>
  var Socket;
  function init(){
    Socket = new WebSocket('ws://' + window.location.hostname + ':81/');

    var r = 127;
    var g = 127;
    var b = 127;
    document.getElementById('demoBack').style.backgroundColor = "rgb(" + r + "," + g + "," + b + ")";
    document.getElementById('petalBack').style.backgroundColor = "rgb(" + r + "," + g + "," + b + ")";
    document.getElementById('leavesBack').style.backgroundColor = "rgb(" + r + "," + g + "," + b + ")";
  }

var sliderR = document.getElementById("myRangeR");
var outputR = document.getElementById("demoR");
var divback = document.getElementById("demoBack");
outputR.innerHTML = sliderR.value;
sliderR.oninput = function() {
  outputR.innerHTML = this.value;
  changeAll();
}

var sliderG = document.getElementById("myRangeG");
var outputG = document.getElementById("demoG");
outputG.innerHTML = sliderG.value;
sliderG.oninput = function() {
  outputG.innerHTML = this.value;
  changeAll();
}

var sliderB = document.getElementById("myRangeB");
var outputB = document.getElementById("demoB");
outputB.innerHTML = sliderB.value;
sliderB.oninput = function() {
  outputB.innerHTML = this.value;
  changeAll();
}

function changeAll() {
    var r = document.getElementById('demoR').innerHTML;
    var g = document.getElementById('demoG').innerHTML;
    var b = document.getElementById('demoB').innerHTML;
    var hexColor =  'c'+ r + ','+  g + ','+ b ;
    document.getElementById('demoBack').style.backgroundColor = "rgb(" + r + "," + g + "," + b + ")";
    Socket.send(hexColor);
}

var sliderPetalR = document.getElementById("petalRangeR");
var outputPetalR = document.getElementById("petalR");
var divbackPetal = document.getElementById("petalBack");
outputPetalR.innerHTML = sliderPetalR.value;
sliderPetalR.oninput = function() {
  outputPetalR.innerHTML = this.value;
  changeAllPetals();
}

var sliderPetalG = document.getElementById("petalRangeG");
var outputPetalG = document.getElementById("petalG");
outputPetalG.innerHTML = sliderPetalG.value;
sliderPetalG.oninput = function() {
  outputPetalG.innerHTML = this.value;
  changeAllPetals();
}

var sliderPetalB = document.getElementById("petalRangeB");
var outputPetalB = document.getElementById("petalB");
outputPetalB.innerHTML = sliderPetalB.value;
sliderPetalB.oninput = function() {
  outputPetalB.innerHTML = this.value;
  changeAllPetals();
}

function changeAllPetals() {
    var r = document.getElementById('petalR').innerHTML;
    var g = document.getElementById('petalG').innerHTML;
    var b = document.getElementById('petalB').innerHTML;
    var hexColor =  'p'+ r + ','+  g + ','+ b ;
    document.getElementById('petalBack').style.backgroundColor = "rgb(" + r + "," + g + "," + b + ")";
    Socket.send(hexColor);
}

var sliderLeavesR = document.getElementById("leavesRangeR");
var outputLeavesR = document.getElementById("leavesR");
var divbackLeaves = document.getElementById("leavesBack");
outputLeavesR.innerHTML = sliderLeavesR.value;
sliderLeavesR.oninput = function() {
  outputLeavesR.innerHTML = this.value;
  changeAllLeaves();
}

var sliderLeavesG = document.getElementById("leavesRangeG");
var outputLeavesG = document.getElementById("leavesG");
outputLeavesG.innerHTML = sliderLeavesG.value;
sliderLeavesG.oninput = function() {
  outputLeavesG.innerHTML = this.value;
  changeAllLeaves();
}

var sliderLeavesB = document.getElementById("leavesRangeB");
var outputLeavesB = document.getElementById("leavesB");
outputLeavesB.innerHTML = sliderLeavesB.value;
sliderLeavesB.oninput = function() {
  outputLeavesB.innerHTML = this.value;
  changeAllLeaves();
}

function changeAllLeaves() {
    var r = document.getElementById('leavesR').innerHTML;
    var g = document.getElementById('leavesG').innerHTML;
    var b = document.getElementById('leavesB').innerHTML;
    var hexColor =  'l'+ r + ','+  g + ','+ b ;
    document.getElementById('leavesBack').style.backgroundColor = "rgb(" + r + "," + g + "," + b + ")";
    Socket.send(hexColor);
}
</script>
</html>
)=====";

void setup()
{
  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);
  sunflower();
  FastLED.show(); 
  delay(1000);
  
  WiFi.begin(ssid,password);
#ifdef DEBUG
    Serial.begin(115200);
#endif
  while(WiFi.status()!=WL_CONNECTED)
  {
#ifdef DEBUG
    Serial.print(".");
#endif   
    delay(500);
  }
#ifdef DEBUG
  Serial.println("");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
#endif
  server.on("/",[](){
    server.send_P(200, "text/html", webpage);
     });
  server.begin();
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void loop()
{
  webSocket.loop();
  server.handleClient();
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length)
{
  if(type == WStype_TEXT){
    if(payload[0] == 'c'){  // check for center
      char * pEnd;
      uint8_t redVal = (uint8_t) strtol((const char *) &payload[1], &pEnd, 10);
      uint8_t greenVal = (uint8_t) strtol( pEnd+1, &pEnd, 10);
      uint8_t blueVal = (uint8_t) strtol( pEnd+1, NULL, 10);
#ifdef DEBUG      
      Serial.println(redVal);
      Serial.println(greenVal);
      Serial.println(blueVal);
#endif
      fillSunFlowerPollen(redVal,greenVal ,blueVal);
      FastLED.show(); 
    }
    if(payload[0] == 'p'){  // check for center
      char * pEnd;
      uint8_t redVal = (uint8_t) strtol((const char *) &payload[1], &pEnd, 10);
      uint8_t greenVal = (uint8_t) strtol( pEnd+1, &pEnd, 10);
      uint8_t blueVal = (uint8_t) strtol( pEnd+1, NULL, 10);
#ifdef DEBUG
      Serial.println(redVal);
      Serial.println(greenVal);
      Serial.println(blueVal);
#endif
      fillSunFlowerPetals(redVal,greenVal ,blueVal);
      FastLED.show(); 
    }
    if(payload[0] == 'l'){  // check for center
      char * pEnd;
      uint8_t redVal = (uint8_t) strtol((const char *) &payload[1], &pEnd, 10);
      uint8_t greenVal = (uint8_t) strtol( pEnd+1, &pEnd, 10);
      uint8_t blueVal = (uint8_t) strtol( pEnd+1, NULL, 10);
#ifdef DEBUG
      Serial.println(redVal);
      Serial.println(greenVal);
      Serial.println(blueVal);
#endif
      fillSunFlowerLeaves(redVal,greenVal ,blueVal);
      FastLED.show(); 
    }
  }
#ifdef DEBUG
    for(int i=0; i< length; i++)
      Serial.print((char) payload[i]);
    Serial.println();
#endif 
}

void fillSunFlowerPollen( uint8_t r,uint8_t g,uint8_t b)
{
   fill_solid( &leds[CENTER_ST],CENTER_SZ, CRGB(r,g,b));     //center 41-71 
}

void fillSunFlowerPetals( uint8_t r,uint8_t g,uint8_t b)
{
   fill_solid( &leds[PETALS_ST],PETALS_SZ, CRGB(r,g,b));     //petals 17-40
}

void fillSunFlowerLeaves( uint8_t r,uint8_t g,uint8_t b)
{
   fill_solid( &leds[ALL_LEAF_ST],ALL_LEAF_SZ, CRGB(r,g,b));     //leaves 1-16
}

void sunflower()
{
  fill_solid( &leds[LLEAF_ST],LLEAF_SZ,  CRGB(34,139,34));     //left  leaf 1-8
  fill_solid( &leds[RLEAF_ST],RLEAF_SZ, CRGB(34,139,34));     //right leaf 9-16
  fill_solid( &leds[PETALS_ST],PETALS_SZ, CRGB(255,140,0));     //petals 17-40
  fill_solid( &leds[CENTER_ST],CENTER_SZ, CRGB(255,0,0));     //center 41-71 
}

void toggleLED()
{
  digitalWrite(pin_led,!digitalRead(pin_led));
  server.send_P(200,"text/html", webpage);
}
