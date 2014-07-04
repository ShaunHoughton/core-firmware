// This #include statement was automatically added by the Spark IDE.
#include "idDHT22.h"


// declaration for DHT11 handler
int idDHT22pin = D4; //Digital pin for comunications
int DoorPin = D0;


void dht22_wrapper(); // must be declared before the lib initialization
double TempC, TempF, Humid, DewP;
int DoorState = HIGH;
boolean DoorStateChanged = false;

//Default SensorTime to 30sec
unsigned long SensorPollTime = 10000;
unsigned long SensorTimer = millis();
unsigned long CurrentMilliSec = 0;

//Setup debounce time
volatile unsigned long DebounceTimer = 0;
int DebounceMilliSec = 100;

//Setup action constants
const int PwrOn = 1;
const int PwrOff = 2;
const int PwrCycle = 3;

// DHT instantiate
idDHT22 DHT22(idDHT22pin, dht22_wrapper);

//Function templates
void PollSensors();
void PollDoor();
void DoorStateChange();
int PowerControl(String Command);




void setup()
{
    pinMode(DoorPin,INPUT_PULLUP);
    attachInterrupt(DoorPin, PollDoor, CHANGE);
    
	Serial.begin(115200);
	
	Serial.println("idDHT22 Example program");
	Serial.print("LIB version: ");
	Serial.println(idDHT22LIB_VERSION);
	Serial.println("---------------");
    
    //Setup external variables
    Spark.variable("TemperatureC", &TempC, DOUBLE);
    Spark.variable("DewPoint", &DewP, DOUBLE);
    Spark.variable("Humidity", &Humid, DOUBLE);
    Spark.variable("DoorState", &DoorState, INT);
    
    //Setup external functions
    Spark.function("Power", PowerControl);
}

// This wrapper is in charge of calling
// must be defined like this for the lib work
void dht22_wrapper()
{
	DHT22.isrCallback();
}

void loop()
{
    
    // check to see if timer has rolled over
    if (SensorTimer > millis())
    {
        SensorTimer = 0;
    }
    
    // check sensors as per timer setting
    if (millis() - SensorTimer > SensorPollTime)
    {
        PollSensors();
        SensorTimer = millis();
    }
    
    if (DoorStateChanged)
    {
        DoorStateChanged = false;
        Serial.print("Time to log door state change: ");
        Serial.println(millis() - CurrentMilliSec);
        
        if (DoorState == HIGH)
        {
            Serial.println("Door Opened");
            Spark.publish("Door-State", "Open", 60, PRIVATE);
            
        } else
        {
            Serial.println("Door Closed");
            Spark.publish("Door-State", "Closed", 60, PRIVATE);
        }
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
	while (DHT22.acquiring())
		;
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
	
    Humid = DHT22.getHumidity();
    Serial.print("Humidity (%): ");
	Serial.println(Humid, 2);
    
    TempC = DHT22.getCelsius();
    Serial.print("Temperature (oC): ");
	Serial.println(TempC, 2);
    
	DewP = DHT22.getDewPoint();
    Serial.print("Dew Point (oC): ");
	Serial.println(DewP);
    
    DoorState = digitalRead(DoorPin);
    Serial.print("Door State: ");
	Serial.println(DoorState);
    
    
}
