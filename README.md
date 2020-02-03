# bus-clock-esp-32s
Device for bus arrival notification. Tells you when bus comes via API. supports multiple buses.

Inputs are 'touches' that esp-32 supports. touch 3 pins with hand. 
It may work better if hand is slightly wet, or threshold for touch can be changed.


Outputs are Blynk app alarm, and liquid crystal display.
I also added custom charactor for lc display. (Each number of custom char represents a row of 8 pixels.)


Following are libraries used in this project. 

- BlynkSimpleEsp32.h
- Wire.h
- LiquidCrystal_I2C.h
- Arduino.h
- WiFi.h
- WiFiMulti.h
- HTTPClient.h

Blynk and LiquidCrystal libs can be modified or deleted, as they are only related to output part.
Also, Blynk main function should be executed very often. IE) called from loop(), without delay functions.
But delay function was crucial, so a new function was made.
The new delay function calls Blynk function every millisecond. This solved the problem.
