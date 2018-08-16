/*
 * Fake beacon frames for ESP8266 using the Arduino IDE
 *
 * Based on https://github.com/markszabo/FakeBeaconESP8266 which is 
 *  - Based on the WiFiBeaconJam by kripthor (https://github.com/kripthor/WiFiBeaconJam) 
 * 
 * More info: http://nomartini-noparty.blogspot.com/2016/07/esp8266-and-beacon-frames.html
  more about beacon frames https://mrncciew.com/2014/10/08/802-11-mgmt-beacon-frame/

 */

#include <ESP8266WiFi.h> 

extern "C" {
  #include "user_interface.h"
}

bool ledState = false;
uint32_t currentTime = 0;
uint32_t attackTime = 0;
uint32_t packetRateTime = 0;
byte channel = 1;
  
void setup() {
  delay(500);
  system_update_cpu_freq(160);
  currentTime = millis();
  wifi_set_opmode(STATION_MODE);
  wifi_promiscuous_enable(1); 
  pinMode(BUILTIN_LED, OUTPUT); 
  randomSeed(os_random());
}



void loop() {
   currentTime = millis();

     // Randomize channel for this startup run session//
  channel = genSafishChannel();// 
    
  //snBcn("test"); //sends beacon frames with the SSID 'test'
  //sendRandomBeacon(10); //sends beacon frames with 10 character long random SSID
  //sendFuzzedBeacon("test",10); //sends beacon frames with 10 different SSID all starting with 'test' and ending with whitespaces (spaces and/or tabs)
  Beacon8r();
    if(ledState){
      digitalWrite(BUILTIN_LED,HIGH);
      ledState = false;
    }else{
      digitalWrite(BUILTIN_LED,LOW);
      ledState = true;
    }
  
}

void sendFuzzedBeacon(char* baseSsid, int nr) {
  int baseLen = strlen(baseSsid);
  int i=0;
  for(int j=0; j < 32 - baseLen; j++) { //32 is the maximum length of the SSID
    for(int k=0; k < pow(2,j); k++) {
      int kk = k;
      String ssid = baseSsid;
      for(int l=0; l < j; l++) {
        if(kk%2 == 1) ssid += " "; //add a space
        else ssid += "\t"; //add a tab
        kk /= 2;
      }
      char charBufSsid[33];
      ssid.toCharArray(charBufSsid, 33);
      snBcn(charBufSsid);
      if(++i >= nr) return; 
    }
  }
}

void sendRandomBeacon(int len) {
  char ssid[len+1];
  randomString(len, ssid);
  snBcn(ssid);
}

void randomString(int len, char* ssid) {
  String alfa = "1234567890qwertyuiopasdfghjkklzxcvbnm QWERTYUIOPASDFGHJKLZXCVBNM_";
  for(int i = 0; i < len; i++) {
    ssid[i] = alfa[random(65)];
  }
}

void randomNumString(int len, char* ssid) {
  String alfa = "1234567890";
  for(int i = 0; i < len; i++) {
    ssid[i] = alfa[random(9)];
  }
}

int genSafishChannel(){
    int chan =  random(1,3);
    if(chan ==1)
      return 1;  
    if(chan ==2)
      return 6;
    if(chan ==3)
      return 11;

    return 1;  
}

void snBcn(char* ssid){  
    
    int sflen = 2;
    char suffixssid[sflen+1];
    randomNumString(sflen, suffixssid);
    
    int oglen = strlen(ssid);
    char result[oglen+sflen+1]; 
    strcpy(result, ssid);
    strcat(result, suffixssid);
    
    sendSuffBcn(result);
}

