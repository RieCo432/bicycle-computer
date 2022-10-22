#include <Arduino_GFX_Library.h>
#include <nrf52840.h>
#include <nrfx_saadc.h>
#include <AnalogIn.h>
#include <pinDefinitions.h>


#define GFX_BL 2

static uint32_t screenWidth = 240;
static uint32_t screenHeight = 240;

const float hor_font_blocks = 5;
const float ver_font_blocks = 7;

const uint8_t speed_limit_kmh = 70;
const char speed_unit[] = "km/h";
const char distance_unit[] = "km";
const float arc_span = 150.0;
const float arc_start = - 180.0 + (180.0 - arc_span) / 2.0;
const float battery_level_update_period = 5000; // in ms

const String current_speed_value_format = "%d";

const char current_distance_format[] = "%.1f km";
const char current_duration_format[] = "%d:%02d:%02d";
const char current_avg_speed_format[] = "%4.1f";
const char current_max_speed_format[] = "%4.1f";

const char alltime_distance_format[] = "%05.0f km";
const char alltime_duration_format[] = "%05d:%02d";
const char alltime_avg_speed_format[] = "%4.1f";
const char alltime_max_speed_format[] = "%4.1f";

const char battery_voltage_format[] = "%5.3fV";
const char battery_level_format[] = "%3d%%";

const uint8_t current_speed_value_font_size = 10;
const uint8_t current_speed_value_cursor_x_single = screenWidth / 2 - (hor_font_blocks / 2.0) * current_speed_value_font_size;
const uint8_t current_speed_value_cursor_x_double = screenWidth / 2 - (hor_font_blocks + 0.5) * current_speed_value_font_size;
const uint8_t current_speed_value_cursor_y = screenHeight / 2 - (ver_font_blocks / 2.0) * current_speed_value_font_size;

const uint8_t current_speed_unit_font_size = 2;
const uint8_t current_speed_unit_cursor_x_single = current_speed_value_cursor_x_single + (hor_font_blocks + 1) * current_speed_value_font_size;
const uint8_t current_speed_unit_cursor_x_double = current_speed_value_cursor_x_double + (2 * hor_font_blocks + 2) * current_speed_value_font_size;
const uint8_t current_speed_unit_cursor_y = current_speed_value_cursor_y + ver_font_blocks * current_speed_value_font_size - ver_font_blocks * current_speed_unit_font_size;

const uint8_t current_duration_font_size = 2;
const uint8_t current_duration_cursor_x_single = screenWidth / 2 - (3.5f * hor_font_blocks + 3.0f) * current_duration_font_size;
const uint8_t current_duration_cursor_x_double = screenWidth / 2 - (4.0f * hor_font_blocks + 3.5f) * current_duration_font_size;
const uint8_t current_duration_cursor_y = current_speed_value_cursor_y - (ver_font_blocks + 2) * current_duration_font_size;

const uint8_t current_distance_font_size = 2;
const uint8_t current_distance_cursor_x_single = screenWidth / 2 - (3.0f * hor_font_blocks + 2.5f) * current_distance_font_size;
const uint8_t current_distance_cursor_x_double = screenWidth / 2 - (3.5f * hor_font_blocks + 3.0f) * current_distance_font_size;
const uint8_t current_distance_cursor_x_triple = screenWidth / 2 - (4.0f * hor_font_blocks + 3.5f) * current_distance_font_size;
const uint8_t current_distance_cursor_y = current_duration_cursor_y - (ver_font_blocks + 2) * current_distance_font_size;

const uint8_t current_avg_speed_font_size = 2;
const uint8_t current_avg_speed_value_cursor_x = current_avg_speed_font_size;
const uint8_t current_avg_speed_value_cursor_y = screenHeight / 2 - (ver_font_blocks + 0.5) * current_avg_speed_font_size;

const uint8_t current_avg_speed_unit_cursor_x = current_avg_speed_value_cursor_x;
const uint8_t current_avg_speed_unit_cursor_y = screenHeight / 2 + 0.5 * current_avg_speed_font_size;

const uint8_t current_max_speed_font_size = 2;
const uint8_t current_max_speed_value_cursor_x = screenWidth - (4.0 * hor_font_blocks + 4.0) * current_max_speed_font_size;
const uint8_t current_max_speed_value_cursor_y =  current_avg_speed_value_cursor_y;

