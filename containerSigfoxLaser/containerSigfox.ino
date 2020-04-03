#include <SigFox.h>
#include <ArduinoLowPower.h>
#include <Wire.h>
#include <VL53L0X.h>

VL53L0X sensor;


//#define DEBUG

#define PWRSENSPIN 2
#define TRIGPIN 3
#define ECHOPIN 4
#define LONG_RANGE
//#define HIGH_SPEED
#define HIGH_ACCURACY


void setup() {
  Serial1.begin(9600);
  // Uncomment this line and comment begin() if you are working with a custom board
  //if (!SigFox.begin(SPI1, 30, 31, 33, 28, LED_BUILTIN)) {
  SigFox.begin();
  // Enable debug led and disable automatic deep sleep
  // Comment this line when shipping your project :)
  #ifdef DEBUG
  SigFox.debug();
  #endif
  
  delay(100);
  SigFox.end();

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PWRSENSPIN, OUTPUT);
  pinMode(TRIGPIN, OUTPUT); // Sets the trigPin as an Output
  pinMode(ECHOPIN, INPUT); // Sets the echoPin as an Input
  
  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(PWRSENSPIN, LOW);
  digitalWrite(TRIGPIN, LOW);

  analogReadResolution(12);//12bits
  analogReference(AR_INTERNAL1V65);

  LowPower.attachInterruptWakeup(RTC_ALARM_WAKEUP, alarmEvent0, CHANGE);


  Wire.begin();
  sensor.setTimeout(500);
  if (!sensor.init())
  {
    Serial.println("Failed to detect and initialize sensor!");
    //while (1) {}
  }

#if defined LONG_RANGE
  // lower the return signal rate limit (default is 0.25 MCPS)
  sensor.setSignalRateLimit(0.1);
  // increase laser pulse periods (defaults are 14 and 10 PCLKs)
  sensor.setVcselPulsePeriod(VL53L0X::VcselPeriodPreRange, 18);
  sensor.setVcselPulsePeriod(VL53L0X::VcselPeriodFinalRange, 14);
#endif

#if defined HIGH_SPEED
  // reduce timing budget to 20 ms (default is about 33 ms)
  sensor.setMeasurementTimingBudget(20000);
#elif defined HIGH_ACCURACY
  // increase timing budget to 200 ms
  sensor.setMeasurementTimingBudget(200000);
#endif
 
  
  delay(10000);
   
}

void loop()
{

  String message;
  message += (char) readVcc();
  message += (char) readDistance(PWRSENSPIN, TRIGPIN, ECHOPIN);
  message += (char) readDistanceLaser();

#ifdef DEBUG
    delay(2000);
#else
    sendString(message);
    Serial.flush();
    LowPower.deepSleep(10800000);
#endif


 
}

byte readVcc (){
  // read the input on analog pin:
  int sensorValue = analogRead(ADC_BATTERY);
  

  float voltage = sensorValue * 0.123321123; //voltage in 1/100 Volt, 300 means 3.00V
  //(165 / 4095)  / (33/(68+33))

  #ifdef DEBUG
  Serial.print("Voltage(V): ");
  Serial.println(voltage/100.);
  #endif

  voltage = voltage - 200;
  if (voltage < 0) voltage = 0; //2.0V and below is 0
  if (voltage >= 255) voltage = 255; //4.55V and above is 255

  return (byte) voltage;
}

byte readDistance(int pwrSensPin, int trigPin, int echoPin) {
  // defines variables
  long duration;
  int distance;

  digitalWrite(pwrSensPin, HIGH);//power up the sensor
  delay(200);//wait to stabilize (it was 100)

  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  // Calculating the distance
  distance = (int) (duration * 0.017);

  #ifdef DEBUG
  // Prints the distance on the Serial Monitor
  Serial.print("Distance(cm): ");
  Serial.println(distance);
  #endif

  digitalWrite(pwrSensPin, LOW);//power down the sensor
  
  if (distance >= 255) distance = 255; //byte limit is 255

  return (byte) distance;
}

byte readDistanceLaser(){

  sensor.init();
  
  int distance = sensor.readRangeSingleMillimeters()*0.1;

  #ifdef DEBUG
  // Prints the distance on the Serial Monitor
  Serial.print("Distance Laser(cm): ");
  Serial.println(distance);
  #endif
  
  if (distance >= 255) distance = 255; //byte limit is 255

  return (byte) distance;
}

void sendString(String str) {
  // Start the module
  SigFox.begin();
  // Wait at least 30mS after first configuration (100mS before)
  delay(100);
  // Clears all pending interrupts
  SigFox.status();
  delay(1);

  // Remove EOL
  str.trim();

  SigFox.beginPacket();
  SigFox.print(str);

  int ret = SigFox.endPacket();  // send buffer to SIGFOX network

  #ifdef DEBUG
  if (ret > 0) {
    Serial.println("No transmission");
  } else {
    Serial.println("Transmission ok");
  }
  Serial.println(SigFox.status(SIGFOX));
  Serial.println(SigFox.status(ATMEL));
  #endif

  // Send the module to the deepest sleep
  SigFox.end();
}
void alarmEvent0() {
}
