# GPS - Identifier 
This Arduino project creates a simple device that tracks your geolocation and guides you along a predefined route — perfect for walks, runs, or cycling trips.

## Hardware description
This project is built around an Arduino Uno microcontroller and uses a 1602 character LCD display, a NEO-6M GPS module for receiving position data, and a microSD card reader for data storage. The components were assembled on a solderless breadboard.

Here is an example of connecting:

| LCD display 1602 with I2C | Arduino Uno |
|---------------------------|-------------|
| GND           |  GND|
|VCC|5V|
|SDA|A4|
|SCL|A5|

The display and microcontroller communicate via the I2C bus. The LiquidCrystal_I2C.h library is used to interface with the display.<br><br>



|GPS module NEO 6M|Arduino Uno|
|-----------------|-----------|
|GND|GND|
|VCC| 3.3V or 5V, if a module has a voltage regulator|
|RX|3|
|TX|2|

The NEO-6M GPS module communicates with the microcontroller via a serial connection. The SoftwareSerial.h library is used to handle the communication, while the TinyGPSPlus.h library processes the GPS data. For optimal performance, the NEO-6M GPS module needs an unobstructed view of the sky to receive satellite signals.<br><br>



| Micro SD reader| Arduino Uno|
|----------------|------------|
|GND|GND|
|VCC|5V|
|MISO|12|
|MOSI|11|
|SCK|13|
|CS|8|

The SD card reader communicates with the microcontroller via the SPI bus. The SPI.h library is used to configure the SPI protocol, while the SD.h library handles file operations on the SD card.

## Usage

1. Route preparation:<br>
The user accesses a specially designed website to create a route. By clicking on the map and entering location names, additional stops can be added. These stops can be edited (e.g., to change their names) or deleted. Next, the user must choose the mode of transport for traveling along the route — options include walking, running, or cycling. Afterward, by clicking the "Save route" button, a file named stops.txt will be automatically downloaded.

2. Working with the stops.txt file:<br>
The stops.txt file contains details about the selected mode of transport and the individual stops (including their names, latitudes, and longitudes). Each piece of information is placed on a new line. It is important to manually save this file to an SD card, which will then be connected to the Arduino.

3. Start of the Program:<br>
Once the Arduino is powered on, the program begins. It first compares the user-defined first stop with his current location. If the user is not at the correct starting point, the program will alert them to move there. Once the user reaches the correct position, the device will begin providing updates on their progress along the route via the display.

4. Travel Information: <br>
While the user is traveling between stops, the display shows the name of the next stop and the remaining distance to it. Once the user enters the predefined radius around the next stop, the device announces the name of the stop and the total distance traveled. The radius around each stop is adjusted based on the selected mode of transport — the faster the user is moving, the larger the radius.

5. End of the Program:<br>
Once the user reaches the final stop of the route, the display will show a greeting message along with a summary of the total distance traveled.

## Web link
Here is the website to create the route: https://vikaatgh.github.io./GPS-Identifier/

## Repository navigation
- The docs folder contains the code for the webpage used to create the route.<br>
- The code folder stores the Arduino program.<br>
- The example folder includes the stops.txt file with the route data, as well as a log file that shows all prints to the Serial Monitor during program execution.

