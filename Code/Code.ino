/*
   This is the code for the project,
   All in One Home Automation project using ESP RainMaker made using ESP32 (Version 2)

   This code was written by Sachin Soni for techiesms YouTube channel

   This code is provided free for project purpose and fair use only.
   Do mail us if you want to use it commercially
   techiesms@gmail.com

   Copyrighted © by techiesms

   Watch out it's complete tutorial on our YouTube channel
   https://www.YouTube.com/techiesms
*/

#include "RMaker.h"
#include "WiFi.h"
#include "WiFiProv.h"
#include <IRremote.h>     // https://github.com/Arduino-IRremote/Arduino-IRremote (3.6.1)
#include <DHT.h>          // https://github.com/adafruit/DHT-sensor-library (1.4.3)
#include <SimpleTimer.h>  // https://github.com/kiryanenko/SimpleTimer (1.0.0)
#include <AceButton.h>    // https://github.com/bxparks/AceButton (1.9.2)
#include <Preferences.h>
Preferences pref;

// Relay State
bool toggleState_1 = LOW; //Define integer to remember the toggle state for relay 1
bool toggleState_2 = LOW; //Define integer to remember the toggle state for relay 2
bool toggleState_3 = LOW; //Define integer to remember the toggle state for relay 3
bool toggleState_4 = LOW; //Define integer to remember the toggle state for relay 4
int FanState      = 0 ;

using namespace ace_button;

// BLE Credentials
const char *service_name = "PROV_techiesms"; //name of node in BLE
const char *pop = "Techiesms123"; //password

// for Turning On and Off the Serial Monitor
#define DEBUG_SW 1

// By Default all the Relays will be in OFF State
#define DEFAULT_RELAY_STATE false

// define the Node Name
char nodeName[] = "All in One Home Automation";

// GPIO for Relay (Appliance Control)
static uint8_t relay1 = 15;
static uint8_t relay2 = 2;
static uint8_t relay3 = 4;
static uint8_t relay4 = 22;

// GPIO for Relay (Fan Speed Control)
static uint8_t Speed1 = 21;
static uint8_t Speed2 = 19;
static uint8_t Speed4 = 18;

// GPIO for switch
static uint8_t switch1 = 32;
static uint8_t switch2 = 35;
static uint8_t switch3 = 34;
static uint8_t switch4 = 39;

// GPIO for Fan Regulator Knob
static uint8_t fan_switch = 33;
static uint8_t s1 = 27;
static uint8_t s2 = 14;
static uint8_t s3 = 12;
static uint8_t s4 = 13;

static uint8_t gpio_reset = 0;   // Reset Pin
static uint8_t IR_SENS    = 17;  // IR Receiver Pin
static uint8_t DHTPIN     = 16;  // DHT11 Pin

// Flags for Fan Speed
bool speed1_flag = 1;
bool speed2_flag = 1;
bool speed3_flag = 1;
bool speed4_flag = 1;
bool speed0_flag = 1;

// Name of the device shown in the App
char Device1[] = "light1";
char Device2[] = "light2";
char Device3[] = "light3";
char Device4[] = "light4";

// IR Remote Code for Lights
#define IR_Relay1           0x1FE50AF
#define IR_Relay2           0x1FED827
#define IR_Relay3           0x1FEF807
#define IR_Relay4           0x1FE30CF
#define IR_Relay_All_Off    0x1FE48B7
#define IR_Relay_All_On     0x1FE7887

// IR Remote Code for Fan
#define IR_Speed_Up         0x1FE609F
#define IR_Speed_Dw         0x1FEA05F
#define IR_Fan_off          0x1FE10EF
#define IR_Fan_on           0x1FE906F

// Relay State
bool switch_state_ch1 = LOW;
bool switch_state_ch2 = LOW;
bool switch_state_ch3 = LOW;
bool switch_state_ch4 = LOW;

// Declaring & Setting Deafult value in all variables
float temperature_value = 0;
float humidity_value    = 0;
int Slider_Value        = 0;
int curr_speed          = 0;
bool fan_power          = 0;

