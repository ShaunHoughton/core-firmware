#include "exosite.h"
#include "idDHT22.h"
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

// declaration for DHT11 handler
int idDHT22pin = D4; //Digital pin for comunications
int DoorPin = D0;


void dht22_wrapper(); // must be declared before the lib initialization

//Setup sensor monitoring and control variables
double AmbTempCurrent, HumidityCurrent;
int AmbTempHigh, AmbHumiHigh;
int DoorState = LOW;
boolean DoorStateChanged = false;

//Default SensorTime to 30sec
unsigned long SensorPollTime = 60000;
unsigned long SensorTimer = millis();
unsigned long CurrentMilliSec = 0;

//Setup debounce time
volatile unsigned long DebounceTimer = 0;
int DebounceMilliSec = 100;

//Setup action constants
const int PwrOn = 1;
const int PwrOff = 2;
const int PwrCycle = 3;

//Setup message strings
char SensorData[75];
char MessageString[70];

// DHT instantiate
idDHT22 DHT22(idDHT22pin, dht22_wrapper);

//Function templates
void PollSensors();
void PollDoor();
void DoorStateChange();
int PowerControl(String Command);
int SetSensorParams(String Command);



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
    Serial.begin(115200);
    delay(3000);
    Serial.println("Boot");
    
    //Set time zone for South Africa
    Time.zone(+2);
    
    //Setup pin for door monitoring
    pinMode(DoorPin,INPUT_PULLUP);
    attachInterrupt(DoorPin, PollDoor, CHANGE);

	
	Serial.println("idDHT22 Example program");
	Serial.print("LIB version: ");
	Serial.println(idDHT22LIB_VERSION);
	Serial.println("---------------");
    
    //Setup external variables
    Spark.variable("Ambient_Temperature", &AmbTempCurrent, DOUBLE);
    Spark.variable("HumidityCurrentity", &HumidityCurrent, DOUBLE);
    Spark.variable("DoorState", &DoorState, INT);
    
    //Setup external functions
    Spark.function("Power", PowerControl);
    Spark.function("SensorParams",SetSensorParams);

}

// This wrapper is in charge of calling
// must be defined like this for the lib work
void dht22_wrapper()
{
	DHT22.isrCallback();
}

/*==============================================================================
 * loop
 *
 * Arduino loop function.
 *=============================================================================*/
void loop(){
    
    // check to see if timer has rolled over
    if (SensorTimer > millis())
    {
        SensorTimer = 0;
    }
    
    // check sensors as per timer setting
    if (millis() - SensorTimer > SensorPollTime)
    {
        PollSensors();
        delay(5000);
        SensorTimer = millis();
        //Write to "uptime" and read from "uptime" and "command" datasources.
        if ( exosite.writeRead(writeString+String(AmbTempCurrent), readString, returnString))
        {
            Serial.println("OK");
            Serial.println(returnString);
        }else
        {
            Serial.println("Error");
            delay(500);
        }
        
    }
    
    if (DoorStateChanged)
    {
        DoorStateChanged = false;
        Serial.print("Time to log door state change: ");
        Serial.println(millis() - CurrentMilliSec);
        sprintf(SensorData,"{\"Ambient_Temperature\":%.2f, \"Humidity\":%.2f,\"Door_State\":%d}",AmbTempCurrent, HumidityCurrent, DoorState);
        Spark.publish("SensorData",SensorData);
        
        
        //        if (DoorState == HIGH)
        //        {
        //            Serial.println("Door Opened");
        //            Spark.publish("Door-State", "Open", 60, PRIVATE);
        //
        //        } else
        //        {
        //            Serial.println("Door Closed");
        //            Spark.publish("Door-State", "Closed", 60, PRIVATE);
        //        }
    }
    
}

int PowerControl(String Command)
{
    char MessageString[60];
    Serial.print("Command = ");
    Serial.println(Command);
    String StrOutletNo = Command.substring(0,1);
    int OutletNo = StrOutletNo.toInt();
    String Action = Command.substring(2);
    int ActionNo = Action.toInt();
    //    Serial.print("Outlet: ");
    //    Serial.print(StrOutletNo);
    //    Serial.print(" ");
    //    Serial.println(OutletNo);
    //    Serial.print("Action: ");
    //    Serial.println(Action);
    sprintf(MessageString,"Outlet is %d Action is %d\n",OutletNo,ActionNo);
    Serial.println(MessageString);
    
    switch (ActionNo)
    {
        case PwrOn:
            Serial.println("On");
            break;
            
        case PwrOff:
            Serial.println("Off");
            break;
            
        case PwrCycle:
            Serial.println("Cycle");
            break;
            
        default:
            break;
    }
    
    return 1;
}

