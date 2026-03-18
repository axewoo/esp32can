#include <Arduino.h>
#include <Wire.h>
#include <CAN.h>
#include "rgb_lcd.h"

rgb_lcd lcd;

#define TX_GPIO_NUM 18
#define RX_GPIO_NUM 4
#define CAN_ID_LCD 0x100

volatile uint8_t newR = 0, newG = 0, newB = 0;
volatile bool colorUpdate = false;

void onReceive(int packetSize);

void setup()
{
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n===== CAN BUS RECEIVER =====");
    Serial.println("Initializing I2C...");
    Wire.begin();
    delay(100);
    
    Serial.println("Initializing LCD...");
    lcd.begin(16, 2, LCD_5x8DOTS);
    lcd.setRGB(0, 0, 0);
    Serial.println("LCD OK");
    
    Serial.println("Starting CAN...");
    CAN.setPins(RX_GPIO_NUM, TX_GPIO_NUM);
    
    if (!CAN.begin(250E3))
    {
        Serial.println("ERROR: CAN init failed!");
        while (1);
    }

    CAN.onReceive(onReceive);
    Serial.println("CAN OK - waiting for frames...");
}

void loop()
{
    if (CAN.parsePacket())
    {
        onReceive(CAN.available());
    }
    
    if (colorUpdate)
    {
        colorUpdate = false;
        lcd.setRGB(newR, newG, newB);
        Serial.print("LCD Color Set: R=");
        Serial.print(newR);
        Serial.print(" G=");
        Serial.print(newG);
        Serial.print(" B=");
        Serial.println(newB);
    }
    
    delay(10);
}

void onReceive(int packetSize)
{
    Serial.println("\n--- Frame Received ---");
    
    Serial.print("Type: ");
    if (CAN.packetRtr())
    {
        Serial.println("RTR (Remote Transmission Request)");
        Serial.print("ID: 0x");
        Serial.print(CAN.packetId(), HEX);
        Serial.print(", DLC: ");
        Serial.println(CAN.packetDlc());
        return;
    }
    else
    {
        Serial.println("DATA");
    }

    Serial.print("Extended: ");
    Serial.println(CAN.packetExtended() ? "Yes" : "No");

    Serial.print("ID: 0x");
    Serial.print(CAN.packetId(), HEX);
    
    Serial.print(", Size: ");
    Serial.print(packetSize);
    Serial.println(" bytes");

    if (!CAN.packetExtended() && CAN.packetId() == CAN_ID_LCD && packetSize == 3)
    {
        newR = (uint8_t)CAN.read();
        newG = (uint8_t)CAN.read();
        newB = (uint8_t)CAN.read();

        Serial.print("Color received: R=");
        Serial.print(newR);
        Serial.print(" G=");
        Serial.print(newG);
        Serial.print(" B=");
        Serial.println(newB);

        colorUpdate = true;
    }
    else
    {
        Serial.print("Data: ");
        while (CAN.available())
        {
            Serial.print(CAN.read(), HEX);
            Serial.print(" ");
        }
        Serial.println();
    }
    
    Serial.println("-----------------------");
}