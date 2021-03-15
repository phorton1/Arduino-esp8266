#include <ESP8266WiFi.h>

// To compile this program on the teensy2Esp8266 interface breadboard,
// compile that program to the teensy 4.0, run it in a console, and
// press 'P' for program to put the esp8266 in program mode and act
// as a serial port to the device.

#define WITH_ACCEL   0
#define WITH_SERIAL  1

// This code is loaded directly to the ESP01-ESP2866 board.
//
// Getting the code to the ESP01 can be done with an FTDI RS232 (breadboard),
// as seen in the Pics of the old adxl345 board, or via teensy2Esp8266 with
// a different breadboard (running the console program).
//
//-----------------------------------------------------
// INSTRUCTIONS
//-----------------------------------------------------
// 1. Arduino Board Manager must contain "Community ESP8266" board set
// 2. Use first generic ESP8266 board and FDTI serial board port (4)
// 3. Press both buttons
// 4. Release the right "reset" button keeping left "program" button depressed
//    then release the left program button.
// 5. Program will upload.
// 6. press the Reset (right) button as needed.
//
// See /zip/Arduino/ESP8266/_flash for more info.
// and pics and notes there.


#define SSID  "THX36"
#define PASS  "prh87924"


#define USE_DNS  0
#define SEND_COMPRESSED_INTS  0

#include <WiFiClient.h>
#include <WiFiServer.h>

#if USE_DNS
    #include <ESP8266mDNS.h>
#endif

WiFiServer server(80);
WiFiClient client;

#if WITH_ACCEL
    extern void initAccel();
    extern void readAccel(short *fx, short *fy, short *fz);
#endif



void setup(void)
{
    pinMode(0,INPUT);
    pinMode(2,INPUT);

    #if WITH_SERIAL
        Serial.begin(115200);
        delay(3500);
        Serial.println("");
        Serial.println("esp8266-adxl345 version 1.2 test program started");
    #endif

    WiFi.mode(WIFI_STA);
    WiFi.begin(SSID, PASS);

    // Wait for connection

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        #if WITH_SERIAL
            Serial.print(".");
        #endif
    }

    #if WITH_SERIAL
        Serial.println();
        Serial.println("Wifi Started");
        Serial.print("Connected to ");
        Serial.println(SSID);
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
    #endif

    #if USE_DNS
        if (MDNS.begin("prh_esp8266_001"))
        {
            delay(600);
            #if WITH_SERIAL
                Serial.println("MDNS prh_esp8266_001 started");
            #endif
        }
    #endif

    #if USE_UDP
        udp.begin(80);
        delay(600);
        #if WITH_SERIAL
           Serial.println("UDP started");
        #endif
    #else
        server.begin();
        delay(600);
        #if WITH_SERIAL
            Serial.println("WiFiServer started");
        #endif
    #endif

    #if WITH_ACCEL
        delay(1000);
        initAccel();
    #endif

    #if WITH_SERIAL
        Serial.println("Accel started ..");
        delay(1000);
        Serial.println("Test program started");
    #endif
}

#if WITH_ACCEL
    void sendAccel()
    {
        short x,y,z;
        readAccel(&x,&y,&z);

        #if WITH_SERIAL
            Serial.print("sendAccel(");
            Serial.print(x);
            Serial.print(",");
            Serial.print(y);
            Serial.print(",");
            Serial.print(z);
            Serial.println(")");
        #endif

        #if SEND_COMPRESSED_INTS
            char buf[7];
            memcpy(&buf[0],&x,2);
            memcpy(&buf[2],&y,2);
            memcpy(&buf[4],&z,2);
            buf[6] = '\n';
            client.write(buf,7);
        #else
            client.print("a(");
            client.print(x);
            client.print(",");
            client.print(y);
            client.print(",");
            client.print(z);
            client.println(")");
        #endif
    }
#endif



void loop(void)
{
    if (client && !client.connected())
    {
        #if WITH_SERIAL
            Serial.println("CLIENT DISCONNECTED");
        #endif
    }
    if (!client || !client.connected())
    {
        client = server.available();
        if (client)
        {
            #if WITH_SERIAL
                Serial.println("CLIENT CONNECTED");
            #endif
            client.println("welcome to the ESP8266-ADXL345 Server version 1.1");
        }
    }
    if (client.connected())
    {
        if (client.available())
        {
            char command = client.read();
            #if WITH_SERIAL
                Serial.print("command=");
                Serial.write(command);
                Serial.println();
            #endif

            if (command == 'a')
            {
                #if WITH_ACCEL
                    sendAccel();
                #else
                    static int counter = 237;
                    #if WITH_SERIAL
                        Serial.print("sendA(");
                        Serial.print(counter);
                        Serial.println(")");
                    #endif
                    client.print("a(");
                    client.print(counter);
                    client.println(")");
                    counter++;
                #endif
            }
        }

        if (Serial.available())
        {
            char c = Serial.read();
            client.write(c);
        }
    }
}
