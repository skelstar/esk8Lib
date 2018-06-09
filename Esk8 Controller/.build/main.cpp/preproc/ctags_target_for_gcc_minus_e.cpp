# 1 "k:\\GitHubs\\esk8Lib\\Esk8 Controller\\src\\main.cpp"
# 1 "k:\\GitHubs\\esk8Lib\\Esk8 Controller\\src\\main.cpp"
# 2 "k:\\GitHubs\\esk8Lib\\Esk8 Controller\\src\\main.cpp" 2
# 3 "k:\\GitHubs\\esk8Lib\\Esk8 Controller\\src\\main.cpp" 2
# 4 "k:\\GitHubs\\esk8Lib\\Esk8 Controller\\src\\main.cpp" 2
# 5 "k:\\GitHubs\\esk8Lib\\Esk8 Controller\\src\\main.cpp" 2
// #include <Adafruit_NeoPixel.h>
// #include <U8g2lib.h>

# 9 "k:\\GitHubs\\esk8Lib\\Esk8 Controller\\src\\main.cpp" 2

# 11 "k:\\GitHubs\\esk8Lib\\Esk8 Controller\\src\\main.cpp" 2
# 12 "k:\\GitHubs\\esk8Lib\\Esk8 Controller\\src\\main.cpp" 2
# 13 "k:\\GitHubs\\esk8Lib\\Esk8 Controller\\src\\main.cpp" 2
# 14 "k:\\GitHubs\\esk8Lib\\Esk8 Controller\\src\\main.cpp" 2
// #include <WiiChuck.h>

//--------------------------------------------------------------------------------

const char compile_date[] = "Jun  4 2018" " " "05:47:11";

int getThrottleValue(int raw);
void setupEncoder();
void setPixels(CRGB c) ;
void setCommsState(int newState);
void serviceCommsState();
void servicePixels();

// https://sandhansblog.wordpress.com/2017/04/16/interfacing-displaying-a-custom-graphic-on-an-0-96-i2c-oled/
static const unsigned char wifiicon14x10 [] __attribute__((__progmem__)) = {
   0xf0, 0x03, 0xfc, 0x0f, 0x1e, 0x1c, 0xc7, 0x39, 0xf3, 0x33, 0x38, 0x06, 0x1c, 0x0c, 0xc8, 0x04, 0xe0, 0x01, 0xc0, 0x00
};

//--------------------------------------------------------------------------------
# 46 "k:\\GitHubs\\esk8Lib\\Esk8 Controller\\src\\main.cpp"
// const char boardSetup[] = "DEV Board";
// #define SPI_CE				22	// white/purple
// #define SPI_CS				5  // green
const char boardSetup[] = "WEMOS TTGO Board";
# 60 "k:\\GitHubs\\esk8Lib\\Esk8 Controller\\src\\main.cpp"
//--------------------------------------------------------------




//int role = ROLE_MASTER;
int role = 0;
bool radioNumber = 1;

int sendIntervalMs = 200;
bool updateOled = false;

RF24 radio(33 /* white/purple*/, 26 /* green*/); // ce pin, cs pin
//--------------------------------------------------------------

// #define     OLED_CONTRAST_HIGH	100        // 256 highest
// #define     OLED_CONTRAST_LOW	20
// U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, OLED_SCL, OLED_SDA, U8X8_PIN_NONE);

//--------------------------------------------------------------------------------
# 90 "k:\\GitHubs\\esk8Lib\\Esk8 Controller\\src\\main.cpp"
debugHelper debug;

Rotary rotary = Rotary(16, 17);

esk8Lib esk8;

//--------------------------------------------------------------------------------



CRGB leds[8];
//Adafruit_NeoPixel leds = Adafruit_NeoPixel(NUM_PIXELS, PIXEL_PIN, NEO_RGB + NEO_KHZ800);




