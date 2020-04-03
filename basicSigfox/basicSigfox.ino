#include <SigFox.h>
#include <ArduinoLowPower.h>

// First we include the libraries
#include <OneWire.h> 
#include <DallasTemperature.h>
/********************************************************************/
// Data wire is plugged into pin 10 on the Arduino 
#define ONE_WIRE_BUS 10
/********************************************************************/
// Setup a oneWire instance to communicate with any OneWire devices  
// (not just Maxim/Dallas temperature ICs) 
OneWire oneWire(ONE_WIRE_BUS); 
/********************************************************************/
// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);


//#define DEBUG


void setup() {
  Serial1.begin(9600);

  sensors.begin(); 
  delay(100);
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


  analogReadResolution(12);//12bits
  analogReference(AR_INTERNAL1V65);

  pinMode(1, INPUT_PULLUP);
  //LowPower.attachInterruptWakeup(RTC_ALARM_WAKEUP, alarmEvent0, CHANGE);
  LowPower.attachInterruptWakeup(1, alarmEvent1, FALLING);

  pinMode(LED_BUILTIN, OUTPUT);

  delay(10000);
   
}

void loop()
{
  #ifdef DEBUG
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
  #endif
  
  byte temp1, temp2;
  readTemp(&temp1, &temp2);
  

  String message;
  message += (char) readVcc();
  message += (char) temp1;
  message += (char) temp2;

  delay(1000);
  REG_EIC_INTFLAG = EIC_INTFLAG_EXTINT1;//clear flag on INT1 to avoid multiple interrupts


#ifdef DEBUG
    LowPower.deepSleep(5000);
#else
    sendString(message);
    Serial.flush();
    LowPower.deepSleep(3600000);
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

void readTemp(byte *degree, byte *decimals){
   // call sensors.requestTemperatures() to issue a global temperature 
 // request to all devices on the bus 
/********************************************************************/
 sensors.begin();

 sensors.requestTemperatures(); // Send the command to get temperature readings 

/********************************************************************/
 
 float temp = sensors.getTempCByIndex(0);

 if ((temp<-30)||(temp>50)){
   sensors.begin();
   sensors.requestTemperatures();
   temp = sensors.getTempCByIndex(0);
    
 }
 
 *degree = (byte)((int) temp + 100);
 *decimals = (byte)((temp*100) - (int)(temp*100));
 
 #ifdef DEBUG
 Serial.print("Temperature is: ");
 Serial.print(*degree -100);
 Serial.print(".");
 Serial.println(*decimals);
 #endif
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
void alarmEvent1() {
}