static Switch my_switch1("light1", &relay1);
static Switch my_switch2("light2", &relay2);
static Switch my_switch3("light3", &relay3);
static Switch my_switch4("light4", &relay4);
static Fan my_fan("Fan");

static TemperatureSensor temperature("Temperature");
static TemperatureSensor humidity("Humidity");

ButtonConfig config1;
AceButton button1(&config1);
ButtonConfig config2;
AceButton button2(&config2);
ButtonConfig config3;
AceButton button3(&config3);
ButtonConfig config4;
AceButton button4(&config4);
ButtonConfig config5;
AceButton button5(&config5);

void handleEvent1(AceButton*, uint8_t, uint8_t);
void handleEvent2(AceButton*, uint8_t, uint8_t);
void handleEvent3(AceButton*, uint8_t, uint8_t);
void handleEvent4(AceButton*, uint8_t, uint8_t);
void handleEvent5(AceButton*, uint8_t, uint8_t);

DHT dht(DHTPIN, DHT11);
IRrecv irrecv(IR_SENS);
decode_results results;
SimpleTimer Timer;

void sysProvEvent(arduino_event_t *sys_event)
{
  switch (sys_event->event_id)
  {
    case ARDUINO_EVENT_PROV_START:
      if (DEBUG_SW)if (DEBUG_SW)Serial.printf("\nProvisioning Started with name \"%s\" and PoP \"%s\" on BLE\n", service_name, pop);
      printQR(service_name, pop, "ble");
      break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      if (DEBUG_SW)if (DEBUG_SW)Serial.print("\nConnected IP address : ");
      if (DEBUG_SW)if (DEBUG_SW)Serial.println(IPAddress(sys_event->event_info.got_ip.ip_info.ip.addr));
      break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      if (DEBUG_SW)if (DEBUG_SW)Serial.println("\nDisconnected. Connecting to the AP again... ");
      break;
    case ARDUINO_EVENT_PROV_CRED_RECV:
      if (DEBUG_SW)if (DEBUG_SW)Serial.println("\nReceived Wi-Fi credentials");
      if (DEBUG_SW)if (DEBUG_SW)Serial.print("\tSSID : ");
      if (DEBUG_SW)if (DEBUG_SW)Serial.println((const char *) sys_event->event_info.prov_cred_recv.ssid);
      if (DEBUG_SW)if (DEBUG_SW)Serial.print("\tPassword : ");
      if (DEBUG_SW)if (DEBUG_SW)Serial.println((char const *) sys_event->event_info.prov_cred_recv.password);
      break;
    case ARDUINO_EVENT_PROV_INIT:
      wifi_prov_mgr_disable_auto_stop(10000);
      break;
    case ARDUINO_EVENT_PROV_CRED_SUCCESS:
      if (DEBUG_SW)if (DEBUG_SW)Serial.println("Stopping Provisioning!!!");
      wifi_prov_mgr_stop_provisioning();
      break;
  }
}