const uint8_t current_max_speed_unit_cursor_x = current_max_speed_value_cursor_x;
const uint8_t current_max_speed_unit_cursor_y = current_avg_speed_unit_cursor_y;

const uint8_t alltime_distance_font_size = 2;
const uint8_t alltime_distance_cursor_x = screenWidth / 2 - (7.0 * hor_font_blocks + 6.5) * alltime_distance_font_size;
const uint8_t alltime_distance_cursor_y = current_speed_value_cursor_y + (ver_font_blocks * current_speed_value_font_size) + 2 * alltime_distance_font_size;

const uint8_t alltime_duration_font_size = 2;
const uint8_t alltime_duration_cursor_x = alltime_distance_cursor_x;
const uint8_t alltime_duration_cursor_y = alltime_distance_cursor_y + (ver_font_blocks * alltime_distance_font_size) + alltime_duration_font_size;

const uint8_t alltime_avg_speed_font_size = 2;
const uint8_t alltime_avg_speed_cursor_x = screenWidth / 2 + (3.0 * hor_font_blocks + 2.5) * alltime_avg_speed_font_size;
const uint8_t alltime_avg_speed_cursor_y = alltime_distance_cursor_y;

const uint8_t alltime_max_speed_font_size = 2;
const uint8_t alltime_max_speed_cursor_x = alltime_avg_speed_cursor_x;
const uint8_t alltime_max_speed_cursor_y = alltime_duration_cursor_y;

const uint8_t battery_level_font_size = 2;
const uint8_t battery_level_cursor_x = screenWidth / 2 - (2.0 * hor_font_blocks + 1.5) * battery_level_font_size;
const uint8_t battery_level_cursor_y = screenHeight - (ver_font_blocks + 2.0) * battery_level_font_size;

const uint8_t battery_voltage_font_size = 2;
const uint8_t battery_voltage_cursor_x = screenWidth / 2 - (3.0 * hor_font_blocks + 2.5) * battery_voltage_font_size;
const uint8_t battery_voltage_cursor_y = battery_level_cursor_y - (ver_font_blocks + 2.0) * battery_voltage_font_size;










Arduino_DataBus *bus = new Arduino_HWSPI(4 /*DC*/, 5 /*CS*/);
Arduino_G *output_display = new Arduino_GC9A01(bus, 3 /* RST */, 0 /* rotation */, true /* IPS */);
Arduino_GFX *gfx = new Arduino_Canvas(screenWidth, screenHeight, output_display);


float current_duration = 0; // in seconds
float previous_duration = 0; // in seconds

float current_distance = 0; // in meters
float previous_distance = 0; // in meters

float current_speed = 0; // in meters/second
float current_speed_kmh = 0; // in km/h

float current_avg_speed = 0; // in meters/second
float current_avg_speed_kmh = 0; // in meters/second

float current_max_speed = 0; // in meters/second
float current_max_speed_kmh = 0; // in meters/second

float alltime_max_speed = 0;
float alltime_max_speed_kmh = 0;

float alltime_distance = 0;
float alltime_duration = 0;
float alltime_avg_speed = 0;
float alltime_avg_speed_kmh = 0;



char current_speed_value_string[2];

char current_distance_string[8];
char current_duration_string[8];
char current_avg_speed_string[4];
char current_max_speed_string[4];

char alltime_distance_string[8];
char alltime_duration_string[8];
char alltime_avg_speed_string[4];
char alltime_max_speed_string[4];

char battery_level_string[4];
char battery_voltage_string[6];

float battery_voltage;
uint8_t battery_level;


uint16_t interface_color = 0x07E0;

// Battery Stuff
class HackAnalogIn: public mbed::AnalogIn {
  using mbed::AnalogIn::AnalogIn;
  public:
    analogin_t getAnalogIn_t();
};

analogin_t HackAnalogIn::getAnalogIn_t() {
  return this->_adc;
}

void startReadingBatteryLevel (nrf_saadc_value_t* buffer) {
  auto pin = PIN_VBAT;
  PinName name = analogPinToPinName(pin);
  if (name == NC) { return; }
  HackAnalogIn* adc = static_cast<HackAnalogIn*>(analogPinToAdcObj(pin));
  if (adc == NULL) {
    adc = new HackAnalogIn(name);
    analogPinToAdcObj(pin) = static_cast<mbed::AnalogIn*>(adc);
#ifdef ANALOG_CONFIG
    if (isAdcConfigChanged) {
      adc->configure(adcCurrentConfig);
    }
#endif
  }

  nrfx_saadc_buffer_convert(buffer, 1);
  nrfx_err_t ret = nrfx_saadc_sample();
  if (ret == NRFX_ERROR_BUSY) {
    // failed to start sampling
    return;
  }
}