// uint32_t COLOUR_OFF = leds.Color(0, 0, 0);
// uint32_t COLOUR_RED = leds.Color(255, 0, 0);
// uint32_t COLOUR_GREEN = leds.Color(0, 255, 0);
// uint32_t COLOUR_BLUE = leds.Color(0, 0, 255);
// uint32_t COLOUR_PINK = leds.Color(128, 0, 100);
// uint32_t COLOUR_ACCELERATING = COLOUR_BLUE;
// uint32_t COLOUR_DECELERATING = COLOUR_PINK;
// uint32_t COLOUR_THROTTLE_IDLE = COLOUR_GREEN;

CRGB COLOUR_OFF = CRGB::Black;
CRGB COLOUR_RED = CRGB::Red;
CRGB COLOUR_GREEN = CRGB::Green;
CRGB COLOUR_BLUE = CRGB::Blue;
CRGB COLOUR_PINK = CRGB::Pink;

CRGB COLOUR_ACCELERATING = CRGB::Navy;
CRGB COLOUR_DECELERATING = CRGB::Crimson;
CRGB COLOUR_THROTTLE_IDLE = CRGB::Green;

CRGB COLOUR_WHITE = CRGB::White;

//--------------------------------------------------------------------------------





void zeroThrottleReadyToSend();

void listener_deadmanSwitch( int eventCode, int eventPin, int eventParam );
myPushButton deadmanSwitch(25, true, 0x1, listener_deadmanSwitch);
void listener_deadmanSwitch( int eventCode, int eventPin, int eventParam ) {

 switch (eventCode) {

  case deadmanSwitch.EV_BUTTON_PRESSED:
   if (esk8.controllerPacket.throttle > 127) {
   }
   debug.print(1 << 3, "EV_BUTTON_PRESSED (DEADMAN) \n");
   break;

  case deadmanSwitch.EV_RELEASED:
   if (esk8.controllerPacket.throttle > 127) {
     zeroThrottleReadyToSend();
     //setPixels(COLOUR_DEADMAN_OFF, 0);
   }
   debug.print(1 << 3, "EV_BUTTON_RELEASED (DEADMAN) \n");
   break;

  case deadmanSwitch.EV_HELD_SECONDS:
   //Serial.printf("EV_BUTTON_HELD (DEADMAN): %d \n", eventParam);
   break;
 }
}

void listener_dialButton( int eventCode, int eventPin, int eventParam );
myPushButton dialButton(34 /* 36 didn't work*/, true, 0x1, listener_dialButton);
void listener_dialButton( int eventCode, int eventPin, int eventParam ) {

 switch (eventCode) {

  case dialButton.EV_BUTTON_PRESSED:
   break;

  case dialButton.EV_RELEASED:
            zeroThrottleReadyToSend();
   break;

  case dialButton.EV_HELD_SECONDS:
   break;
 }
}

//--------------------------------------------------------------

// lower number = more coarse





//--------------------------------------------------------------

int encoderCounter = 0;
bool encoderChanged = false;
volatile bool statusChanged = true;
volatile bool packetReadyToBeSent = false;
volatile long lastPacketFromMaster = 0;
# 204 "k:\\GitHubs\\esk8Lib\\Esk8 Controller\\src\\main.cpp"
//--------------------------------------------------------------------------------

Scheduler runner;

void tFastFlash_callback();
Task tFastFlash(500, 1, &tFastFlash_callback);
void tFastFlash_callback() {
    Serial.printf("tFastFlash_callback \n");
    setPixels(COLOUR_OFF);
    tFastFlash.disable();
}

void fastFlashLed() {
    tFastFlash.setIterations(1);
    tFastFlash.enable();
}

bool tFlashLeds_onEnable();
void tFlashLedsOn_callback();
void tFlashLedsOff_callback();

// CRGB tFlashLedsColour = COLOUR_RED;
Task tFlashLeds(500, (-1), &tFlashLedsOff_callback);