void write_callback(Device *device, Param *param, const param_val_t val, void *priv_data, write_ctx_t *ctx)
{
  const char *device_name = device->getDeviceName();
  const char *param_name = param->getParamName();

  if (strcmp(device_name, "Fan") == 0)
  {
    if (strcmp(param_name, "Power") == 0)
    {
      if (DEBUG_SW)if (DEBUG_SW)Serial.printf("Received Fan power = %s for %s - %s\n", val.val.b ? "true" : "false", device_name, param_name);
      fan_power = val.val.b;
      if (fan_power) {
        if (curr_speed == 0)
        {
          speed_0();
        }
        if (curr_speed == 1)
        {
          speed_1();
        }
        if (curr_speed == 2)
        {
          speed_2();
        }
        if (curr_speed == 3)
        {
          speed_3();
        }
        if (curr_speed == 4)
        {
          speed_4();
        }
      }
      else
        speed_0();
      param->updateAndReport(val);
    }
    if (strcmp(param_name, "My_Speed") == 0)
    {
      if (DEBUG_SW)if (DEBUG_SW)Serial.printf("\nReceived value = %d for %s - %s\n", val.val.i, device_name, param_name);
      int Slider_Value = val.val.i;
      if (Slider_Value == 1)
      {
        speed_1();
      }
      if (Slider_Value == 2)
      {
        speed_2();
      }
      if (Slider_Value == 3)
      {
        speed_3();
      }
      if (Slider_Value == 4)
      {
        speed_4();
      }
      if (Slider_Value == 0)
      {
        speed_0();
      }
      param->updateAndReport(val);
    }
  }

  if (strcmp(device_name, Device1) == 0)
  {
    if (DEBUG_SW)if (DEBUG_SW)Serial.printf("Switch value_1 = %s\n", val.val.b ? "true" : "false");

    if (strcmp(param_name, "Power") == 0)
    {
      if (DEBUG_SW)if (DEBUG_SW)Serial.printf("Received value = %s for %s - %s\n", val.val.b ? "true" : "false", device_name, param_name);
      switch_state_ch1 = val.val.b;
      (switch_state_ch1 == false) ? digitalWrite(relay1, LOW) : digitalWrite(relay1, HIGH);
      pref.putBool("Relay1", switch_state_ch1);
      param->updateAndReport(val);
    }

  } else if (strcmp(device_name, Device2) == 0) {

    if (DEBUG_SW)if (DEBUG_SW)Serial.printf("SSwitch value_2 = %s\n", val.val.b ? "true" : "false");

    if (strcmp(param_name, "Power") == 0) {
      if (DEBUG_SW)if (DEBUG_SW)Serial.printf("Received value = %s for %s - %s\n", val.val.b ? "true" : "false", device_name, param_name);
      switch_state_ch2 = val.val.b;
      (switch_state_ch2 == false) ? digitalWrite(relay2, LOW) : digitalWrite(relay2, HIGH);
      pref.putBool("Relay2", switch_state_ch2);
      param->updateAndReport(val);
    }

  } else if (strcmp(device_name, Device3) == 0) {

    if (DEBUG_SW)if (DEBUG_SW)Serial.printf("Switch value_3 = %s\n", val.val.b ? "true" : "false");

    if (strcmp(param_name, "Power") == 0) {
      if (DEBUG_SW)if (DEBUG_SW)Serial.printf("Received value = %s for %s - %s\n", val.val.b ? "true" : "false", device_name, param_name);
      switch_state_ch3 = val.val.b;
      (switch_state_ch3 == false) ? digitalWrite(relay3, LOW) : digitalWrite(relay3, HIGH);
      pref.putBool("Relay3", switch_state_ch3);
      param->updateAndReport(val);
    }

  } else if (strcmp(device_name, Device4) == 0)
  {

    if (DEBUG_SW)if (DEBUG_SW)Serial.printf("Switch value_4 = %s\n", val.val.b ? "true" : "false");

    if (strcmp(param_name, "Power") == 0) {
      if (DEBUG_SW)if (DEBUG_SW)Serial.printf("Received value = %s for %s - %s\n", val.val.b ? "true" : "false", device_name, param_name);
      switch_state_ch4 = val.val.b;
      (switch_state_ch4 == false) ? digitalWrite(relay4, LOW) : digitalWrite(relay4, HIGH);
      pref.putBool("Relay4", switch_state_ch4);
      param->updateAndReport(val);
    }

  }
}

void readSensor() {

  float h = dht.readHumidity();
  float t = dht.readTemperature(); // or dht.readTemperature(true) for Fahrenheit

  if (isnan(h) || isnan(t)) {
    if (DEBUG_SW)Serial.println("Failed to read from DHT sensor!");
    return;
  }
  else
  {
    humidity_value = h;
    temperature_value = t;
    if (DEBUG_SW)Serial.print("Temperature - "); if (DEBUG_SW)Serial.println(t);
    if (DEBUG_SW)Serial.print("Humidity - "); if (DEBUG_SW)Serial.println(h);
  }
}

