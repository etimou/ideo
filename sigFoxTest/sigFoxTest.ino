/*
  SigFox First Configuration

  This sketch demonstrates the usage of MKRFox1200 SigFox module.
  Since the board is designed with low power in mind, it depends directly on ArduinoLowPower library

  This example code is in the public domain.

device: "18E419"
time: "1572006834"
duplicate: "false"
snr: "14.97"
station: "0DD4"
data: "303130323033"
avgSnr: "16.27"
lat: "46.0"
lng: "3.0"
rssi: "-103.00"
seqNumber: "5"
deviceTypeId: "5db2b350c563d665b5d9d177"  
*/

union float2bytes { float f; char b[sizeof(float)]; };
 
float2bytes f2b;
 
#include <SigFox.h>
#include <ArduinoLowPower.h>
#include <TinyGPS.h>

TinyGPS gps;

void setup() {
  pinMode(7, INPUT_PULLUP);
  Serial1.begin(9600);
 

  // Uncomment this line and comment begin() if you are working with a custom board
  //if (!SigFox.begin(SPI1, 30, 31, 33, 28, LED_BUILTIN)) {
  SigFox.begin();
  // Enable debug led and disable automatic deep sleep
  // Comment this line when shipping your project :)
  SigFox.debug();

  delay(100);



   
}

void loop()
{
  while(digitalRead(7)){};

  
  bool newData = false;

  // For one second we parse GPS data and report some key values
  for (unsigned long start = millis(); millis() - start < 1000;)
  {
    while (Serial1.available())
    {
      char c = Serial1.read();
      // Serial.write(c); // uncomment this line if you want to see the GPS data flowing
      if (gps.encode(c)) // Did a new valid sentence come in?
        newData = true;
    }
  }

  float lat = 45.01;
  float lon = 3.01;

  if (newData)
  {
    gps.f_get_position(&lat, &lon);
  }

 
  String message;


  f2b.f = lat;
  for ( int i=sizeof(float)-1; i <sizeof(float) ; --i )
   message += f2b.b[i];
  f2b.f = lon;
  for ( int i=sizeof(float)-1; i <sizeof(float) ; --i )
   message += f2b.b[i];

 
/*
  Serial.println("Type the message to be sent");
  while (!Serial.available());

  
  while (Serial.available()) {
    message += (char)Serial.read();
  }

  // Every SigFox packet cannot exceed 12 bytes
  // If the string is longer, only the first 12 bytes will be sent

  if (message.length() > 12) {
    Serial.println("Message too long, only first 12 bytes will be sent");
  }
  */
  
  

  // Remove EOL
  message.trim();


 
    sendString(message);
    for (int i=0; i<60; i++)
    {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(1000);
      digitalWrite(LED_BUILTIN, LOW);
      delay(1000);
    }
    sendString(message);
    for (int i=0; i<60; i++)
    {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(1000);
      digitalWrite(LED_BUILTIN, LOW);
      delay(1000);
    }
    sendString(message);
  

  // Send the module to the deepest sleep
  SigFox.end();


}

void sendString(String str) {
  // Start the module
  SigFox.begin();
  // Wait at least 30mS after first configuration (100mS before)
  delay(100);
  // Clears all pending interrupts
  SigFox.status();
  delay(1);

  SigFox.beginPacket();
  SigFox.print(str);

  int ret = SigFox.endPacket();  // send buffer to SIGFOX network
  if (ret > 0) {
    Serial.println("No transmission");
  } else {
    Serial.println("Transmission ok");
  }

  Serial.println(SigFox.status(SIGFOX));
  Serial.println(SigFox.status(ATMEL));
  SigFox.end();
}

/*
void sendStringAndGetResponse(String str) {
  // Start the module
  SigFox.begin();
  // Wait at least 30mS after first configuration (100mS before)
  delay(100);
  // Clears all pending interrupts
  SigFox.status();
  delay(1);

  SigFox.beginPacket();
  SigFox.print(str);

  int ret = SigFox.endPacket(true);  // send buffer to SIGFOX network and wait for a response
  if (ret > 0) {
    Serial.println("No transmission");
  } else {
    Serial.println("Transmission ok");
  }

  Serial.println(SigFox.status(SIGFOX));
  Serial.println(SigFox.status(ATMEL));

  if (SigFox.parsePacket()) {
    Serial.println("Response from server:");
    while (SigFox.available()) {
      Serial.print("0x");
      Serial.println(SigFox.read(), HEX);
    }
  } else {
    Serial.println("Could not get any response from the server");
    Serial.println("Check the SigFox coverage in your area");
    Serial.println("If you are indoor, check the 20dB coverage or move near a window");
  }
  Serial.println();

  SigFox.end();
}*/