bool tFlashLeds_onEnable() {
 // setPixels(tFlashLedsColour, 0);
 Serial.println("tFlashLeds_onEnable");
 tFlashLeds.enable();
}
void tFlashLedsOn_callback() {
 tFlashLeds.setCallback(&tFlashLedsOff_callback);
 // setPixels(tFlashLedsColour, 0);
 Serial.println("tFlashLedsOn_callback");
}
void tFlashLedsOff_callback() {
 tFlashLeds.setCallback(&tFlashLedsOn_callback);
 // setPixels(COLOUR_OFF, 0);
 Serial.println("tFlashLedsOff_callback");
}

void tSendControllerValues_callback() {
 if (esk8.sendThenReadPacket() == true) {
  lastPacketFromMaster = millis();
 }
 updateOled = true;
 debug.print(1 << 4, "tSendControllerValues_callback(): batteryVoltage:%.1f \n", esk8.boardPacket.batteryVoltage);
}

Task tSendControllerValues(200, (-1), &tSendControllerValues_callback);

//------------------------------------------------------------------------------

void encoderInterruptHandler() {
 unsigned char result = rotary.process();

 // debug.print(DEBUG, "Encoder event \n");

 bool canAccelerate = deadmanSwitch.isPressed();

 if (result == 0x10 && (canAccelerate || encoderCounter < 0)) {
  if (encoderCounter < 10 /* acceleration (ie 15 divides 127-255 into 15)*/) {
   encoderCounter++;
   int throttle = getThrottleValue(encoderCounter);
   esk8.controllerPacket.throttle = throttle;
   packetReadyToBeSent = true;
   debug.print(1 << 5, "encoderCounter: %d, throttle: %d \n", encoderCounter, throttle);
   statusChanged = true;

            setPixels(COLOUR_ACCELERATING);
            fastFlashLed();
  }
 }
 else if (result == 0x20) {
  if (encoderCounter > -18 /* decceleration (ie -20 divides 0-127 into 20)*/) {
   encoderCounter--;
   int throttle = getThrottleValue(encoderCounter);
   esk8.controllerPacket.throttle = throttle;
   packetReadyToBeSent = true;
   debug.print(1 << 5, "encoderCounter: %d, throttle: %d \n", encoderCounter, throttle);
   statusChanged = true;

            setPixels(COLOUR_DECELERATING);
            fastFlashLed();
  }
 }
}

//--------------------------------------------------------------

// Accessory chuk;

//--------------------------------------------------------------
// Prototypes
void sendMessage();

bool calc_delay = false;

//--------------------------------------------------------------

volatile uint32_t otherNode;
volatile long lastRxMillis = 0;


//--------------------------------------------------------------
void sendMessage() {
 if (esk8.sendPacketToBoard()) {
  lastPacketFromMaster = millis();
  updateOled = true;
 }
 debug.print(1 << 4, "Sending message: throttle:%d \n", esk8.controllerPacket.throttle);
}
//--------------------------------------------------------------

void powerupEvent(int state);

int powerButtonIsPressedFunc() {
 return deadmanSwitch.isPressed() &&
   dialButton.isPressed();
}

//myPowerButtonManager powerButton(ENCODER_BUTTON_PIN, HIGH, 3000, 3000, powerupEvent);
myPowerButtonManager powerButton(25, 0x1, 3000, 3000, powerupEvent, powerButtonIsPressedFunc);

void powerupEvent(int state) {

 switch (state) {
  case powerButton.TN_TO_POWERING_UP:
   setPixels(COLOUR_GREEN);
   statusChanged = true;
   break;
  case powerButton.TN_TO_POWERED_UP_WAIT_RELEASE:
   setPixels(COLOUR_OFF);
   // skip this and go straight to RUNNING
   statusChanged = true;
   powerButton.setState(powerButton.TN_TO_RUNNING);
   break;
  case powerButton.TN_TO_RUNNING:
   setPixels(COLOUR_OFF);
   statusChanged = true;
   break;
  case powerButton.TN_TO_POWERING_DOWN:
   tFlashLeds.disable(); // in case comms is offline
   zeroThrottleReadyToSend();
   setPixels(COLOUR_RED);
   statusChanged = true;
   break;
  case powerButton.TN_TO_POWERING_DOWN_WAIT_RELEASE: {
       long pixelTime = millis();
       powerButton.setState(powerButton.TN_TO_POWER_OFF);
       statusChanged = true;
            }
   break;
  case powerButton.TN_TO_POWER_OFF:
    setPixels(COLOUR_OFF);
   statusChanged = true;
   delay(100);
   esp_deep_sleep_start();
   Serial.println("This will never be printed");
   break;
 }
}