void sendSensor()
{
  readSensor();
  temperature.updateAndReportParam("Temperature", temperature_value);
  humidity.updateAndReportParam("Temperature", humidity_value);
}


// This function will recall the last state
void getRelayState()
{
  toggleState_1 = pref.getBool("Relay1", 0);
  Serial.print("Last State Relay1 - "); Serial.println(toggleState_1);
  digitalWrite(relay1, toggleState_1);
  my_switch1.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, toggleState_1);
  delay(200);
  toggleState_2 = pref.getBool("Relay2", 0);
  Serial.print("Last State Relay2- "); Serial.println(toggleState_2);
  digitalWrite(relay2, toggleState_2);
  my_switch2.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, toggleState_2);
  delay(200);
  toggleState_3 = pref.getBool("Relay3", 0);
  Serial.print("Last State Relay3- "); Serial.println(toggleState_3);
  digitalWrite(relay3, toggleState_3);
  my_switch3.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, toggleState_3);
  delay(200);
  toggleState_4 = pref.getBool("Relay4", 0);
  Serial.print("Last State Relay4- "); Serial.println(toggleState_4);
  digitalWrite(relay4, toggleState_4);
  my_switch3.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, toggleState_4);
  delay(200);
  FanState = pref.getInt("Fan", 0);
  Serial.print("Last State Fan- "); Serial.println(FanState);

  if (FanState == 0)
    speed_0();
  else if (FanState == 1)
    speed_1();
  else if (FanState == 2)
    speed_2();
  else if (FanState == 3)
    speed_3();
  else if (FanState == 4)
    speed_4();
  else
  {}
  delay(200);

}

void ir_remote() {
  if (DEBUG_SW)Serial.println("Inside IR REMOTE");
  if (irrecv.decode(&results)) {
    if (DEBUG_SW)Serial.println(results.value, HEX); //print the HEX code
    switch (results.value) {
      case IR_Relay1:
        switch_state_ch1 = !switch_state_ch1;
        digitalWrite(relay1, switch_state_ch1);
        if (DEBUG_SW)Serial.println("RELAY1 ON");
        my_switch1.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, switch_state_ch1);
        pref.putBool("Relay1", switch_state_ch1);
        delay(100);
        break;
      case IR_Relay2:
        switch_state_ch2 = !switch_state_ch2;
        digitalWrite(relay2, switch_state_ch2);
        my_switch2.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, switch_state_ch2);
        pref.putBool("Relay2", switch_state_ch2);
        delay(100);
        break;
      case IR_Relay3:
        switch_state_ch3 = !switch_state_ch3;
        digitalWrite(relay3, switch_state_ch3);
        my_switch3.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, switch_state_ch3);
        pref.putBool("Relay3", switch_state_ch3);
        delay(100);
        break;
      case IR_Relay4:
        switch_state_ch4 = !switch_state_ch4;
        digitalWrite(relay4, switch_state_ch4);
        my_switch4.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, switch_state_ch4);
        pref.putBool("Relay4", switch_state_ch4);
        delay(100);
        break;
      case IR_Relay_All_Off:
        All_Lights_Off();
        break;
      case IR_Relay_All_On:
        All_Lights_On();
        break;
      case IR_Fan_on:
        if (curr_speed == 0)
        {
          speed_0                                                                                                                                                                                                                                          ();
        }
        else if (curr_speed == 1)
        {
          speed_1();
        }
        else if (curr_speed == 2)
        {
          speed_2();
        }
        else if (curr_speed == 3)
        {
          speed_3();
        }
        else if (curr_speed == 4)
        {
          speed_4();
        }
        else
        {}
        break;
      case IR_Fan_off:
        speed_0();
        break;
      case IR_Speed_Up:
        if (curr_speed == 1)
        {
          speed_2();
        }
        else if (curr_speed == 2)
        {
          speed_3();
        }
        else if (curr_speed == 3)
        {
          speed_4();
        }
        else if (curr_speed == 4)
        {
          //Do nothing
        }
        else {}

        break;
      case IR_Speed_Dw:
        if (curr_speed == 1)
        {
          //Do nothing
        }
        if (curr_speed == 2)
        {
          speed_1();
        }
        if (curr_speed == 3)
        {
          speed_2();
        }
        if (curr_speed == 4)
        {
          speed_3();
        }
        else
        {}

        break;
      default : break;
    }
    irrecv.resume();
  }
}

