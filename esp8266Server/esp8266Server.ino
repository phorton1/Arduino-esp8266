//  A pass through Serial Over Wifi Server/Client scheme
//
//     The idea is that the esp01 module can be programmed and then
//     used as 1/2 of a communications link via serial data presented
//     to and received from it's TX and RX pins.
//
//     The scheme can use UDP or TCP/IP.
//     UDP is faster but with no connection checking
//        and always sends a character at a time.
//     TCP is secure but slower, and so typically
//        uses a line buffer for "packets" but
//        can also be configured for single characters.
//
//
// This code is loaded directly to the ESP01-ESP2866 board.
// See /zip/Arduino/ESP8266/_flash for more info.
//
// - Arduino Board Manager must contain "Community ESP8266" board set
// - Use first generic ESP8266 board in Arduino IDE
//
// To compile this program on the teensy2Esp8266 interface breadboard,
// compile that program to the teensy 4.0, run it in a console, and
// press 'P' for program to put the esp8266 in program mode and act
// as a serial port to the device.
//
// To program with FTDI Serial Board
//
// 1. Arduino Board Manager must contain "Community ESP8266" board set
// 2. Use first generic ESP8266 board and FDTI serial board port (4)
// 3. Press both buttons
// 4. Release the right "reset" button keeping left "program" button depressed
//    then release the left program button.
// 5. Program will upload.
// 6. press the Reset (right) button as needed.

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiServer.h>

#define HOST_BAUD_RATE     115200

#define WITH_SERIAL_STARTUP    1
    // displays serial data at startup that *should* be ignored
#define WITH_SERIAL_DEBUG      1
    // shows serial data during connections that *should* be ignored
#define WITH_WELCOME_MESSAGE   1

#define SSID  "THX37"
#define PASS  "prh87924"

WiFiServer server(80);
WiFiClient client;


void setup(void)
{
    pinMode(0,INPUT);
    pinMode(2,INPUT);

    Serial.begin(HOST_BAUD_RATE);
    delay(500);

    #if WITH_SERIAL_STARTUP
        delay(1500);
        Serial.println("");
        Serial.println("esp8266Server version 1.1 starting");
        Serial.print("Coonnecting to ");
        Serial.println(SSID);
    #endif


    WiFi.mode(WIFI_STA);
    WiFi.begin(SSID, PASS);

    // Wait for connection

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        #if WITH_SERIAL_STARTUP
            Serial.print(".");
        #endif
    }

    #if WITH_SERIAL_STARTUP
        Serial.println();
        Serial.println("Wifi Started");
        Serial.print("Connected to ");
        Serial.println(SSID);
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
    #endif

    delay(500);
    server.begin();
    #if WITH_SERIAL_STARTUP
        delay(500);
        Serial.println("esp8266Server ready");
            // teensy can wait for this line to be ready
    #endif
}




void loop(void)
{
    if (client && !client.connected())
    {
        // this never happens
        #if WITH_SERIAL_DEBUG
            Serial.println("CLIENT DISCONNECTED");
        #endif
    }

    if (!client || !client.connected())
    {
        client = server.available();
        if (client)
        {
            #if WITH_SERIAL_DEBUG
                Serial.println("CLIENT CONNECTED");
                    // teensy can ignore this line I suppose
            #endif
            #if WITH_WELCOME_MESSAGE
                client.println("esp8266Server");
                    // windows client could ignore this line I suppose
            #endif
        }
    }
    if (client.connected())
    {
        if (client.available())
        {
            char c = client.read();
            Serial.write(c);
        }

        if (Serial.available())
        {
            char c = Serial.read();
            client.write(c);
        }
    }
}