//--------------------------------------------------------------------------------





volatile int commsState = -1;

//--------------------------------------------------------------
void setup() {

 Serial.begin(9600);

 radio.begin();

 debug.init();

 debug.addOption(1 << 3, "DEBUG");
 debug.addOption(1 << 0, "STARTUP");
 debug.addOption(1 << 4, "COMMUNICATION");
 debug.addOption(1 << 2, "ERROR");
 debug.addOption(1 << 5, "THROTTLE_DEBUG");
 debug.addOption(1 << 6, "REGISTER");

 debug.setFilter(1 << 5 | 1 << 6); //DEBUG | STARTUP | COMMUNICATION | ERROR);

 debug.print(1 << 0, "%s \n", compile_date);
 debug.print(1 << 0, "NOTE: %s\n", boardSetup);

 esk8.begin(&radio, role, radioNumber, &debug);

 //FastLED.addLeds<TM1804, PIXEL_PIN, RGB>(leds, NUM_PIXELS);

 // leds.begin();

 FastLED.addLeds<NEOPIXEL, 5>(leds, 8);
 FastLED.show();

 runner.startNow();
    runner.addTask(tFastFlash);
 runner.addTask(tFlashLeds);
 runner.addTask(tSendControllerValues);

 tSendControllerValues.setInterval(esk8.getSendInterval());
 tSendControllerValues.enable();

 powerButton.begin(1 << 3);

 while (powerButton.isRunning() == false) {
  powerButton.serviceButton();
 }

 // encoder
 setupEncoder();
}

