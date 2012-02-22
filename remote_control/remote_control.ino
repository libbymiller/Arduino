
#include <stdint.h>
#include <enc28j60.h>
#include <EtherCard.h>
#include <net.h>

// ethernet interface mac address, must be unique on the LAN
static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };

byte Ethernet::buffer[700];
static uint32_t timer;
char website[] PROGMEM = "dev.example.com";

int potPin = 3;    // select the input pin for the potentiometer
int ledPin = 6;   // select the pin for the LED
int val = 0;       // variable to store the value coming from the sensor
int buttonState = 0; 
const int buttonPin = 2; 


static void my_callback (byte status, word off, word len) {
  Serial.println(">>>");
  Ethernet::buffer[off+300] = 0;
  Serial.print((const char*) Ethernet::buffer + off);
  Serial.println("...");
}

void setup() {
  Serial.begin(57600);
  Serial.println("\n[button-thingy]");
  pinMode(ledPin, OUTPUT);  // declare the ledPin as an OUTPUT
  pinMode(buttonPin, INPUT); 

  if (ether.begin(sizeof Ethernet::buffer, mymac) == 0) 
    Serial.println( "Failed to access Ethernet controller");
  if (!ether.dhcpSetup())
    Serial.println("DHCP failed");
  
  ether.printIp("IP:  ", ether.myip);
  ether.printIp("GW:  ", ether.gwip);  
  ether.printIp("DNS: ", ether.dnsip);  

  if (!ether.dnsLookup(website)){
    Serial.println("DNS failed");
    ether.hisip[0] = 46;  
    ether.hisip[1] = 137;
    ether.hisip[2] = 5;
    ether.hisip[3] = 187;
  }
  ether.printIp("SRV: ", ether.hisip);

}

void loop() {
  ether.packetLoop(ether.packetReceive());
  val = analogRead(potPin);   // read the value from the sensor
  //Serial.println(val);
  int genre = 0;

  
  if(val<5){
     genre = 1;
//    Serial.println("drama");
  }
  if(val>5 && val<250){
     genre = 2;
//    Serial.println("history");    
  }
  if(val> 250 && val<600){
//      Serial.println("politics");   
     genre = 3;
  }
  if(val>600 && val<1000){
//      Serial.println("science");   
     genre = 4;
  } 
  if(val>1000){
//      Serial.println("comedy");    
     genre = 5;
  }

  buttonState = digitalRead(buttonPin);


  if (buttonState == HIGH) {    
    digitalWrite(ledPin, HIGH); 

    Serial.println(val);

    char room[5];
    char buf[2];
    
    itoa(9999,room,10);
    itoa(genre,buf,10);
     
    char cbuff[22];

    strcpy (cbuff,"?room=");
    strcat (cbuff,room);
    strcat (cbuff,"&genre=");
    strcat (cbuff,buf);
    strcat (cbuff,"&");
    Serial.println(cbuff);

    ether.browseUrl(PSTR("/2012/01/remote/api/xmpp"), cbuff, website, my_callback);
    delay(5000);  
  }
  else {
    //Serial.println("low");

    digitalWrite(ledPin, LOW);
  }
}