void All_Lights_Off()
{
  switch_state_ch1 = 0;
  digitalWrite(relay1, LOW);
  my_switch1.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, switch_state_ch1);
  pref.putBool("Relay1", switch_state_ch1);

  switch_state_ch2 = 0;
  digitalWrite(relay2, LOW);
  my_switch2.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, switch_state_ch2);
  pref.putBool("Relay2", switch_state_ch2);

  switch_state_ch3 = 0;
  digitalWrite(relay3, LOW);
  my_switch3.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, switch_state_ch3);
  pref.putBool("Relay3", switch_state_ch3);

  switch_state_ch4 = 0;
  digitalWrite(relay4, LOW);
  my_switch4.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, switch_state_ch4);
  pref.putBool("Relay4", switch_state_ch4);

}

void All_Lights_On()
{
  switch_state_ch1 = 1;
  digitalWrite(relay1, HIGH);
  my_switch1.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, switch_state_ch1);
  pref.putBool("Relay1", switch_state_ch1);

  switch_state_ch2 = 1;
  digitalWrite(relay2, HIGH);
  my_switch2.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, switch_state_ch2);
  pref.putBool("Relay2", switch_state_ch2);

  switch_state_ch3 = 1;
  digitalWrite(relay3, HIGH);
  my_switch3.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, switch_state_ch3);
  pref.putBool("Relay3", switch_state_ch3);

  switch_state_ch4 = 1;
  digitalWrite(relay4, HIGH);
  my_switch4.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, switch_state_ch4);
  pref.putBool("Relay4", switch_state_ch4);

}

void setup()
{

  if (DEBUG_SW)Serial.begin(115200);
  pref.begin("Relay_State", false);
  // Set the Relays GPIOs as output mode
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(relay3, OUTPUT);
  pinMode(relay4, OUTPUT);

  //Turning All Relays Off by default
  digitalWrite(relay1, LOW);
  digitalWrite(relay2, LOW);
  digitalWrite(relay3, LOW);
  digitalWrite(relay4, LOW);

  // Set the Relays GPIOs as output mode
  pinMode(Speed1, OUTPUT);
  pinMode(Speed2, OUTPUT);
  pinMode(Speed4, OUTPUT);

  //Turning All Relays Off by default
  digitalWrite(Speed1, LOW);
  digitalWrite(Speed2, LOW);
  digitalWrite(Speed4, LOW);

  // Configure the input GPIOs
  pinMode(switch1, INPUT_PULLUP);
  pinMode(switch2, INPUT_PULLUP);
  pinMode(switch3, INPUT_PULLUP);
  pinMode(switch4, INPUT_PULLUP);

  pinMode(fan_switch, INPUT_PULLUP);
  pinMode(s1, INPUT_PULLUP);
  pinMode(s2, INPUT_PULLUP);
  pinMode(s3, INPUT_PULLUP);
  pinMode(s4, INPUT_PULLUP);

  pinMode(gpio_reset, INPUT);

  config1.setEventHandler(button1Handler);
  config2.setEventHandler(button2Handler);
  config3.setEventHandler(button3Handler);
  config4.setEventHandler(button4Handler);
  config5.setEventHandler(button5Handler);

  button1.init(switch1);
  button2.init(switch2);
  button3.init(switch3);
  button4.init(switch4);
  button5.init(fan_switch);

  irrecv.enableIRIn(); // Enabling IR sensor
  dht.begin();    // Enabling DHT sensor

  Node my_node;
  my_node = RMaker.initNode(nodeName);

  //Custom Fan device
  my_fan.addCb(write_callback);
  Param level_param("My_Speed", "custom.param.level", value(0), PROP_FLAG_READ | PROP_FLAG_WRITE);
  level_param.addBounds(value(0), value(4), value(1));
  level_param.addUIType(ESP_RMAKER_UI_SLIDER);
  my_fan.addParam(level_param);
  my_node.addDevice(my_fan);
  my_fan.updateAndReportParam("My_Speed", 0);
  my_fan.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, fan_power);
  delay(500);

  my_switch1.addCb(write_callback);
  my_node.addDevice(my_switch1);
  my_switch1.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, switch_state_ch1);
  delay(500);

  my_switch2.addCb(write_callback);
  my_node.addDevice(my_switch2);
  my_switch2.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, switch_state_ch2);
  delay(500);

  my_switch3.addCb(write_callback);
  my_node.addDevice(my_switch3);
  my_switch3.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, switch_state_ch3);
  delay(500);

  my_switch4.addCb(write_callback);
  my_node.addDevice(my_switch4);
  my_switch4.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, switch_state_ch4);
  delay(500);


  my_node.addDevice(temperature);
  my_node.addDevice(humidity);


  Timer.setInterval(2000);

  //This is optional
  RMaker.enableOTA(OTA_USING_PARAMS);
  //If you want to enable scheduling, set time zone for your region using setTimeZone().
  //The list of available values are provided here https://rainmaker.espressif.com/docs/time-service.html
  // RMaker.setTimeZone("Asia/Shanghai");
  // Alternatively, enable the Timezone service and let the phone apps set the appropriate timezone
  RMaker.enableTZService();
  RMaker.enableSchedule();

  if (DEBUG_SW)Serial.printf("\nStarting ESP-RainMaker\n");
  RMaker.start();

  WiFi.onEvent(sysProvEvent);
