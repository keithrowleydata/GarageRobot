/*
Garage Robot
By: Keith Rowley
Started: June 11th, 2012

The following sites provided code that was modified for use in this project:
http://www.hacktronics.com/Tutorials/arduino-1-wire-tutorial.html
 http://www.hacktronics.com/Tutorials/arduino-1-wire-address-finder.html
*/

/* *** Command Characters Used *** 
a - Has the door auto closed since last check

b - DS18B20 temp at control box is garage

c - Click garage door

d - DS18B20 temp downstairs in the basement
e - DHT11 humidity dowstairs in the basement
f - DHT11 temp downstairs in the basement

g - Garage door status
m - Number of milliseconds since last motion was detected in the garage

o - DS18B20 temp outside
-p - DHT22 humidity outside
-q - DHT22 temp outside

s - Status of the switch on top of control box

t - DS18B20 temp in the attic

u - DS18B20 temp upstairs in the hallway
-v - DHT11 humidity upstairs in the hallway
-w - DHT11 temp upstairs
*/


/* *** Included Libraries *** */
#include <OneWire.h>
#include <DallasTemperature.h>
#include <DHT.h>

/* *** Constants *** */

/* Led Pin */
#define LedPin 13

/* DS18B20 Pins */
#define ONE_WIRE_BUS 14
#define ONE_WIRE_BUSTWO 15

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);
OneWire oneWireTwo(ONE_WIRE_BUSTWO);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

// Assign the addresses of 1-Wire temp sensors.
DeviceAddress GarageThermometer =     { 0x28, 0x76, 0x6D, 0x97, 0x03, 0x00, 0x00, 0x4A };
DeviceAddress UpstairsThermometer =   { 0x28, 0x7F, 0xA5, 0x97, 0x03, 0x00, 0x00, 0xC0 };
DeviceAddress BasementThermometer =   { 0x28, 0x59, 0x7B, 0x97, 0x03, 0x00, 0x00, 0x38 };
DeviceAddress OutsideThermometer =    { 0x28, 0xEA, 0x6F, 0x97, 0x03, 0x00, 0x00, 0x05 };
DeviceAddress AtticThermometer =      { 0x28, 0x7D, 0x49, 0x97, 0x03, 0x00, 0x00, 0x55 };

/* DHT Pins */
#define DHT_OUTSIDE 16e
#define DHT_UPSTAIRS 17
#define DHT_BASEMENT 18

DHT DhtOutside(DHT_OUTSIDE, DHT22);
DHT DhtUpstairs(DHT_UPSTAIRS, DHT11);
DHT DhtBasement(DHT_BASEMENT, DHT11);

/* Relay Pins */
#define ExtraRelayOne 9        // Extra Relay
#define ExtraRelayTwo 10       // Extra Relay
#define CarDoorClicker 11      // The Car Clickers run through this relay so they can be turned off from code.
#define GarageDoorClicker 12   // Port with garage door relay attached.

/* HC-SR04 distance sensor constants */
#define SR04_TriggerPin 7           // Trigger Pin
#define SR04_EchoPin 8              // EchoPin
#define DoorOpenDistance 50         // Distance in CM of the Garage Door when it is open
#define NextDoorCheckWaitTime 5000  // Wait 5 seconds between door status checks.

// SR04 Timer variable
unsigned long NextDoorStatusCheck = 0;

// Time the garage can remain open with no motion before the program closes it.
#define GARAGE_CAN_STAY_OPEN 600000 // 10 Minutes.

// Variable for switch to turn off auto closing.
#define TopSwitch 19 // A5 = digital pin 19

// Motion Sensor Pin
#define MotionSensor 2

// Motion Sensor Timer variable
volatile unsigned long LastMotion = 0;

// Auto Closed Event variable
int DoorAutoClosed = 0;

/* 
  Garage Door Status.
  0 for unknown.
  1 for Closed.
  2 for open.
*/
int GarageDoorStatus;

// incoming serial character.
char inChar;