void sendSuffBcn  (char* ssid) {  
  
   currentTime = millis(); //gets current time
   
  
    wifi_set_channel(channel);

    uint8_t packet[128] = { 0x80, 0x00, //Frame Control 
                        0x230, 0x020, //Duration
                /*4*/   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, //Destination address 
                /*10*/  0x01, 0x02, 0x03, 0x04, 0x05, 0x06, //Source address - overwritten later
                /*16*/  0x01, 0x02, 0x03, 0x04, 0x05, 0x06, //BSSID - overwritten to the same as the source address
                /*22*/  0xc0, 0x6c, //Seq-ctl
                //Frame body starts here
                /*24*/  0x83, 0x51, 0xf7, 0x8f, 0x0f, 0x00, 0x00, 0x00, //timestamp - the number of microseconds the AP has been active
                /*32*/  0xFF, 0xA28, //Beacon interval  stock was: 0xFF, 0x00,
                /*34*/  0x01, 0x04, //Capability info
                /* SSID */
                /*36*/  0x00
                };

    int ssidLen = strlen(ssid);
    packet[37] = ssidLen;

    for(int i = 0; i < ssidLen; i++) {
      packet[38+i] = ssid[i];
    }

    uint8_t postSSID[13] = {0x01, 0x08, 0x82, 0x84, 0x8b, 0x96, 0x24, 0x30, 0x48, 0x6c, //supported rate
                        0x03, 0x01, 0x04 /*DSSS (Current Channel)*/ };

    for(int i = 0; i < 12; i++) {
      packet[38 + ssidLen + i] = postSSID[i];
    }

    packet[50 + ssidLen] = channel;

    // Randomize SRC MAC
    packet[10] = packet[16] = random(256);
    packet[11] = packet[17] = random(256);
    packet[12] = packet[18] = random(256);
    packet[13] = packet[19] = random(256);
    packet[14] = packet[20] = random(256);
    packet[15] = packet[21] = random(256);


 /* packet[15] = packet[21]= packet[10] = packet[16] = random(256);
  packet[13] = packet[19] =  packet[11] = packet[17] = random(256);
  packet[14] = packet[20] =   packet[12] = packet[18] = random(256);
   */ 
  
 
packet[24] = currentTime; //sets current time so packets show up on wireshark

    int packetSize = 51 + ssidLen;

     bool sent = false; 
     int packySent = 0;
    //delay(1);
    for(int k=0;k<250 && !sent && packySent <2 ;k++){
        sent = wifi_send_pkt_freedom(packet, packetSize, 0) == 0;
        if(sent){
          packySent++;
        }else{
          delay(1);
        }
      }
}

void Beacon8r() {

   /* // each of these sections runs good on one esp8266 for being visible on a phone's wifi.
  snBcn("Moose and Squirrel ");
  snBcn("Boris and Natasha");
  snBcn("FBI Party Van 667");
  snBcn("Mr Fusion Center");
  snBcn("Dr Oh No");
  snBcn("Potato Internet");
  snBcn("UberPatriot");
  snBcn("TacoEjército");
  snBcn("TaterTots");
  snBcn("TrevorForget");
  snBcn("LemonParty");
  snBcn("VelcroParty");
  snBcn("PickleRick!");
  snBcn("ShinyAndChrome");
  snBcn("WhoIsTrevorGault");
  snBcn("SovereignTrevor");
  snBcn("TheSpyWhoTrevoredMe");
  snBcn("Full Metal Trevor");
  snBcn("Show me the Trevor");
  snBcn("KraftTrevor");
 
  snBcn("Top Trevor");
  snBcn("Trevor Tank");
  snBcn("The Dark Trevor");
  snBcn("Trevor 500");
  snBcn("Terminator 8: The Trevorning");
  snBcn("Weekend at Trevor's");
  snBcn("Saving Private Trevor");
  snBcn("Going Full Trevor");
  snBcn("Soul Trevor");
  snBcn("Trevor Force 6");
  snBcn("The Bold and the Trevor");
  snBcn("Running Trevor");
  snBcn("Trevor the explorer");
  snBcn("BioTrevor 2000");
  snBcn("Trevor goes to Whitecastle");
  snBcn("Sir Trevor's tale");
  snBcn("TrevNasty -The superstar years");
  snBcn("Raising Trevor");
  snBcn("TrevorStarter");
  snBcn("Church of Trevorology");
  snBcn("The Last Trevor");
  snBcn("Blue Trevor");
  snBcn("Generation Trevor");
  snBcn("Fire Team Trevor");
  */
  snBcn("Trevor Party!");
  snBcn("Ghost Trevor Killah");
  snBcn("Trevor 2000");
  snBcn("Cooking with Trevor");
  snBcn("Trevor's Home Garden");
  snBcn("I kissed a Trevor");
  snBcn("Trevor <3's Grifter for3ver");
  snBcn("Too many Trevors ");
  snBcn("Teenage Mutant Ninja Trevors ");
  snBcn("Trevor Memorial Hotel");
  snBcn("Trev-Nasty");
  snBcn("Trevor At Large");
  snBcn("Fresh Trevor of Bel-Air");
  snBcn("Tinker Tailor Soldier Trevor");
  snBcn("Attack of the 50 foot Trevor");
  snBcn("Godzilla Vs Trevor");
  snBcn("Hack for Trevor");
  snBcn("Aliens vs. Trevor");
  snBcn("Kindergarten Trevor");
  snBcn("What Trevor Wants ");
  snBcn("The way of the Trevor");
  snBcn("Trevor Air");
  snBcn("Leaving Trevor ");
  snBcn("The hardest Trevor");
  snBcn("Trevor Kong");
  snBcn("Le Trevor Noir");
  snBcn("Bridge over the River Trevor");
  snBcn("Breakfast of Trevors");
  snBcn("My milkshake brings all the Trevors");
 // */
  }
