#include <ESP8266WiFi.h>
#include <WiFiClient.h> 

// Adapted from: https://gist.github.com/jaretburkett/21e3b918d1df1ac296fc
//how many clients should be able to telnet to this ESP8266 
#define MAX_SRV_CLIENTS 3

/* Set these to your desired credentials. */
const char *ssid = "ALL YOUR PASSWORDS"; // Broadcasts a secure wifi access point with this name.
const char *password = "password";
/* In this case unless you hook this up to a computer it does nothing even if you can guess the password unless uart serial is hooked up */

WiFiServer server(23);
WiFiClient serverClients[MAX_SRV_CLIENTS];


int led = 5;

void setup() {
  delay(1000);
  Serial.begin(115200);
  /* You can remove the password parameter if you want the AP to be open. */
  WiFi.softAP(ssid, password);

  IPAddress myIP = WiFi.softAPIP();
 
 // start telnet server
  server.begin();
  server.setNoDelay(true);
}

void loop() {
  uint8_t i;
  if(server.hasClient()){
    digitalWrite(led, HIGH);
  } else{
    digitalWrite(led, LOW);
  }
  //check if there are any new clients
  if (server.hasClient()){
    for(i = 0; i < MAX_SRV_CLIENTS; i++){
      //find free/disconnected spot
      if (!serverClients[i] || !serverClients[i].connected()){
        if(serverClients[i]) serverClients[i].stop();
        serverClients[i] = server.available();
        continue;
      }
    }
    //no free/disconnected spot so reject
    WiFiClient serverClient = server.available();
    serverClient.stop();
  }
  //check clients for data
  for(i = 0; i < MAX_SRV_CLIENTS; i++){
    if (serverClients[i] && serverClients[i].connected()){
      if(serverClients[i].available()){
        //get data from the telnet client and push it to the UART
        while(serverClients[i].available()) Serial.write(serverClients[i].read());
      }
    }
  }
  //check UART for data
  if(Serial.available()){
    size_t len = Serial.available();
    uint8_t sbuf[len];
    Serial.readBytes(sbuf, len);
    //push UART data to all connected telnet clients
    for(i = 0; i < MAX_SRV_CLIENTS; i++){
      if (serverClients[i] && serverClients[i].connected()){
        serverClients[i].write(sbuf, len);
        delay(1);
      }
    }
  }
}