void setup() {
  Serial.begin(115200);
  
  gfx->begin();
  gfx->fillScreen(RED);
  
  pinMode(GFX_BL, OUTPUT);
  digitalWrite(GFX_BL, HIGH);
  
  screenWidth = gfx->width();
  screenHeight = gfx->height();

  // Battery Stuff
  pinMode(P0_14, OUTPUT);
  digitalWrite(P0_14,LOW);
  pinMode(P0_13, OUTPUT);
  digitalWrite(P0_13,LOW);
}

nrf_saadc_value_t BatteryLevel = { 0 };

void loop() {

  previous_duration = current_duration;
  current_duration = ((float) millis()) / 1000.0 * 60.0;

  alltime_duration = current_duration;

  previous_distance = current_distance;
  current_distance = millis();

  alltime_distance = current_distance;

  current_speed = (current_distance - previous_distance) / (current_duration - previous_duration);
  current_speed_kmh = current_speed * 3.6;
  
  current_avg_speed = current_distance / current_duration;
  current_avg_speed_kmh = 3.6 * current_avg_speed;

  alltime_avg_speed = alltime_distance / alltime_duration;
  alltime_avg_speed_kmh = 3.6 * alltime_avg_speed;

  if (current_speed > current_max_speed) {
    current_max_speed = current_speed;
    current_max_speed_kmh = current_speed_kmh;
    
    if (current_speed > alltime_max_speed) {
      alltime_max_speed = current_speed;
      alltime_max_speed_kmh = current_speed_kmh;
    }
  }

  if (current_speed_kmh <= speed_limit_kmh / 2) {
    interface_color = gfx->color565(255 * current_speed_kmh / (speed_limit_kmh / 2), 255, 0);
  } else {
    interface_color = gfx->color565(255, 255 - min(255 * (current_speed_kmh - (speed_limit_kmh / 2)) / (speed_limit_kmh / 2), 255), 0);
  }

  // Battery stuff
  static unsigned long _lastT = 0;
  unsigned long _t = millis();

  if (_t - _lastT > battery_level_update_period) {
    startReadingBatteryLevel(&BatteryLevel);
    _lastT = _t;
  }
  
  // check if ADC conversion has completed
  if (nrf_saadc_event_check(NRF_SAADC_EVENT_DONE)) {
    // ADC conversion completed. Reading is stored in BatteryLevel
    nrf_saadc_event_clear(NRF_SAADC_EVENT_DONE);
    battery_voltage = (float)BatteryLevel / 4095 * 3.3 / 510 * (1000 + 510);
  }

  draw_base();
  draw_current_speed();
  draw_current_duration();
  draw_current_distance();
  draw_current_avg_speed();
  draw_current_max_speed();
  draw_alltime_distance();
  draw_alltime_avg_speed();
  draw_alltime_duration();
  draw_alltime_max_speed();
  draw_battery_voltage();
  draw_battery_level();

  gfx->flush();
}

void draw_base() {
  gfx->fillScreen(BLACK);
  gfx->setTextColor(interface_color);
}

void draw_current_speed() {
  
  gfx->setCursor(current_speed_kmh < 10 ? current_speed_value_cursor_x_single : current_speed_value_cursor_x_double, current_speed_value_cursor_y);
  gfx->setTextSize(current_speed_value_font_size);
  sprintf(current_speed_value_string, "%d", (int) roundf(current_speed_kmh));
  gfx->print(current_speed_value_string);
  
  /*gfx->setCursor(current_speed_kmh < 10 ? current_speed_unit_cursor_x_single : current_speed_unit_cursor_x_double, current_speed_unit_cursor_y);
  gfx->setTextSize(current_speed_unit_font_size);
  gfx->print(speed_unit);*/

  float arc_end = arc_start + min(arc_span * current_speed_kmh / speed_limit_kmh, arc_span);
  gfx->fillArc(screenWidth / 2, screenHeight / 2, screenWidth / 2, screenWidth / 2 - 24, arc_start, arc_end, interface_color);
}