void PollDoor()
{
    CurrentMilliSec = millis();
    
    Serial.println(DebounceTimer);
    
    if (CurrentMilliSec > DebounceTimer)
    {
        DebounceTimer  = CurrentMilliSec + DebounceMilliSec;
        DoorState = digitalRead(DoorPin);
        DoorStateChanged = true;
    }
    
}

void PollSensors()
{
    
    Serial.print("\nRetrieving information from sensor: ");
	Serial.print("Read sensor: ");
	//delay(100);
	DHT22.acquire();
    Serial.println("DHT22 acquired.");
	while (DHT22.acquiring());
	int result = DHT22.getStatus();
	switch (result)
	{
		case IDDHTLIB_OK:
			Serial.println("OK");
			break;
		case IDDHTLIB_ERROR_CHECKSUM:
			Serial.println("Error\n\r\tChecksum error");
			break;
		case IDDHTLIB_ERROR_ISR_TIMEOUT:
			Serial.println("Error\n\r\tISR Time out error");
			break;
		case IDDHTLIB_ERROR_RESPONSE_TIMEOUT:
			Serial.println("Error\n\r\tResponse time out error");
			break;
		case IDDHTLIB_ERROR_DATA_TIMEOUT:
			Serial.println("Error\n\r\tData time out error");
			break;
		case IDDHTLIB_ERROR_ACQUIRING:
			Serial.println("Error\n\r\tAcquiring");
			break;
		case IDDHTLIB_ERROR_DELTA:
			Serial.println("Error\n\r\tDelta time to small");
			break;
		case IDDHTLIB_ERROR_NOTSTARTED:
			Serial.println("Error\n\r\tNot started");
			break;
		default:
			Serial.println("Unknown error");
			break;
	}
	
    HumidityCurrent = DHT22.getHumidity();
    Serial.print("Humidity (%): ");
	Serial.println(HumidityCurrent, 2);
    Serial.print("Humidity High Limit(%): ");
	Serial.println(AmbHumiHigh);
    
    AmbTempCurrent = DHT22.getCelsius();
    Serial.print("Temperature (oC): ");
	Serial.println(AmbTempCurrent, 2);
    Serial.print("Temperature High Limit(oC): ");
	Serial.println(AmbTempHigh);
    
    
    DoorState = digitalRead(DoorPin);
    Serial.print("Door State: ");
	Serial.println(DoorState);
    
    sprintf(SensorData,"{\"Ambient_Temperature\":%.2f, \"Humidity\":%.2f,\"Door_State\":%d}",AmbTempCurrent, HumidityCurrent, DoorState);
    Spark.publish("SensorData",SensorData);
    
    //Calculate and print out time
    unsigned long now = millis();
    unsigned nowSec = now/1000UL;
    unsigned sec = nowSec%60;
    unsigned min = (nowSec%3600)/60;
    unsigned hours = (nowSec%86400)/3600;
    unsigned days = (nowSec/86400);
    
    sprintf(MessageString,"{ Uptime: \"Days\": %u,\"Hours\": %u, \"Minutes\": %u, \"Seconds\": %u}",days,hours,min,sec);
    Serial.println(MessageString);
    Serial.println(Time.timeStr());
    
}

int SetSensorParams(String Command)
{
    char MessageString[70];
    Serial.print("Command = ");
    Serial.println(Command);
    int CommaAt = Command.indexOf(',');
    String StrSensor = Command.substring(0,CommaAt);
    String StrValue = Command.substring(CommaAt+1);
    double IntValue = StrValue.toInt();
    
    Serial.print("Sensor: ");
    Serial.println(StrSensor);
    Serial.print("String Value: ");
    Serial.println(StrValue);
    Serial.print("Value: ");
    Serial.println(IntValue);
    
    //    sprintf(MessageString,"Sensor is %s Value is %d\n",StrSensor, IntValue);
    //    Serial.println(MessageString);
    
    if (StrSensor.compareTo("AmbTempHigh") == 0)
    {
        AmbTempHigh = IntValue;
        return 1;
    } else if (StrSensor.compareTo("AmbHumiHigh") == 0)
    {
        AmbHumiHigh = IntValue;
        return 1;
    } else
    {
        return -1;
    }
    
}
