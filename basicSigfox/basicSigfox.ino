
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

typedef struct __attribute__ ((packed)) sigfox_message {
  uint8_t _battery ; 
  int16_t _temperature ; 
} SigfoxMessage;

// Stub for message which will be sent
SigfoxMessage msg;


//#define DEBUG


void setup() {
  Serial.begin(9600);

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



  analogReadResolution(10);//10bits
  analogReference(AR_INTERNAL1V0);//1.0V built-in reference

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
  
  int16_t temp1;
  readTemp(&temp1);
  /*

  String message;
  message += (char) readVcc();
  message += (char) highByte(temp1);
  Serial.print((char) highByte(temp1), HEX);
  message += (char) lowByte(temp1);
  Serial.print((char)lowByte(temp1), HEX);*/

  msg._battery = (char) readVcc();
  msg._temperature = temp1;

  
 
  delay(1000);
  REG_EIC_INTFLAG = EIC_INTFLAG_EXTINT1;//clear flag on INT1 to avoid multiple interrupts


#ifdef DEBUG
    delay(5000);
#else
    sendString();
    Serial.flush();
    LowPower.deepSleep(3600000);
#endif


 
}

byte readVcc (){
  // read the input on analog pin:
  int sensorValue = analogRead(ADC_BATTERY);

  float voltage = sensorValue * 0.299; // (1.0/(2^10-1))*(68+33)/33 *100

  #ifdef DEBUG
  Serial.print("Voltage(V): ");
  Serial.println(voltage/100.);
  #endif

  voltage = voltage - 200;
  if (voltage < 0) voltage = 0; //2.0V and below is 0
  if (voltage >= 255) voltage = 255; //4.55V and above is 255

Serial.println((byte) voltage);

  return (byte) voltage;
}

void readTemp(int16_t *degree){
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
 
 *degree = (int16_t) (temp*10.);

 
 #ifdef DEBUG
 Serial.print("Temperature is: ");
 Serial.println(*degree *0.1);
 #endif
}



void sendString() {
  // Start the module
  SigFox.begin();
  // Wait at least 30mS after first configuration (100mS before)
  delay(100);
  // Clears all pending interrupts
  SigFox.status();
  delay(1);

  // Remove EOL
  //str.trim();

  SigFox.beginPacket();
  //SigFox.print(str);
  SigFox.write((uint8_t*)&msg, sizeof(SigfoxMessage));

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
