
#include <math.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>

/* Set these to your desired credentials */
const char *ssid = "wifi_name";  //ENTER YOUR WIFI SETTINGS 
const char *password = "wifi_pwd";

/* Set these values for force sensors */
const int FSR_PIN = A0; // Pin connected to FSR/resistor divider
const float VCC = 4.98; // Measured voltage of Ardunio 5V line
const float R_DIV = 3230.0; // Measured resistance of 3.3k resistor

/* Set these to connect with Thingspeak */
const char *host = "api.thingspeak.com";
const int channelID = 123456 ;
String apiKey = "XXXXXXXXXXXXXXXX";  //ENTER YOUR API KEY 
const int postingInterval = 10 * 1000; // post data every 20 seconds


//---------------------------------------------------------------------
void setup() {
      delay(1000);
      Serial.begin(115200);
    
      WiFi.mode(WIFI_STA);        //This line hides the viewing of ESP as wifi hotspot
      
      WiFi.begin(ssid, password);     //Connect to your WiFi router
      Serial.println("");
    
      Serial.print("Connecting");
      // Wait for connection
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
      }
    
      //If connection successful show IP address in serial monitor
      Serial.println("");
      Serial.print("Connected to ");
      Serial.println(ssid);
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());  //IP address assigned to your ESP
      Serial.begin(9600);
      pinMode(FSR_PIN, INPUT);      //Send sensor value to your ESP
}

//---------------------------------------------------------------------
void loop() {
  WiFiClient client;          
  const int httpPort = 80; //Port 80 is commonly used for www

//Connect to host, host(web site) is define at top 
     if(!client.connect(host, httpPort)){
       Serial.println("Connection Failed");
       delay(300);
       return; //Keep retrying until we get connected
     }
 
//---------------------------------------------------------------------
//Make GET request as pet HTTP GET Protocol format
// Measure the voltage at 5V and resistance of your 3.3k resistor
 
  float force;
  int fsrADC = analogRead(FSR_PIN);

// If the FSR(sensor) has no pressure, the resistance will be
// near infinite. So the voltage should be near 0.
  
      if (fsrADC != 0) // If the analog reading is non-zero
      {
  
// Use ADC reading to calculate voltage:

float fsrV = fsrADC * VCC / 1023.0;

// Use voltage and static resistor value to 
// calculate FSR resistance:

    float fsrR = R_DIV * (VCC / fsrV - 1.0);
    Serial.println("Resistance: " + String(fsrR) + " ohms");
    
// Guesstimate force based on slopes in figure 3 of
// FSR datasheet:

    float fsrG = 1.0 / fsrR; // Calculate conductance
    // Break parabolic curve down into two linear slopes:
    if (fsrR <= 600) 
      force = (fsrG - 0.00075) / 0.00000032639;
    else
      force =  fsrG / 0.000000642857;
    Serial.println("Force: " + String(force) + " g");
    Serial.println();

    delay(500);
  }
  else
  {
    // No pressure detected
  }

//---------------------------------------------------------------------
/* To send apikey TO Thingspeak*/
  String ADCData;
  
  ADCData = String(force);   //Integer to string conversion
  String Link="GET /update?api_key="+apiKey+"&field1=";  //Requeste webpage  
  Link = Link + ADCData;
  Link = Link + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n";                
  client.print(Link);
  delay(100);
  
//---------------------------------------------------------------------
//Wait for server to respond with timeout of 5 Seconds
  int timeout=0;
  while((!client.available()) && (timeout < 1000))     //Wait 5 seconds for data
   {
       delay(10);  //Use this with time out
       timeout++;
       }

//---------------------------------------------------------------------
//If data is available before time out read it.
 if(timeout < 500)
 {
     while(client.available()){
        Serial.println(client.readString()); //Response from ThingSpeak       
     }
 }
 else
 {
     Serial.println("Request timeout..");
 }

 delay(5000);  //Read Web Page every 5 seconds
}
//=======================================================================
