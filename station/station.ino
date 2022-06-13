/**
 * The slave device implementation. The slave device (Arduino NANO) controls the sensor
 * and sends data to the main device via i2c through a request event. It continuously
 * assembles the data in the loop function, and when the request is received, it transmits
 * the data currently in the buffer.
 */

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

/** The approximate pressure at sea level. */
#define SEALEVELPRESSURE_HPA (1013.25)

/** The sensor. */
Adafruit_BME280 bme;

/** toSend enum is used to indicate which data must be read next. */
enum toSend {
  Temperature,
  Pressure,
  Altitude,
  Humidity
} current = toSend::Temperature;

/** The data buffer. */
char buff[15];

void setup() {
  // Join i2c bus with address #8.
  Wire.begin(8);
  Wire.onRequest(requestEvent);

  
  Serial.begin(9600);
  bool status;

  // Check if sensor is connected, if not, stop execution.
  status = bme.begin(0x76);
  if (!status) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while(1);
  }

}

void loop() {
  // Assemble the data that will be sent via the i2c communication.
  // This has to be assembled in the loop function as the requestEvent
  // function acts as an interrupt and must be as fast as possible, otherwise
  // the i2c communication hangs.
  assembleData();
  
  Serial.println(buff);
  delay(1000);
}

/**
 * Assembles the data in a form appropriate for sending via i2c.
 * It automatically cycles through and assembles the next metric 
 * which needs to be sent. 
 * This can and should be optimised in the future.
 */
String assembleData() {
  // Check which metric needs to be sent next. Read that metric into the buffer,
  // add the flag that signifies what the metric is, and zero terminate the string.
  if (current == toSend::Temperature) {
    current = toSend::Pressure;
    dtostrf(bme.readTemperature(), 4, 4, buff);
    for (int i = 0; i < 15; i++) {
      if (buff[i] == '\0') {
        buff[i] = 'T';
        buff[i + 1] = '\0';
        break;
      }
    }
  }
  else if (current == toSend::Pressure) {
    current = toSend::Altitude;  
    dtostrf(bme.readPressure() / 100.0F, 4, 4, buff);
    for (int i = 0; i < 15; i++) {
      if (buff[i] == '\0') {
        buff[i] = 'P';
        buff[i + 1] = '\0';
        break;
      }
    }
  }
  else if (current == toSend::Altitude) {
    current = toSend::Humidity;
    dtostrf(bme.readAltitude(SEALEVELPRESSURE_HPA), 4, 4, buff);
    for (int i = 0; i < 15; i++) {
      if (buff[i] == '\0') {
        buff[i] = 'A';
        buff[i + 1] = '\0';
        break;
      }
    }
  }
  else if (current == toSend::Humidity) {
    current = toSend::Temperature;
    dtostrf(bme.readHumidity(), 4, 4, buff);
    for (int i = 0; i < 15; i++) {
      if (buff[i] == '\0') {
        buff[i] = 'H';
        buff[i + 1] = '\0';
        break;
      }
    }
  }
}

/**
 * Code that executes when the master device requests data from this device.
 * All that happens is the buffer assembled in the loop function is sent.
 */
void requestEvent() {
  Wire.write(buff, 10);
}
