#include "myDebug.h"

#define TEENSY_ONBOARD_LED    13
#define LED_FLASH_ON          10
#define LED_FLASH_OFF        800

#define ESP2866_RESET         16        // A2
#define ESP2866_GPIO_0_PROG    7
#define ESP2866_GPIO_2         8




void setup()
{
    Serial3.begin(9600);

    pinMode(ESP2866_RESET,OUTPUT);
    pinMode(ESP2866_GPIO_0_PROG,OUTPUT);
    pinMode(ESP2866_GPIO_2,INPUT);

    digitalWrite(ESP2866_RESET,1);
    digitalWrite(ESP2866_GPIO_0_PROG,1);

    pinMode(TEENSY_ONBOARD_LED, OUTPUT);
    digitalWrite(TEENSY_ONBOARD_LED,1);

    Serial.begin(115200);

    delay(1200);
    digitalWrite(TEENSY_ONBOARD_LED,0);
    display(0,"teensyEsp8266 v1.3 started",0);
}



void loop()
{
    // Flash LED

    static int flash_state = 0;
    static elapsedMillis flash_time = 0;

    if ((flash_state && flash_time>LED_FLASH_ON) ||
        (!flash_state && flash_time>LED_FLASH_OFF))
    {
        flash_state = !flash_state;
        digitalWrite(TEENSY_ONBOARD_LED, flash_state);
        flash_time  = 0;
    }


    // commands are recognized by being preceded and followed by 200ms of silence.
    // all chars are passed through
    //      R to Reset
    //      P to enter program mode

    static uint32_t char_time = 0;
    static uint32_t prev_char_time = 0;
    static char command_char = 0;

    if (Serial.available())
    {
        char c = Serial.read();

        prev_char_time = char_time;
        char_time = millis();
        if (char_time - prev_char_time > 200)
            command_char = c;
        else
            command_char = 0;

        Serial3.write(c);
    }

    if (command_char && millis() > char_time + 200)
    {
        // display(0,"command_char=%c",command_char);
        if (command_char == 'r')
        {
            display(0,"reset",0);
            digitalWrite(ESP2866_RESET,0);
            delay(50);
            digitalWrite(ESP2866_RESET,1);
        }
        else if (command_char == 'p')
        {
            display(0,"entering program mode",0);
            digitalWrite(ESP2866_RESET,0);
            digitalWrite(ESP2866_GPIO_0_PROG,0);
            delay(1000);
            digitalWrite(ESP2866_RESET,1);
            delay(1000);
            digitalWrite(ESP2866_GPIO_0_PROG,1);
        }
        command_char = 0;
    }


    if (Serial3.available())
    {
        char c = Serial3.read();
        Serial.write(c);
    }
}