#if CONFIG_IDF_TARGET_ESP32
  WiFiProv.beginProvision(WIFI_PROV_SCHEME_BLE, WIFI_PROV_SCHEME_HANDLER_FREE_BTDM, WIFI_PROV_SECURITY_1, pop, service_name);
#else
  WiFiProv.beginProvision(WIFI_PROV_SCHEME_SOFTAP, WIFI_PROV_SCHEME_HANDLER_NONE, WIFI_PROV_SECURITY_1, pop, service_name);
#endif


  getRelayState(); // Get the last state of Relays

  my_switch1.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, false);
  my_switch2.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, false);
  my_switch3.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, false);
  my_switch4.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, false);


}

void loop()
{
  button1.check();
  button2.check();
  button3.check();
  button4.check();
  button5.check();
  fan();

  // Read GPIO0 (external button to gpio_reset device
  if (digitalRead(gpio_reset) == LOW) {
    //Push button pressed
    if (DEBUG_SW)Serial.printf("reset Button Pressed!\n");
    // Key debounce handling
    delay(100);
    int startTime = millis();
    while (digitalRead(gpio_reset) == LOW) delay(50);
    int endTime = millis();

    if ((endTime - startTime) > 5000) {
      // If key pressed for more than 5secs, reset all
      if (DEBUG_SW)Serial.printf("reset to factory.\n");
      RMakerFactoryReset(2);
    }
  }
  delay(100);

  if (WiFi.status() != WL_CONNECTED)
  {
    if (DEBUG_SW)Serial.println("WiFi Not Connected");

  }
  else
  {
    if (DEBUG_SW)Serial.println("WiFi Connected");
    if (Timer.isReady()) {
      if (DEBUG_SW)Serial.println("Sending Sensor Data");
      sendSensor();
      Timer.reset();      // Reset a second timer
    }
  }

  ir_remote(); //IR remote Control
}

//functions for defineing manual switch

