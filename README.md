# IR Daikin Server ESP8266
Simple http interface to control Daikin A/C units via IR using ESP8266.
UPD: 
1.New WEB-UI (html page loads from Littlefs)
2.Removed EEPROM save\restore - from data is now saved to localStorage on client side with JS

# Dependencies
 - Arduino core for ESP8266 WiFi chip (https://github.com/esp8266/Arduino)
 - IRremote ESP8266 Library (https://github.com/markszabo/IRremoteESP8266)

# Suggested circuit
![Suggested Circuit](/suggested_circuit.png?raw=true "Suggested Circuit")