void setup()
{
  // start serial port at 9600 bps
  Serial.begin(9600);

  // Set the Relay pins High - Low is on with this relay.
  digitalWrite(ExtraRelayOne, HIGH);
  digitalWrite(ExtraRelayTwo, HIGH);
  digitalWrite(CarDoorClicker, HIGH);
  digitalWrite(GarageDoorClicker, HIGH);

  // Set the Relay pins as OUTPUT.
  pinMode(ExtraRelayOne, OUTPUT); 
  pinMode(ExtraRelayTwo, OUTPUT); 
  pinMode(CarDoorClicker, OUTPUT); 
  pinMode(GarageDoorClicker, OUTPUT); 

  // SR04 Pins  
  pinMode(SR04_TriggerPin, OUTPUT);
  pinMode(SR04_EchoPin, INPUT);
  
  // Set Top Switch as an input and enable pullup resister for it 
  pinMode(TopSwitch, INPUT);
  digitalWrite(TopSwitch, HIGH);
  
  // Set motion sensor pin as an input, enable pullup resister for it and Attach an interrupt to this pin.
  pinMode(MotionSensor, INPUT);
  digitalWrite(MotionSensor, HIGH);
  attachInterrupt(0, MotionDetected, CHANGE);
  
  // Set Led Pin as an output.
  pinMode(LedPin, OUTPUT);
  
  // Start up the DallasTemperature library
  sensors.begin();
  
  // set the resolution for the DB18B20 temp sensors to 10 bit (good enough?)
   //sensors.setResolution(GarageThermometer, 10);
   sensors.setResolution(UpstairsThermometer, 10);
   sensors.setResolution(BasementThermometer, 10);
   sensors.setResolution(OutsideThermometer, 10);
   //sensors.setResolution(AtticThermometer, 10);
  
  // Start up the DHT Humidity Sensors.
  DhtOutside.begin();
  DhtUpstairs.begin();
  DhtBasement.begin();
}

void loop()
{
  if (Serial.available() > 0) 
  {
    // get incoming byte:
    inChar = Serial.read();
    //Serial.println(inChar);
    
    if (inChar == 'c')
    {
      ClickGarageDoor();
    }
    
    if (inChar == 'g')
    {    
      // Print the garage door status to the serial port.
      if (GarageDoorStatus == 1)
      {
        Serial.println("Garage Door: Closed");
      }
      
      if (GarageDoorStatus == 2)
      {
        Serial.println("Garage Door: Open");
      }  
    }
    
    if (inChar == 'm')
    {
      // Print the number of miliseconds since last motion to the serial port.
      Serial.println("Millis since Last Motion: " + String(millis() - LastMotion));
    }
    
    if (inChar == 's')
    {
      // Print the Top Switch status to the serial port.
      if (digitalRead(TopSwitch) == HIGH)
      {
        Serial.println("Switch: On");
      }
      else
      {
        Serial.println("Switch: Off");
      }
    }
    
    if (inChar == 'a')
    {    
      // Tell if the garage door has been auto closed since this question was last asked.
      if (DoorAutoClosed == 1)
      {
        Serial.println("Garage Door Auto Closed: True");
        DoorAutoClosed = 0;
      }
      else
      {
        Serial.println("Garage Door Auto Closed: False");
      }  
    }
    
    if (inChar == 'b')
    {    
      // Temperature at controll box in garage.
      printTemperature(GarageThermometer);
    }
    
    if (inChar == 'd')
    {    
      // Temperature downstairs.
      printTemperature(BasementThermometer);
    }
    
    if (inChar == 'e')
    {    
      // Temperature downstairs.
      DHTprintHumidity(DhtBasement);
    }
    
    if (inChar == 'f')
    {    
      // Temperature downstairs.
      DHTprintTemperature(DhtBasement);
    }
    
    if (inChar == 'o')
    {    
      // Temperature Outside.
      printTemperature(OutsideThermometer);
    }
    
    if (inChar == 't')
    {    
      // Temperature Outside.
      printTemperature(AtticThermometer);
    }
    
    if (inChar == 'u')
    {    
      // Temperature Outside.
      printTemperature(UpstairsThermometer);
    }
  }
  
  if(millis() >= NextDoorStatusCheck)
  {
    // Chech the garage door status every NextDoorCheckWaitTime Milliseconds.
    GarageDoorStatus = CheckDoorStatus();
  }
  
  if((GarageDoorStatus == 2) && (millis() > LastMotion + GARAGE_CAN_STAY_OPEN) && (digitalRead(TopSwitch) == HIGH))
  {
    // Auto close the garage door if: 
    // the garage door is open
    // and the last motion in the garage was more than GARAGE_CAN_STAY_OPEN milliseconds ago
    // and the TopSwitch is on.
    
    // Use flipping the LED as a standin for auto closing the door for now. 
    digitalWrite(LedPin, !(digitalRead(LedPin)));
    
//    ClickGarageDoor();
    
    // Note that the door was auto closed in a reporting variable.
    DoorAutoClosed = 1;
  }
}

