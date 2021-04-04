// An ESP8266 pass through Serial Over Wifi Server
//
// This code is loaded directly to the ESP01-ESP2866 board.
//
// - Arduino Board Manager must contain "Community ESP8266" board set
// - Use first generic ESP8266 board in Arduino IDE
//
// To compile this program via the teensy2Esp8266 program and circuit,
// compile that program to the teensy 4.0, run it in a console, and
// press 'ctrl-P' for program to put the esp8266 in program mode and act
// as a serial port to the device.  Then compile this program and have
// the IDE send it to the teensy port.
//
// The pound character '#' is a special command delimiter that is
// not passed through this server.  It is caught and used by the
// esp8266.  The commands are:
//
//            #HOST 0/1         (you can turn host mode on and off)
//            #HOST_ID zzzzzz   (you can override the default host_id)
//            #HOST_PASS        (you can override the default host password)
//            #SSID xxxxxx      (you must provide the SSID and PASS at least once)
//            #PASS yyyyyy
//            #RESET            clear EEPROM to defaults
//
// When modified with the above commands the above settings are retained
// in the esp8266 EEPROM for subsequent reboots.


#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiServer.h>

#define BUFFER_WIFI_XMIT    1
    // instead of char by char

#define HOST_BAUD_RATE   78440 // 11520
    // the baud rate of 78440 is specifically chosen to allow
    // teensyEsp8266.ino to not need to change baud rates to
    // swicth in and out of programming mode.

#define DEFAULT_HOST_MODE  1
#define DEFAULT_AP_IP      "192.168.4.1"
#define DEFAULT_HOST_ID    "exp8266Server"
#define DEFAULT_HOST_PASS  "exp13579"


#define WITH_SERIAL_STARTUP    0
    // Displays serial data at startup confirming
    // the esp8266 code is running ok.
#define WITH_WELCOME_MESSAGE   1
    // Displays startup message to client
#define DEBUG_EEPROM  0
    // show guts of EEPROM reads and writes

#define MAX_SYMBOL_LENGTH 31
#define MAX_COMMAND_LENGTH (10+MAX_SYMBOL_LENGTH)


#define EEPROM_HOST          0      // one byte host mode 255==0
#define EEPROM_HOST_ID       32
#define EEPROM_SSID          64
#define EEPROM_PASS          96
#define EEPROM_HOST_PASS     128


//-------------------------------
// vars
//-------------------------------

WiFiServer server(80);
WiFiClient client;

int  is_host = 0;
char host_id[MAX_SYMBOL_LENGTH+1] = {0};
char host_pass[MAX_SYMBOL_LENGTH+1] = {0};
char ssid[MAX_SYMBOL_LENGTH+1] = {0};
char pass[MAX_SYMBOL_LENGTH+1] = {0};

int started = 0;
int in_command = 0;
int command_len = 0;
char command[MAX_COMMAND_LENGTH];

#if BUFFER_WIFI_XMIT
    #define MAX_PACKET   255
    char packet[MAX_PACKET+1] = {0};
    int packet_len = 0;
#endif


//-----------------------------
// methods
//-----------------------------

void readEEPROMString(int location, char *buf, const char *def_value)
{
    byte len = EEPROM.read(location++);

    if (len >= MAX_SYMBOL_LENGTH)
    {
        #if DEBUG_EEPROM
            Serial.print("ESP using default for location");
            Serial.print(location);
            Serial.print(" string=");
            Serial.println(buf);
        #endif
        strcpy(buf,def_value);
    }
    else
    {
        #if DEBUG_EEPROM
            Serial.print("ESP EEPROM read string len=");
            Serial.print(len);
            Serial.print(" from ");
            Serial.println(location);
        #endif

        int i = 0;
        while (i<len)
            buf[i++] = EEPROM.read(location++);
        buf[i++] = 0;

        #if DEBUG_EEPROM
            Serial.print("ESP string=");
            Serial.println(buf);
        #endif
    }
}


void writeEEPROMString(const char *what, int location, char *buf)
{
    Serial.print("ESP setting ");
    Serial.print(what);
    Serial.print(" to '");
    Serial.print(buf);
    Serial.println("'");

    byte len = strlen(buf);
    EEPROM.write(location++,len);
    for (int i=0; i<len; i++)
    {
        EEPROM.write(location++,buf[i]);
    }
    EEPROM.commit();
}



void readEEPROM()
{
    is_host = EEPROM.read(EEPROM_HOST);
    if (is_host == 255)
        is_host = DEFAULT_HOST_MODE;

    #if DEBUG_EEPROM
        Serial.print("ESP EEPROM host=");
        Serial.println(is_host);
    #endif

    readEEPROMString(EEPROM_HOST_ID,host_id,DEFAULT_HOST_ID);
    readEEPROMString(EEPROM_HOST_PASS,host_pass,DEFAULT_HOST_PASS);
    readEEPROMString(EEPROM_SSID,ssid,DEFAULT_HOST_ID);
    readEEPROMString(EEPROM_PASS,pass,DEFAULT_HOST_PASS);
}



//----------------------------------------------------
// connect
//----------------------------------------------------