void draw_current_distance() {
  sprintf(current_distance_string, current_distance_format, current_distance / 1000.0f);
  if (current_distance < 10000.0) {
    gfx->setCursor(current_distance_cursor_x_single, current_distance_cursor_y);
  } else if (current_distance < 100000.0) {
    gfx->setCursor(current_distance_cursor_x_double, current_distance_cursor_y);
  } else {
    gfx->setCursor(current_distance_cursor_x_triple, current_distance_cursor_y);
  }
  gfx->setTextSize(current_distance_font_size);
  gfx->print(current_distance_string);
}

void draw_current_duration() {
  uint8_t hours = current_duration / 3600;
  uint8_t minutes = (int) current_duration / 60 % 60;
  uint8_t seconds = (int) current_duration % 60;
  sprintf(current_duration_string, current_duration_format, hours, minutes, seconds);
  gfx->setCursor(hours < 10 ? current_duration_cursor_x_single : current_duration_cursor_x_double, current_duration_cursor_y);
  gfx->setTextSize(current_duration_font_size);
  gfx->print(current_duration_string);
}

void draw_current_avg_speed() {
  sprintf(current_avg_speed_string, current_avg_speed_format, current_avg_speed_kmh);
  gfx->setCursor(current_avg_speed_value_cursor_x, current_avg_speed_value_cursor_y);
  gfx->setTextSize(current_avg_speed_font_size);
  gfx->print(current_avg_speed_string);

  gfx->setCursor(current_avg_speed_unit_cursor_x, current_avg_speed_unit_cursor_y);
  gfx->print(speed_unit);
}

void draw_current_max_speed() {
  sprintf(current_max_speed_string, current_max_speed_format, current_max_speed_kmh);
  gfx->setCursor(current_max_speed_value_cursor_x, current_max_speed_value_cursor_y);
  gfx->setTextSize(current_max_speed_font_size);
  gfx->print(current_max_speed_string);

  gfx->setCursor(current_max_speed_unit_cursor_x, current_max_speed_unit_cursor_y);
  gfx->print(speed_unit);
}

void draw_alltime_distance() {
  sprintf(alltime_distance_string, alltime_distance_format, alltime_distance / 1000.0f);
  gfx->setCursor(alltime_distance_cursor_x, alltime_distance_cursor_y);
  gfx->setTextSize(alltime_distance_font_size);
  gfx->print(alltime_distance_string);
}

void draw_alltime_duration() {
  uint16_t hours = (int) alltime_duration / 3600;
  uint8_t minutes = (int) alltime_duration  / 60 % 60;
  sprintf(alltime_duration_string, alltime_duration_format, hours, minutes);
  gfx->setCursor(alltime_duration_cursor_x, alltime_duration_cursor_y);
  gfx->setTextSize(alltime_duration_font_size);
  gfx->print(alltime_duration_string);
}

void draw_alltime_avg_speed() {
  sprintf(alltime_avg_speed_string, alltime_avg_speed_format, alltime_avg_speed_kmh);
  gfx->setCursor(alltime_avg_speed_cursor_x, alltime_avg_speed_cursor_y);
  gfx->setTextSize(alltime_avg_speed_font_size);
  gfx->print(alltime_avg_speed_string);
}

void draw_alltime_max_speed() {
  sprintf(alltime_max_speed_string, alltime_max_speed_format, alltime_max_speed_kmh);
  gfx->setCursor(alltime_max_speed_cursor_x, alltime_max_speed_cursor_y);
  gfx->setTextSize(alltime_max_speed_font_size);
  gfx->print(alltime_max_speed_string);
}

void draw_battery_level() {
  battery_level = max(100.0 * (battery_voltage - 2.7) / 1.5, 0);
  sprintf(battery_level_string, battery_level_format, battery_level);
  gfx->setCursor(battery_level_cursor_x, battery_level_cursor_y);
  gfx->setTextSize(battery_level_font_size);
  gfx->print(battery_level_string);
}

void draw_battery_voltage() {
  sprintf(battery_voltage_string, battery_voltage_format, battery_voltage);
  gfx->setCursor(battery_voltage_cursor_x, battery_voltage_cursor_y);
  gfx->setTextSize(battery_voltage_font_size);
  gfx->print(battery_voltage_string);
}
