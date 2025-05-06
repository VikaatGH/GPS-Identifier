#include <SPI.h>                  // For SPI communication (used by SD card)
#include <SD.h>                   // For SD card handling
#include <LiquidCrystal_I2C.h>    // For LCD display via I2C
#include <TinyGPSPlus.h>          // For GPS data parsing
#include <SoftwareSerial.h>       // For serial communication with GPS module

#define SD_CS 9
#define LINE_BUFFER_SIZE 15

LiquidCrystal_I2C lcd(0x27, 16, 2);
TinyGPSPlus gps;                  // GPS parser object
SoftwareSerial ss(2, 3);          // Software serial object to receive data from GPS module

int radius = 0;                   // Radius to consider "reaching" a stop
double distance = 0;              // Current distance to stop
double maxDistance = 0;
double totalDistance = 0;
double prevLat = 0;               // Previous GPS coordinates
double prevLng = 0;
double curLat = 0;                // Current GPS coordinates
double curLng = 0;
bool readNextStop = true;         // Flag to read next stop, if the previous stop was reached
unsigned long startTime;          // Timer for GPS fix attempts
File myFile;                      // File object for SD card

struct Stop {
  String name;
  float latitude;
  float longitude;
};
Stop stop;

// States of finite state machine
enum states { START, DISPLAY_START, DISPLAY_FALSE_START, DISPLAY_ERROR, CHECK_GPS, DISPLAY_MOVE, DISPLAY_STOP, DISPLAY_END, END };
states STATE;                     // Current state

void setup() {
  Serial.begin(9600);
  ss.begin(9600);                 // Initialize GPS module serial

  lcd.init();
  lcd.backlight();

  if (!SD.begin(SD_CS)) {
    Serial.println(F("No SD card."));
    lcd.print("No SD card.");
    delay(2000);
    STATE = END;                  // End program if no SD card
    return;
  }

  myFile = SD.open("stops.txt");
  if (!myFile) {
    Serial.println(F("Can't open the file."));
    lcd.print("Can't open the file.");
    delay(2000);
    STATE = END;                  // End program if file not found
    return;
  }

  // Read first line to determine sport type and set radius
  String sport = myFile.readStringUntil('\n');
  Serial.println(sport);
  if (sport == "Run") {
    radius = 70;
  } else if (sport == "Cycle") {
    radius = 100;
  } else {
    radius = 40;
  }

  // Read the first stop from file
  if (readStop(myFile, stop)) {
    Serial.println(F("The route is empty."));
    lcd.print("The route is empty.");
    delay(2000);
    STATE = END;                  // End if no stops
  } else {
    STATE = START;                // Ready to start tracking
    Serial.println(stop.name);
    Serial.println(stop.latitude, 6);
    Serial.println(stop.longitude, 6);
  }
}

bool readLine(File &myFile, char *buffer, size_t len) {
  size_t pos = 0;
  while (myFile.available()) {
    char c = myFile.read();
    if (c == '\n' || c == '\r') {
      if (pos == 0) continue;     // skip empty lines or \r\n combinations
      break;
    }
    if (pos < len - 1) {
      buffer[pos++] = c;
    }
  }
  buffer[pos] = '\0';             // null-terminate
  return pos > 0;
}

// Read a stop (name, latitude, longitude) from file
int readStop(File &myFile, Stop &stop) {
  char lineBuffer[LINE_BUFFER_SIZE];
  if (!readLine(myFile, lineBuffer, LINE_BUFFER_SIZE)) return 1;
  stop.name = String(lineBuffer);

  if (!readLine(myFile, lineBuffer, LINE_BUFFER_SIZE)) return 1;
  stop.latitude = atof(lineBuffer);

  if (!readLine(myFile, lineBuffer, LINE_BUFFER_SIZE)) return 1;
  stop.longitude = atof(lineBuffer);

  return 0;
}

// Get GPS location and store in provided pointers
int getGPS(double* curLat, double* curLng) {
  while (ss.available() > 0) {
    if (gps.encode(ss.read())) {
      if (gps.location.isValid()) {
        *curLat = gps.location.lat();
        *curLng = gps.location.lng();
        Serial.println(F("GPS"));
        Serial.println(*curLat, 6);
        Serial.println(*curLng, 6);
        return 0;
      } else return 1;            // Invalid location
    } else return 1;              // GPS not ready
  }
}