void connect()
{
    if (started)
    {
        if (!is_host)
            WiFi.disconnect();
        WiFi.mode(WIFI_OFF);
    }

    started = 0;

    #if WITH_SERIAL_STARTUP
        delay(1500);
        Serial.println("");
        Serial.println("esp8266Server for Tumbller version 1.3 starting");
        if (is_host)
        {
            Serial.print("ESP Starting HOST MODE SSID=");
            Serial.print(host_id);
            Serial.print(" PASS=");
            Serial.println(host_pass);
        }
        else if (ssid[0])
        {
            if (pass[0])
            {
                Serial.print("ESP Connecting to ");
                Serial.println(ssid);
            }
            else
            {
                Serial.print("ESP ERROR - No password for SSED=");
                Serial.println(ssid);
            }
        }
        else
        {
            Serial.println("ESP ERROR - No station SSID found!!");
        }
    #endif

    if (is_host)
    {
        // currently there is no password for the softAP

        if (WiFi.softAP(host_id,host_pass))
        {
            Serial.print("HOST AP started IP address=");
            Serial.println(DEFAULT_AP_IP);
            started = 1;
        }
        else
        {
            Serial.println("ESP ERROR = could not start AP");
        }
    }
    else if (ssid[0] && pass[0])
    {
        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid, pass);

        // Wait for connection

        int retry_count = 0;
        while (retry_count++<20 && WiFi.status() != WL_CONNECTED)
        {
            delay(500);
            #if WITH_SERIAL_STARTUP
                Serial.print(".");
            #endif
        }

        if (WiFi.status() != WL_CONNECTED)
        {
            WiFi.mode(WIFI_OFF);

            #if WITH_SERIAL_STARTUP
                Serial.println();
                Serial.print("ESP ERROR - Could not connect to ");
                Serial.println(ssid);
            #endif
        }
        else
        {
            #if WITH_SERIAL_STARTUP
                Serial.println();
                Serial.print("ESP Station Connected to ");
                Serial.println(ssid);
                Serial.print("ESP IP address: ");
                Serial.println(WiFi.localIP());
            #endif

            started = 1;
        }
    }

    if (started)
    {
        delay(500);
        server.begin();
    }

    #if WITH_SERIAL_STARTUP
        delay(500);
        Serial.println("esp8266Server ready");
            // teensy can wait for this line to be ready
    #endif

}


//----------------------------------------------------
// handleCommand and setup
//----------------------------------------------------

void handleCommand()
{
    bool try_connect = 0;
    if (!strncmp(command,"SSID ",5))
    {
        writeEEPROMString("SSID",EEPROM_SSID,&command[5]);
        try_connect = 1;
    }
    else if (!strncmp(command,"PASS ",5))
    {
        writeEEPROMString("PASS",EEPROM_PASS,&command[5]);
        try_connect = 1;
    }
    else if (!strncmp(command,"HOST_ID ",8))
    {
        writeEEPROMString("HOST_ID",EEPROM_HOST_ID,&command[8]);
        try_connect = 1;
    }
    else if (!strncmp(command,"HOST_PASS ",10))
    {
        writeEEPROMString("HOST_ID",EEPROM_HOST_ID,&command[10]);
        try_connect = 1;
    }
    else if (!strcmp(command,"HOST 0") ||
             !strcmp(command,"HOST 1"))
    {
        int val = command[5] == '1' ? 1 : 0;
        Serial.print("ESP setting HOST to ");
        Serial.println(val);
        EEPROM.write(EEPROM_HOST,val);
        EEPROM.commit();
        try_connect = 1;
    }
    else if (!strcmp(command,"RESET"))
    {
        Serial.println("ESP resetting default EEPROM values");
        for (int i=0; i<512; i++)
            EEPROM.write(i,255);
        EEPROM.commit();
        try_connect = 1;
    }
    else
    {
        Serial.print("ESP ERROR - UNKNOWN COMMAND: ");
        Serial.println(command);
    }
    if (try_connect)
    {
        readEEPROM();
        connect();
    }
}


void setup(void)
{
    pinMode(0,INPUT);
    pinMode(2,INPUT);

    Serial.begin(HOST_BAUD_RATE);
    delay(500);

    EEPROM.begin(512);
    readEEPROM();
    connect();
}



//----------------------------------------------------
// loop
//----------------------------------------------------

void loop(void)
{
    if (started)
    {
        if (!client || !client.connected())
        {
            client = server.available();
            if (client)
            {
                // #if WITH_SERIAL_DEBUG
                //     Serial.println("ESP CLIENT CONNECTED");
                // #endif
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
        }

    }   // if (started)


    if (Serial.available())
    {
        char c = Serial.read();
        if (c == '#')
        {
            in_command = 1;
            command_len = 0;
        }
        else if (in_command)
        {
            if (command_len >= MAX_COMMAND_LENGTH || c == 10)
            {
                command[command_len++] = 0;
                handleCommand();
            }
            else if (c != 13)
            {
                command[command_len++] = c;
            }
        }
        else
        {
            #if BUFFER_WIFI_XMIT
                if (packet_len >= MAX_PACKET || c == 10)
                {
                    packet[packet_len++] = 0;
                    packet_len = 0;
                    client.println(packet);
                }
                else if (c != 13)
                {
                    packet[packet_len++] = c;
                }
            #else
                client.write(c);
            #endif
        }
    }

}