void button1Handler(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  if (DEBUG_SW)Serial.println("EVENT1");
  //EspalexaDevice* d1 = espalexa.getDevice(0);
  switch (eventType) {
    case AceButton::kEventPressed:
      if (DEBUG_SW)Serial.println("kEventPressed");
      switch_state_ch1 = true;
      my_switch1.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, switch_state_ch1);
      digitalWrite(relay1, HIGH);
      pref.putBool("Relay1", switch_state_ch1);
      break;
    case AceButton::kEventReleased:
      if (DEBUG_SW)Serial.println("kEventReleased");
      switch_state_ch1 = false;
      my_switch1.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, switch_state_ch1);
      digitalWrite(relay1, LOW);
      pref.putBool("Relay1", switch_state_ch1);
      break;
  }
}
void button2Handler(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  if (DEBUG_SW)Serial.println("EVENT2");
  //EspalexaDevice* d2 = espalexa.getDevice(1);
  switch (eventType) {
    case AceButton::kEventPressed:
      if (DEBUG_SW)Serial.println("kEventPressed");
      switch_state_ch2 = true;
      my_switch2.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, switch_state_ch2);
      digitalWrite(relay2, HIGH);
      pref.putBool("Relay2", switch_state_ch2);
      break;
    case AceButton::kEventReleased:
      if (DEBUG_SW)Serial.println("kEventReleased");
      switch_state_ch2 = false;
      my_switch2.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, switch_state_ch2);
      digitalWrite(relay2, LOW);
      pref.putBool("Relay2", switch_state_ch2);
      break;
  }
}
void button3Handler(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  if (DEBUG_SW)Serial.println("EVENT3");
  //EspalexaDevice* d3 = espalexa.getDevice(2);
  switch (eventType) {
    case AceButton::kEventPressed:
      if (DEBUG_SW)Serial.println("kEventPressed");
      switch_state_ch3 = true;
      my_switch3.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, switch_state_ch3);
      digitalWrite(relay3, HIGH);
      pref.putBool("Relay3", switch_state_ch3);
      break;
    case AceButton::kEventReleased:
      if (DEBUG_SW)Serial.println("kEventReleased");
      switch_state_ch3 = false;
      my_switch3.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, switch_state_ch3);
      digitalWrite(relay3, LOW);
      pref.putBool("Relay3", switch_state_ch3);
      break;
  }
}
void button4Handler(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  if (DEBUG_SW)Serial.println("EVENT4");
  //EspalexaDevice* d4 = espalexa.getDevice(3);
  switch (eventType) {
    case AceButton::kEventPressed:
      if (DEBUG_SW)Serial.println("kEventPressed");
      switch_state_ch4 = true;
      my_switch4.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, switch_state_ch4);
      digitalWrite(relay4, HIGH);
      pref.putBool("Relay4", switch_state_ch4);
      break;
    case AceButton::kEventReleased:
      if (DEBUG_SW)Serial.println("kEventReleased");
      switch_state_ch4 = false;
      my_switch4.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, switch_state_ch4);
      digitalWrite(relay4, LOW);
      pref.putBool("Relay4", switch_state_ch4);
      break;
  }
}

void button5Handler(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  if (DEBUG_SW)Serial.println("EVENT5");
  switch (eventType) {
    case AceButton::kEventPressed:
      if (DEBUG_SW)Serial.println("kEventPressed");
      if (curr_speed == 0)
      {
        speed_0();
      }
      if (curr_speed == 1)
      {
        speed_1();
      }
      if (curr_speed == 2)
      {
        speed_2();
      }
      if (curr_speed == 3)
      {
        speed_3();
      }
      if (curr_speed == 4)
      {
        speed_4();
      }
      break;
    case AceButton::kEventReleased:
      if (DEBUG_SW)Serial.println("kEventReleased");
      digitalWrite(Speed1, LOW);
      digitalWrite(Speed2, LOW);
      digitalWrite(Speed4, LOW);
      fan_power = 0;
      my_fan.updateAndReportParam("My_Speed", 0);
      my_fan.updateAndReportParam("Power", fan_power);
      break;
  }
}


//functions for defineing of speeds

void speed_0()
{
  //All Relays Off - Fan at speed 0
  if (DEBUG_SW)Serial.println("SPEED 0");
  fan_power = 0;
  my_fan.updateAndReportParam("My_Speed", 0);
  my_fan.updateAndReportParam("Power", fan_power);
  digitalWrite(Speed1, LOW);
  digitalWrite(Speed2, LOW);
  digitalWrite(Speed4, LOW);
  pref.putInt("Fan", curr_speed);

}

