//static const char DEVICE_ID[] = "71GM9xi757";
static const char DEVICE_ID[] = "2GMD3PyH22";

static const unsigned long SEND_READING_INTERVAL = 5000; // milliseconds
static const unsigned long UPDATE_INTERVAL = SEND_READING_INTERVAL / 10; // milliseconds

#define USE_BLUETOOTH

#include "Bluetooth.hpp"
#include "CarbonMonoxide.hpp"
#include "TemperatureHumidity.hpp"
#include "UV.hpp"
#include "OpticalDust.hpp"

Bluetooth bluetooth;
CarbonMonoxide carbonMonoxide;
TemperatureHumidity temperatureHumidity;
UV uv;
OpticalDust opticalDust;

static unsigned long lastUpdateTime;

void setup() {
	carbonMonoxide.init();
	temperatureHumidity.init();
	uv.init();
	opticalDust.init();

	#ifdef USE_BLUETOOTH
	pinMode(13, OUTPUT);
	digitalWrite(13, HIGH);
	Serial1.begin(9600);
	bluetooth.init();
	#else
	Serial.begin(9600);
	#endif
}

void loop() {
	carbonMonoxide.update();
	temperatureHumidity.update();
	uv.update();
	opticalDust.update();

	unsigned long now = millis();
	if (now - lastUpdateTime > SEND_READING_INTERVAL) {
		sendReading();
		lastUpdateTime = now;
	}

	delay(UPDATE_INTERVAL);
}

// Data Format
// -----------
// deviceID    => string  (10 bytes)
// temperature => float   ( 4 bytes)
// humidity    => float   ( 4 bytes)
// UV          => float   ( 4 bytes)
// particles   => float   ( 4 bytes)
// CO          => int16_t ( 2 bytes)
// EOL         => string  ( 2 bytes)

#define serialWrite(out, data) (out.write((const char *) &data, sizeof(data)))

void sendReading() {
#ifdef USE_BLUETOOTH
	HardwareSerial &out = Serial1;
	bluetooth.update();
	if (!bluetooth.connected()) {
		return;
	}
#else
	Serial_ &out = Serial;
#endif

#ifdef USE_BLUETOOTH
	out.write(DEVICE_ID);
	serialWrite(out, temperatureHumidity.celcius);
	serialWrite(out, temperatureHumidity.humidity);
	serialWrite(out, uv.value);
	serialWrite(out, opticalDust.value);
	serialWrite(out, carbonMonoxide.value);
	out.write("\r\n");
	out.flush();
#else
	Serial.print("deviceID: "); Serial.print(DEVICE_ID);
	Serial.print(", temperature: "); Serial.print(temperatureHumidity.celcius);
	Serial.print(", humidity: "); Serial.print(temperatureHumidity.humidity);
	Serial.print(", carbonMonoxide: "); Serial.print(carbonMonoxide.value);
	Serial.print(", uv: "); Serial.print(uv.value);
	Serial.print(", particles: "); Serial.print(opticalDust.value);
	Serial.println();
#endif
}