void loop() {

 dialButton.serviceEvents();

 deadmanSwitch.serviceEvents();

 powerButton.serviceButton();

 runner.execute();

 serviceCommsState();

 servicePixels();

 delay(10);
}
//--------------------------------------------------------------
void servicePixels() {

 uint8_t powerState = powerButton.getState();
 debug.print(1 << 3, "powerState: %d \n", powerState);

 if (powerState == powerButton.TN_TO_POWERING_UP) {
  setPixels(COLOUR_GREEN);
 }
 else if (powerState == powerButton.TN_TO_POWERED_UP_WAIT_RELEASE) {
  setPixels(COLOUR_OFF);
 }
 else if (powerState == powerButton.TN_TO_RUNNING) {
  setPixels(COLOUR_RED);
 }
 else if (powerState == powerButton.TN_TO_POWERING_DOWN) {
  setPixels(COLOUR_RED);
 }
 else if (powerState == powerButton.TN_TO_POWERING_DOWN_WAIT_RELEASE) {
  setPixels(COLOUR_OFF);
 }
 else if (powerState == powerButton.TN_TO_POWER_OFF) {
  setPixels(COLOUR_OFF);
 }
 else {
 }
}
//----------------------------------------------------------------
void setupEncoder() {

 pinMode(16, 0x2);
 pinMode(17, 0x2);

 attachInterrupt(((16) == 2 ? 0 : ((16) == 3 ? 1 : -1)), encoderInterruptHandler, 1);
 attachInterrupt(((17) == 2 ? 0 : ((17) == 3 ? 1 : -1)), encoderInterruptHandler, 1);
}
//--------------------------------------------------------------------------------
void serviceCommsState() {

 bool online = esk8.boardOnline();

 if (commsState == 1 && online == false) {
  setCommsState(0);
 }
 else if (commsState == 0 && online) {
  setCommsState(1);
 }
 else if (commsState == -1) {
  setCommsState(online == false ? 0 : 1);
 }
}
//--------------------------------------------------------------------------------
void setCommsState(int newState) {
 if (newState == 0) {
  commsState = 0;
  statusChanged = true;
  debug.print(1 << 3, "Setting commsState: COMMS_OFFLINE\n");
  // start leds flashing
  //tFlashLedsColour = COLOUR_RED;
  //setPixels(tFlashLedsColour, 0);
  //tFlashLeds.enable();
 }
 else if (newState == 1) {
  debug.print(1 << 3, "Setting commsState: COMMS_ONLINE\n");
  commsState = 1;
  statusChanged = true;
  // stop leds flashing
  // tFlashLeds.disable();
  // setPixels(COLOUR_OFF, 0);
 }
}
//--------------------------------------------------------------------------------
void setStatusRegisterPixels() {

 if (statusChanged == false) {
  return;
 }

 debug.print(1 << 6, "setStatusRegisterPixels()");

 if (commsState == 0) {
  setPixels(COLOUR_RED);
 }
 else if (encoderCounter > 0) {
  setPixels(COLOUR_ACCELERATING);
 }
 else if (encoderCounter < 0) {
  setPixels(COLOUR_DECELERATING);
 }
 else if (encoderCounter == 0) {
  setPixels(COLOUR_GREEN);
 }
 else {
  setPixels(COLOUR_OFF);
 }

 statusChanged = false;
}
//--------------------------------------------------------------
void setPixels(CRGB c) {
 for (uint16_t i=0; i<8; i++) {
  leds[i] = c;
  // leds[i] /= 10;
 }
 FastLED.show();
}
//--------------------------------------------------------------------------------
int getThrottleValue(int raw) {
 int mappedThrottle = 0;

 if (raw >= 0) {
  mappedThrottle = map(raw, 0, 10 /* acceleration (ie 15 divides 127-255 into 15)*/, 127, 255);
 }
 else {
  mappedThrottle = map(raw, -18 /* decceleration (ie -20 divides 0-127 into 20)*/, 0, 0, 127);
 }

 return mappedThrottle;
}
//--------------------------------------------------------------------------------
void zeroThrottleReadyToSend() {
 encoderCounter = 0;
 esk8.controllerPacket.throttle = 127;
    debug.print(1 << 5, "encoderCounter: %d, throttle: %d [ZERO] \n", encoderCounter, esk8.controllerPacket.throttle);
 packetReadyToBeSent = true;
}
//--------------------------------------------------------------


// void drawOnline(bool online) {
// 	if (online) {
// 		u8g2.drawXBMP(0, 0, ONLINE_SYMBOL_WIDTH, 10, wifiicon14x10);
// 	}
// 	else {
// 		u8g2.setDrawColor(0);
// 		u8g2.drawBox(0, 0, ONLINE_SYMBOL_WIDTH, 20);
// 		u8g2.setDrawColor(1);
// 	}
// 	// u8g2.sendBuffer();
// }
// //--------------------------------------------------------------------------------
// void drawBatteryVoltage(float voltage) {
// 	u8g2.setCursor(ONLINE_SYMBOL_WIDTH+5, 10);
// 	u8g2.setFont(u8g2_font_helvR08_tf);
// 	u8g2.print(voltage, 1);
// 	u8g2.print("v");
// }
// //--------------------------------------------------------------------------------
// void drawThrottle(int throttleValue, bool show) {

// 	if (show) {
// 		char val[4];
// 		itoa(throttleValue, val, 10);
// 		int strWidth = u8g2.getStrWidth(val);

// 	  	u8g2.setFont(u8g2_font_courB18_tf);
// 		u8g2.setCursor(0, 32);
// 		u8g2.print(val);
// 		//u8g2.drawStr(128-strWidth, 32, val);
// 	}
// }
// //--------------------------------------------------------------------------------
// void drawMessage(char* msg) {
// 	u8g2.clearBuffer();
//   	u8g2.setFont(u8g2_font_courB18_tf);
// 	int strWidth = u8g2.getStrWidth(msg);
// 	u8g2.setCursor(64-(strWidth/2), 16+9);
// 	u8g2.print(msg);
// 	u8g2.sendBuffer();
// }
// //--------------------------------------------------------------------------------
