/**
 * The master device implementation. The master device (Arduino UNO) controls the display
 * and sends data to a node-red dashboard. The data is obtained via the i2c protocol from a 
 * slave device on address #8. The master device obtains the data by requesting 10 bytes from
 * the slave, then fills the appropriate buffer. When all buffers are filled, the data is displayed
 * on the LCD i2c display, and sent to the node-red dashboard.
 */

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

/** The display. */
LiquidCrystal_I2C lcd(0x27, 16, 2);

/** Data holds the read characters of the currents request. */
String data = "";
/** 
 *  The counter holds how many requests have been completed.
 *  This is needed so the buffer fills up before displaying the data.
 */
int counter = 0;

/** The holder of all the metrices. This needs to be full before displaying data. */
float dataHolder[4];

void setup() {
  // Join i2c bus with no address, as this is the master device.
  Wire.begin();
  Serial.begin(9600);

  // Initialize the display.
  lcd.init();
  lcd.clear();
  lcd.backlight();

  // Display initial message.
  lcd.setCursor(2,0);  
  lcd.print("Initializing");
  lcd.setCursor(2,1); 
  lcd.print("station");
}

void loop() {
  // From device #8, request 8 bytes of data.
  Wire.requestFrom(8, 10);
  Serial.println("Request sent");

  // While data is available, read and print to serial com port.
  // This is needed because the peripheral device might send less
  // than 10 bytes of data.
  while (Wire.available()) {
    char c = Wire.read();
    if (c != '\0') {
      Serial.print(c);
      data += c;
    }
    else {
      // When everything has been read, extract data and place it in the appropriate buffer.
      if (data.charAt(data.length() - 1) == 'T') {
        dataHolder[0] = data.substring(0, data.length() - 2).toFloat();
      }
      else if (data.charAt(data.length() - 1) == 'H') {
        dataHolder[1] = data.substring(0, data.length() - 2).toFloat();
      }
      else if (data.charAt(data.length() - 1) == 'P') {
        dataHolder[2] = data.substring(0, data.length() - 2).toFloat();
      }
      else if (data.charAt(data.length() - 1) == 'A') {
        dataHolder[3] = data.substring(0, data.length() - 2).toFloat();
      }
      
      data = "";
      break;
    }
  }
  Serial.println();

  counter++;

  // When all 4 metrics are gathered, proceed with displaying.
  if (counter == 4) {
    counter = 0;

    // Print to serial monitor - this is needed so the node-red dashboard
    // can extract the data and display it.
    Serial.println("Temp " + (String)dataHolder[0]);
    Serial.println("Hum " + (String)dataHolder[1]);
    Serial.println("Prs " + (String)dataHolder[2]);
    Serial.println("Alt " + (String)dataHolder[3]);

    // Display the data.
    lcd.clear();
    displayTemp(dataHolder[0]);
    displayHum(dataHolder[1]);
    delay(2000);
  
    lcd.clear();
    displayPress(dataHolder[2]);
    displayAlt(dataHolder[3]);
    delay(2000);
    
    data = "";
  }
  
  delay(1000);
}

/**
 * LCD display functions.
 */

void displayTemp(float data) {
  // Assemble the message of form: "Temp = val.val*C"
  lcd.setCursor(0,0);
  lcd.print("Temp = ");   
  lcd.setCursor(7, 0);
  lcd.print(data);
  lcd.setCursor(12, 0);
  lcd.print("*C");
}

void displayHum(float data) {
  // Assemble the message of form: "Hum = val.val%"
  lcd.setCursor(0,1);
  lcd.print("Hum = ");   
  lcd.setCursor(6, 1);
  lcd.print(data);
  lcd.setCursor(11, 1);
  lcd.print("%");
}

void displayPress(float data) {
  // Assemble the message of form: "Prs = val.valhPa"
  lcd.setCursor(0,0);
  lcd.print("Prs = ");   
  lcd.setCursor(6, 0);
  lcd.print(data);
  lcd.setCursor(12, 0);
  lcd.print("hPa");
}

void displayAlt(float data) {
  // Assemble the message of form: "Alt = val.valm"
  lcd.setCursor(0,1);
  lcd.print("Alt = ");   
  lcd.setCursor(6, 1);
  lcd.print(data);
  lcd.setCursor(11, 1);
  lcd.print("m");
}
