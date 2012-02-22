#include <stdint.h>
#include <enc28j60.h>
#include <EtherCard.h>
#include <net.h>

// Demo using DHCP and DNS to perform a web client request.
// 2011-06-08 <jc@wippler.nl> http://opensource.org/licenses/mit-license.php
// Adapted marginally by Libby Miller

// ethernet interface mac address, must be unique on the LAN
static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };

byte Ethernet::buffer[700];
static uint32_t timer;
static uint32_t last_blink_check;
static int blink_rate = 5000;

char website[] PROGMEM = "dev.example.com"; //domain - fix for the domain you are using

// called when the client request is complete
static void my_callback (byte status, word off, word len) {
   char *p = (char *)Ethernet::buffer + off;
   while (*p) {
            while (*p) if (*(p++) == '\n') break;
            if (*p == 0) break; // run out of buffer
            if (*(p++) == '\r') break;
   }
   if (*p) {
            p++; // skip \n
            char *buf = (char *)len;

            // we expect an int as a string
            // e.g. minutes since last mention
            // blinking tails off over time going to a base level

            int i = atoi(p);

            Serial.println(i);
            if(i>4){//mins since last
              blink_rate = 5000;
            }
            if(i==5){
              blink_rate = 4000;
            }
            if(i==4){
              blink_rate = 3000;
            }
            if(i==3){
              blink_rate = 2000;
            }
            if(i==2){
              blink_rate = 1000;
            }            
            if(i==1){
              blink_rate = 500;
            }  
            if(i<1){
              blink_rate = 200;
            } 
            Serial.println(blink_rate);

   }

}

void setup () {
  Serial.begin(57600);
  Serial.println("\n[webClient]");

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


// main loop 

void loop () {
  ether.packetLoop(ether.packetReceive());

  blink();

  // seems to be a race condition sometimes
  delay(100); 
  
  if (millis() > timer) {
    timer = millis() + 5000;
    Serial.println("req");
    // idea is you are retrieving http://dev.example.com/2012/01/nanode/results
    // goes to my_callback, which is expecting an int as a string, for example minutes since last mention
    ether.browseUrl(PSTR("/2012/01/nanode"), "results", website, my_callback);
  }
}


// blink is always the same length but happens at a rate determined by blink_rate

void blink(){
   //Serial.println("blink called");
   //Serial.println(millis());
   //Serial.println(last_blink_check + blink_rate);
   if(millis() > (last_blink_check + blink_rate)){
      //Serial.println("blinking!");
      digitalWrite(6, HIGH);   // set the LED on
      delay(500);              // wait for a second
      digitalWrite(6, LOW);    // set the LED off
      delay(500);              // wait for a second 
      last_blink_check = millis();
   }
     
}


