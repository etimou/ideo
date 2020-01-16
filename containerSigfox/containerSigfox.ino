#include <SigFox.h>
#include <ArduinoLowPower.h>

#define calib  1.094771242//sigfox1 David
//#define calib  1.078431373//sigfox2 Etienne

//#define DEBUG

#define PWRSENSPIN 2
#define TRIGPIN 3
#define ECHOPIN 4


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
  analogReference(AR_INTERNAL1V0);

  LowPower.attachInterruptWakeup(RTC_ALARM_WAKEUP, alarmEvent0, CHANGE);
  
  delay(10000);
   
}

void loop()
{

  String message;
  message += (char) readVcc(calib);
  message += (char) readDistance(PWRSENSPIN, TRIGPIN, ECHOPIN);

  sendString(message);
  Serial.flush();
  LowPower.deepSleep(10800000);
  //delay(2000);

 
}

byte readVcc (float calibrationValue){
  // read the input on analog pin:
  int sensorValue = analogRead(ADC_BATTERY);

  float voltage = sensorValue * 0.0747401 * calibrationValue; //voltage in 1/100 Volt, 300 means 3.00V

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