void speed_1()
{
  //Speed1 Relay On - Fan at speed 1
  if (DEBUG_SW)Serial.println("SPEED 1");
  curr_speed = 1;
  fan_power = 1;
  my_fan.updateAndReportParam("My_Speed", 1);
  my_fan.updateAndReportParam("Power", fan_power);
  digitalWrite(Speed1, LOW);
  digitalWrite(Speed2, LOW);
  digitalWrite(Speed4, LOW);
  delay(1000);
  digitalWrite(Speed1, HIGH);
  pref.putInt("Fan", curr_speed);
}

void speed_2()
{
  //Speed2 Relay On - Fan at speed 2
  if (DEBUG_SW)Serial.println("SPEED 2");
  curr_speed = 2;
  fan_power = 1;
  my_fan.updateAndReportParam("My_Speed", 2);
  my_fan.updateAndReportParam("Power", fan_power);
  digitalWrite(Speed1, LOW);
  digitalWrite(Speed2, LOW);
  digitalWrite(Speed4, LOW);
  delay(1000);
  digitalWrite(Speed2, HIGH);
  pref.putInt("Fan", curr_speed);
}

void speed_3()
{
  //Speed1 & Speed2 Relays On - Fan at speed 3
  if (DEBUG_SW)Serial.println("SPEED 3");
  curr_speed = 3;
  fan_power = 1;
  my_fan.updateAndReportParam("My_Speed", 3);
  my_fan.updateAndReportParam("Power", fan_power);
  digitalWrite(Speed1, LOW);
  digitalWrite(Speed2, LOW);
  digitalWrite(Speed4, LOW);
  delay(1000);
  digitalWrite(Speed1, HIGH);
  digitalWrite(Speed2, HIGH);
  pref.putInt("Fan", curr_speed);

}

void speed_4()
{
  //Speed4 Relay On - Fan at speed 4
  if (DEBUG_SW)Serial.println("SPEED 4");
  curr_speed = 4;
  fan_power = 1;
  my_fan.updateAndReportParam("My_Speed", 4);
  my_fan.updateAndReportParam("Power", fan_power);
  digitalWrite(Speed1, LOW);
  digitalWrite(Speed2, LOW);
  digitalWrite(Speed4, LOW);
  delay(1000);
  digitalWrite(Speed4, HIGH);
  pref.putInt("Fan", curr_speed);
}


// function for controling fan using regulator

void fan()
{
  if ( digitalRead(s1) == LOW && speed1_flag == 1)
  {
    speed_1();
    speed1_flag = 0;
    speed2_flag = 1;
    speed3_flag = 1;
    speed4_flag = 1;
    speed0_flag = 1;


  }
  if (digitalRead(s2) == LOW && digitalRead(s3) == HIGH && speed2_flag == 1)
  {
    speed_2();
    speed1_flag = 1;
    speed2_flag = 0;
    speed3_flag = 1;
    speed4_flag = 1;
    speed0_flag = 1;

  }
  if (digitalRead(s2) == LOW && digitalRead(s3) == LOW && speed3_flag == 1)
  {
    speed_3();
    speed1_flag = 1;
    speed2_flag = 1;
    speed3_flag = 0;
    speed4_flag = 1;
    speed0_flag = 1;
  }
  if (digitalRead(s4) == LOW  && speed4_flag == 1)
  {
    speed_4();
    speed1_flag = 1;
    speed2_flag = 1;
    speed3_flag = 1;
    speed4_flag = 0;
    speed0_flag = 1;
  }
  if (digitalRead(s1) == HIGH && digitalRead(s2) == HIGH && digitalRead(s3) == HIGH && digitalRead(s4) == HIGH  && speed0_flag == 1)
  {
    speed_0();
    speed1_flag = 1;
    speed2_flag = 1;
    speed3_flag = 1;
    speed4_flag = 1;
    speed0_flag = 0;
  }

}
