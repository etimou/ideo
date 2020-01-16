/**
 * Running a periodic communication over LoRaWan using a LoRa Radio Node v1.0 Board
 * This demo is using a synchronous LMIC wrapper to simplify reading and understanding
 * for beginers. 
 * You need to setup you LoRaWan configuration in the LoraWan.cpp file before beeing able to
 * register on TTN network or any other Network.
 * 
 * Dependencies:
 * - LMIC LoRaWan stack (find it in library manager)
 * - LowPower 
 * 
 * Documentation:
 * - LMIC need a little setup, please read:
 *   https://www.disk91.com/2019/technology/lora/hoperf-rfm95-and-arduino-a-low-cost-lorawan-solution/
 * - To get more information about this board, please read: 
 *   https://www.disk91.com/2019/technology/lora/first-steps-with-lora-radio-node-arduino/
 * 
 */


#include "LowPower.h"
#include "loraWan.h"
#include <EEPROM.h>

unsigned long calib = 0;
// defines pins numbers
const int trigPin = 3;
const int echoPin = 4;
const int pwrSensPin = A0;


static uint8_t mydata[] = {0x01, 0x02, 0, 0, 0x02, 0x02, 0, 0};


void setup() {
    Serial.begin(9600);
    Serial.println(F("Starting"));

    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(pwrSensPin, OUTPUT);
    pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
    pinMode(echoPin, INPUT); // Sets the echoPin as an Input
    digitalWrite(LED_BUILTIN, LOW);
    digitalWrite(pwrSensPin, LOW);

    //get the calibration value
    EEPROM.get(1, calib);

    lorawan_setup();
}

void loop() {

    readDistance();
    readVcc();


    for (int i = 0; i < 8; i++) {
    Serial.print(mydata[i], HEX);
    Serial.print(" ");
    }
    Serial.println();
    Serial.flush();
  
    lorawan_send(1, mydata, 8, false, NULL, NULL, NULL);
    Serial.flush();
    for (int i=0; i<800; i++) {
      LowPower.powerDown(SLEEP_8S,ADC_OFF, BOD_OFF);
    }

}

void readVcc() {
  // Read 1.1V reference against AVcc
  // set the reference to Vcc and the measurement to the internal 1.1V reference
#if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
#elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
  ADMUX = _BV(MUX5) | _BV(MUX0);
#elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
  ADMUX = _BV(MUX3) | _BV(MUX2);
#else
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
#endif

  delay(70); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA, ADSC)); // measuring

  uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH
  uint8_t high = ADCH; // unlocks both

  long result = (high << 8) | low;

  result = 0.1 * calib / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000 //ok for proto w/ intermediate PCB

  mydata[6] = (uint8_t)(result >> 8);
  mydata[7] = (uint8_t)result;

  Serial.print("Battery(V): ");
  Serial.println(result*0.01);
  
}
void readDistance() {
  // defines variables
  long duration;
  int distance;

  digitalWrite(pwrSensPin, HIGH);//power up the sensor
  delay(100);

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
  // Prints the distance on the Serial Monitor
  Serial.print("Distance(cm): ");
  Serial.println(distance);

  mydata[2] = (uint8_t)(distance >> 8);
  mydata[3] = (uint8_t)distance;

  digitalWrite(pwrSensPin, LOW);//power down the sensor
}