/**************************************************************/
/*****************  Garage Door Functions  ********************/
/**************************************************************/
void ClickGarageDoor() 
{
  Serial.println("ClickGarageDoor");
  digitalWrite(GarageDoorClicker, LOW);
  delay(2000);
  digitalWrite(GarageDoorClicker, HIGH);
}


/**************************************************************/
/********************   SR04 Functions  ***********************/
/**************************************************************/
int CheckDoorStatus()
{
  /*
  HC-SR04 Ping distance sensor
  VCC to arduino 5v GND to arduino GND
  Trig to Arduino pin SR04_TriggerPin
  Echo to Arduino pin SR04_EchoPin
  More info at: http://goo.gl/kJ8Gl
  
  Return the OPEN or CLOSED status of the Garage Door.
  0 for unknown.
  1 for Closed.
  2 for open.
  */
  
  int DoorStatus;
  int duration; 
  int distance;
  
  digitalWrite(SR04_TriggerPin, HIGH);
  delayMicroseconds(1000);
  digitalWrite(SR04_TriggerPin, LOW);
  
  duration = pulseIn(SR04_EchoPin, HIGH);
  distance = (duration/2) / 29.1;
  
  if ((distance > 0) && (distance <= DoorOpenDistance))
  {
    //Serial.println("Garage Door Open");
    
    DoorStatus = 2;
  }
  else
  {
    //Serial.println("Garage Door Closed");
    
    DoorStatus = 1;
  }
  
  // Serial.print(distance);
  // Serial.println(" cm");
  
  //delay(500);
  
  NextDoorStatusCheck = millis() + NextDoorCheckWaitTime;
  
  return DoorStatus;
}

/**************************************************************/
/**********  DS18B20 Temperature Sensor Functions  ************/
/**************************************************************/
void printTemperature(DeviceAddress deviceAddress)
{
  sensors.requestTemperatures();
  float tempC = sensors.getTempC(deviceAddress);
  if (tempC == -127.00) 
  {
    Serial.print("Temp: Error");
  } 
  else 
  {
    Serial.print("Temp: ");
    Serial.print(DallasTemperature::toFahrenheit(tempC));
    Serial.print("\n\r");
  }
}

/**************************************************************/
/********  DHT11 and DHT22 Humidity Sensor Functions  *********/
/**************************************************************/
void DHTprintTemperature(DHT dht)
{
  float tempC = dht.readTemperature();
  if (isnan(tempC)) 
  {
    Serial.print("Temp: Error");
  } 
  else 
  {
    Serial.print("Temp: ");
    Serial.print(DallasTemperature::toFahrenheit(tempC));
  }
  Serial.print("\n\r");
}

void DHTprintHumidity(DHT dht)
{
  float Humid = dht.readHumidity();
  if (isnan(Humid)) 
  {
    Serial.print("Humidity: Error");
  } 
  else 
  {
    Serial.print("Humidity: ");
    Serial.print(Humid);
    Serial.print("\n\r");
  }
}

/**************************************************************/
/****************  Motion Sensor Functions  *******************/
/**************************************************************/
void MotionDetected()
{
   LastMotion = millis();
}


