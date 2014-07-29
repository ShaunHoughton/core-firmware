#include "exosite.h"
/*==============================================================================
 * Configuration Variables
 *
 * Change these variables to your own settings.
 *=============================================================================*/
String cikData = "451130cdfcb4279d270b2dcf733fdd5e3da36aee";  // <-- Fill in your CIK here! (https://portals.exosite.com -> Add Device)

// Use these variables to customize what datasources are read and written to.
String readString = "temp";
String writeString = "temp=";
String returnString;

/*==============================================================================
 * End of Configuration Variables
 *=============================================================================*/
class TCPClient client;
Exosite exosite(cikData, &client);

/*==============================================================================
 * setup
 *
 * Arduino setup function.
 *=============================================================================*/
void setup(){
    Serial.begin(9600);
    delay(3000);
    Serial.println("Boot");
}

/*==============================================================================
 * loop
 *
 * Arduino loop function.
 *=============================================================================*/
void loop(){
    //Write to "uptime" and read from "uptime" and "command" datasources.
    if ( exosite.writeRead(writeString+String(25), readString, returnString)){
        Serial.println("OK");
        Serial.println(returnString);
        delay(30000);
    }else{
        Serial.println("Error");
        delay(500);
    }
    
}