void loop() {
  switch (STATE) {
    // Case when a user is at the start point
    case START: {
      Serial.println(F("START"));
      startTime = millis();
      bool gotFix = false;

      // Try for 5 seconds to get a GPS fix
      while (millis() - startTime < 5000) {
        if (!getGPS(&curLat, &curLng)) {
          gotFix = true;
          break;
        }
      }

      if (gotFix) {
        distance = TinyGPSPlus::distanceBetween(curLat, curLng, stop.latitude, stop.longitude);
        Serial.print(F("Start distance "));
        Serial.println(distance);
        if (distance <= 30) {
          STATE = DISPLAY_START;
        } else {
          STATE = DISPLAY_FALSE_START;
        }
        prevLat = curLat;
        prevLng = curLng;
      } else {
        Serial.println(F("Can't get GPS."));
        STATE = DISPLAY_ERROR;
      }
      break;
    }

    // Case when a module can't get GPS
    case DISPLAY_ERROR: {
      Serial.println(F("DISPLAY_ERROR"));
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Can't get GPS");
      delay(2000);
      STATE = START;
      break;
    }

    // informing of a start
    case DISPLAY_START: {
      Serial.println(F("DISPLAY_START"));
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Let's start");
      delay(3000);
      STATE = CHECK_GPS;
      break;
    }

    // Case when a user is not at the starting point
    case DISPLAY_FALSE_START: {
      Serial.println(F("DISPLAY_FALSE_START"));
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Not at the start");
      delay(2000);
      STATE = START;
      break;
    }

    // Read the next station if the previous station was reached and compare it to the actual GPS position
    case CHECK_GPS: {
      Serial.println(F("CHECK_GPS"));
      if (readNextStop) {
        if (readStop(myFile, stop)) {
          STATE = DISPLAY_END;
          break;
        }
      }

      startTime = millis();
      bool gotFix = false;

      while (millis() - startTime < 5000) {  
        if (!getGPS(&curLat, &curLng)) {
          gotFix = true;
          break;
        }
      }

      if (gotFix) {
        prevLat = curLat;
        prevLng = curLng;
      } else {
        Serial.println(F("Can't get GPS. Will use old values."));
      }

      distance = TinyGPSPlus::distanceBetween(prevLat, prevLng, stop.latitude, stop.longitude);
      if (readNextStop) {
        maxDistance = distance;
      } else {
        if (distance > maxDistance) maxDistance = distance;
      }

      Serial.println(F("CHECK_GPS distance"));
      Serial.println(distance);
      if (distance <= radius) {
        STATE = DISPLAY_STOP;
      } else {
        STATE = DISPLAY_MOVE;
      }

      break;
    }

    // Case when a user is between two stops
    case DISPLAY_MOVE: {
      Serial.println(F("DISPLAY_MOVE"));
      lcd.clear();
      lcd.print("Next: ");
      lcd.print(stop.name);
      lcd.setCursor(0, 1);
      lcd.print("Left: ");
      if (distance >= 1000) {
        lcd.print(distance / 1000, 2);
        lcd.print(" km");
      } else {
        lcd.print((int)distance);
        lcd.print(" m");
      }
      delay(3000);
      readNextStop = false;
      STATE = CHECK_GPS;
      break;
    }

    // Case when a user has reached a stop
    case DISPLAY_STOP: {
      Serial.println(F("DISPLAY_STOP"));
      lcd.clear();
      lcd.print("Reached: ");
      lcd.print(stop.name);
      lcd.setCursor(0, 1);
      lcd.print("Total: ");

      totalDistance += maxDistance;

      if (totalDistance >= 1000) {
        lcd.print(totalDistance / 1000, 2);
        lcd.print(" km");
      } else {
        lcd.print((int)totalDistance);
        lcd.print(" m");
      }
      delay(2000);
      readNextStop = true;
      STATE = CHECK_GPS;
      break;
    }

    // Case when a user has reached the end of the route
    // Display congrats and total distance
    case DISPLAY_END: {
      Serial.println(F("DISPLAY_END"));
      lcd.clear();
      lcd.print("Good job");
      lcd.setCursor(0, 1);
      lcd.print("Total: ");
      if (totalDistance >= 1000) {
        lcd.print(totalDistance / 1000, 2);
        lcd.print(" km");
      } else {
        lcd.print((int)totalDistance);
        lcd.print(" m");
      }
      Serial.println("Total distance: ");
      Serial.print(totalDistance);
      myFile.close();
      while (true) {};
    }

    // Case when the program couldn't find SD card, open the file or the file was empty
    case END: {
      Serial.println(F("END"));
      while (true);
    }
  }
}

