// Visual Micro is in vMicro>General>Tutorial Mode
// 
/*
    Name:       ADS_B_SDR_STM32F407_24_10_08_01.ino
    Created:	08.10.2024 7:41:34
    Author:     MASTER\Alex
*/

#include "Arduino.h"
#include "main.h"
//#include "fatfs.h"
#include "usb_host.h"
#include "Configuration_ESP32.h"


#include "stdint.h"
#include "string.h"
#include <stdlib.h>
#include <stdio.h>

//#include <GxTFT.h> // Hardware-specific library
//#define TFT_Class GxTFT
//#include <GxIO/STM32GENERIC/GxIO_STM32F4_FSMC/GxIO_STM32F4_FSMC.h>
//
//// select one GxCTRL class
//#include <GxCTRL/GxCTRL_ILI9341/GxCTRL_ILI9341.h> // 240x320
//
//// create instance for the selected GxIO class
//GxIO_Class io; // #define GxIO_Class is in the selected header file
//// note: "error: 'GxIO_Class' does not name a type": indicates target board selection mismatch
//
//// create instance for the selected GxCTRL class
//GxCTRL_Class controller(io); // #define GxCTRL_Class is in the selected header file
//
//// select one or adapt
////TFT_Class tft(io, controller, 240, 320); // portrait 240x320
//TFT_Class tft(io, controller, 320, 240); // landscape 240x320
//



void setup() 
{
    //Initialize serial and wait for port to open:
    Serial.begin(115200);
    delay(500);
    Serial.println("Start");

    pinMode(BUTTON_WAKEUP, INPUT);
    pinMode(BUTTON_KEY0, INPUT);
    pinMode(BUTTON_KEY1, INPUT);

    pinMode(LED_RED, OUTPUT);                // Подсветка дисплея
    digitalWrite(LED_RED, LOW);             // 
    delay(500);
    digitalWrite(LED_RED, HIGH);             // 
    pinMode(LED_GREEN, OUTPUT);                // Подсветка дисплея
    digitalWrite(LED_GREEN, LOW);             // 
    delay(500);
    digitalWrite(LED_GREEN, HIGH);             // 

   // tft.init();

    Serial.println("tft.init() done");

  

    MX_USB_HOST_Init();

}


void loop()
{
   // MX_USB_HOST_Process();


}

//unsigned long testFillScreen() {
//    unsigned long start = micros();
//    tft.fillScreen(BLACK);
//    yield();
//    if (controller.ID == 0x8875) delay(200); // too fast to be seen
//    tft.fillScreen(RED);
//    yield();
//    if (controller.ID == 0x8875) delay(200); // too fast to be seen
//    tft.fillScreen(GREEN);
//    yield();
//    if (controller.ID == 0x8875) delay(200); // too fast to be seen
//    tft.fillScreen(BLUE);
//    yield();
//    if (controller.ID == 0x8875) delay(200); // too fast to be seen
//    tft.fillScreen(BLACK);
//    yield();
//    return micros() - start;
//}
//
//unsigned long testText() {
//    tft.fillScreen(BLACK);
//    unsigned long start = micros();
//    tft.setCursor(0, 0);
//    tft.setTextColor(WHITE);  tft.setTextSize(1);
//    tft.println("Hello World!");
//    tft.setTextColor(YELLOW); tft.setTextSize(2);
//    tft.println(1234.56);
//    tft.setTextColor(RED);    tft.setTextSize(3);
//    tft.println(0xDEADBEEF, HEX);
//    tft.println();
//    tft.setTextColor(GREEN);
//    tft.setTextSize(5);
//    tft.println("Groop");
//    tft.setTextSize(2);
//    tft.println("I implore thee,");
//    tft.setTextSize(1);
//    tft.setTextColor(GREEN);
//    tft.println("my foonting turlingdromes.");
//    tft.println("And hooptiously drangle me");
//    tft.println("with crinkly bindlewurdles,");
//    tft.println("Or I will rend thee");
//    tft.println("in the gobberwarts");
//    tft.println("with my blurglecruncheon,");
//    tft.println("see if I don't!");
//    return micros() - start;
//}
//
//unsigned long testLines(uint16_t color) {
//    unsigned long start, t;
//    int           x1, y1, x2, y2,
//        w = tft.width(),
//        h = tft.height();
//
//    tft.fillScreen(BLACK);
//    yield();
//
//    x1 = y1 = 0;
//    y2 = h - 1;
//    start = micros();
//    for (x2 = 0; x2 < w; x2 += 6) tft.drawLine(x1, y1, x2, y2, color);
//    yield();
//    x2 = w - 1;
//    for (y2 = 0; y2 < h; y2 += 6) tft.drawLine(x1, y1, x2, y2, color);
//    yield();
//    t = micros() - start; // fillScreen doesn't count against timing
//
//    tft.fillScreen(BLACK);
//    yield();
//
//    x1 = w - 1;
//    y1 = 0;
//    y2 = h - 1;
//    start = micros();
//    for (x2 = 0; x2 < w; x2 += 6) tft.drawLine(x1, y1, x2, y2, color);
//    yield();
//    x2 = 0;
//    for (y2 = 0; y2 < h; y2 += 6) tft.drawLine(x1, y1, x2, y2, color);
//    yield();
//    t += micros() - start;
//
//    tft.fillScreen(BLACK);
//    yield();
//
//    x1 = 0;
//    y1 = h - 1;
//    y2 = 0;
//    start = micros();
//    for (x2 = 0; x2 < w; x2 += 6) tft.drawLine(x1, y1, x2, y2, color);
//    yield();
//    x2 = w - 1;
//    for (y2 = 0; y2 < h; y2 += 6) tft.drawLine(x1, y1, x2, y2, color);
//    yield();
//    t += micros() - start;
//
//    tft.fillScreen(BLACK);
//    yield();
//
//    x1 = w - 1;
//    y1 = h - 1;
//    y2 = 0;
//    start = micros();
//    for (x2 = 0; x2 < w; x2 += 6) tft.drawLine(x1, y1, x2, y2, color);
//    yield();
//    x2 = 0;
//    for (y2 = 0; y2 < h; y2 += 6) tft.drawLine(x1, y1, x2, y2, color);
//    yield();
//
//    return micros() - start;
//}
//
//unsigned long testFastLines(uint16_t color1, uint16_t color2) {
//    unsigned long start;
//    int           x, y, w = tft.width(), h = tft.height();
//
//    tft.fillScreen(BLACK);
//    yield();
//    start = micros();
//    for (y = 0; y < h; y += 5) tft.drawFastHLine(0, y, w, color1);
//    yield();
//    for (x = 0; x < w; x += 5) tft.drawFastVLine(x, 0, h, color2);
//    yield();
//
//    return micros() - start;
//}
//
//unsigned long testRects(uint16_t color) {
//    unsigned long start;
//    int           n, i, i2,
//        cx = tft.width() / 2,
//        cy = tft.height() / 2;
//
//    tft.fillScreen(BLACK);
//    n = min(tft.width(), tft.height());
//    start = micros();
//    for (i = 2; i < n; i += 6) {
//        i2 = i / 2;
//        tft.drawRect(cx - i2, cy - i2, i, i, color);
//    }
//
//    return micros() - start;
//}
//
//unsigned long testFilledRects(uint16_t color1, uint16_t color2) {
//    unsigned long start, t = 0;
//    int           n, i, i2,
//        cx = tft.width() / 2 - 1,
//        cy = tft.height() / 2 - 1;
//
//    tft.fillScreen(BLACK);
//    n = min(tft.width(), tft.height());
//    for (i = n; i > 0; i -= 6) {
//        i2 = i / 2;
//        start = micros();
//        tft.fillRect(cx - i2, cy - i2, i, i, color1);
//        t += micros() - start;
//        // Outlines are not included in timing results
//        tft.drawRect(cx - i2, cy - i2, i, i, color2);
//        yield();
//    }
//
//    return t;
//}
//
//unsigned long testFilledCircles(uint8_t radius, uint16_t color) {
//    unsigned long start;
//    int x, y, w = tft.width(), h = tft.height(), r2 = radius * 2;
//
//    tft.fillScreen(BLACK);
//    start = micros();
//    for (x = radius; x < w; x += r2) {
//        for (y = radius; y < h; y += r2) {
//            tft.fillCircle(x, y, radius, color);
//        }
//        yield();
//    }
//
//    return micros() - start;
//}
//
//unsigned long testCircles(uint8_t radius, uint16_t color) {
//    unsigned long start;
//    int           x, y, r2 = radius * 2,
//        w = tft.width() + radius,
//        h = tft.height() + radius;
//
//    // Screen is not cleared for this one -- this is
//    // intentional and does not affect the reported time.
//    start = micros();
//    for (x = 0; x < w; x += r2) {
//        for (y = 0; y < h; y += r2) {
//            tft.drawCircle(x, y, radius, color);
//        }
//        yield();
//    }
//
//    return micros() - start;
//}
//
//unsigned long testTriangles() {
//    unsigned long start;
//    int           n, i, cx = tft.width() / 2 - 1,
//        cy = tft.height() / 2 - 1;
//
//    tft.fillScreen(BLACK);
//    n = min(cx, cy);
//    start = micros();
//    for (i = 0; i < n; i += 5) {
//        tft.drawTriangle(
//            cx, cy - i, // peak
//            cx - i, cy + i, // bottom left
//            cx + i, cy + i, // bottom right
//            tft.color565(0, 0, i));
//    }
//
//    return micros() - start;
//}
//
//unsigned long testFilledTriangles() {
//    unsigned long start, t = 0;
//    int           i, cx = tft.width() / 2 - 1,
//        cy = tft.height() / 2 - 1;
//
//    tft.fillScreen(BLACK);
//    start = micros();
//    for (i = min(cx, cy); i > 10; i -= 5) {
//        start = micros();
//        tft.fillTriangle(cx, cy - i, cx - i, cy + i, cx + i, cy + i,
//            tft.color565(0, i, i));
//        t += micros() - start;
//        tft.drawTriangle(cx, cy - i, cx - i, cy + i, cx + i, cy + i,
//            tft.color565(i, i, 0));
//        yield();
//    }
//
//    return t;
//}
//
//unsigned long testRoundRects() {
//    unsigned long start;
//    int           w, i, i2,
//        cx = tft.width() / 2 - 1,
//        cy = tft.height() / 2 - 1;
//
//    tft.fillScreen(BLACK);
//    w = min(tft.width(), tft.height());
//    start = micros();
//    for (i = 0; i < w; i += 6) {
//        i2 = i / 2;
//        tft.drawRoundRect(cx - i2, cy - i2, i, i, i / 8, tft.color565(i, 0, 0));
//    }
//
//    return micros() - start;
//}
//
//unsigned long testFilledRoundRects() {
//    unsigned long start;
//    int           i, i2,
//        cx = tft.width() / 2 - 1,
//        cy = tft.height() / 2 - 1;
//
//    tft.fillScreen(BLACK);
//    start = micros();
//    for (i = min(tft.width(), tft.height()); i > 20; i -= 6) {
//        i2 = i / 2;
//        tft.fillRoundRect(cx - i2, cy - i2, i, i, i / 8, tft.color565(0, i, 0));
//        yield();
//    }
//
//    return micros() - start;
//}
//
//void testEllipses()
//{
//    tft.fillScreen(BLACK);
//    for (int i = 0; i < 40; i++)
//    {
//        int rx = random(60);
//        int ry = random(60);
//        int x = rx + random(480 - rx - rx);
//        int y = ry + random(320 - ry - ry);
//        tft.fillEllipse(x, y, rx, ry, random(0xFFFF));
//    }
//    delay(2000);
//    tft.fillScreen(BLACK);
//    for (int i = 0; i < 40; i++)
//    {
//        int rx = random(60);
//        int ry = random(60);
//        int x = rx + random(480 - rx - rx);
//        int y = ry + random(320 - ry - ry);
//        tft.drawEllipse(x, y, rx, ry, random(0xFFFF));
//    }
//    delay(2000);
//}
//
//void testCurves()
//{
//    uint16_t x = tft.width() / 2;
//    uint16_t y = tft.height() / 2;
//    tft.fillScreen(BLACK);
//
//    tft.drawEllipse(x, y, 100, 60, PURPLE);
//    tft.fillCurve(x, y, 80, 30, 0, CYAN);
//    tft.fillCurve(x, y, 80, 30, 1, MAGENTA);
//    tft.fillCurve(x, y, 80, 30, 2, YELLOW);
//    tft.fillCurve(x, y, 80, 30, 3, RED);
//    delay(2000);
//
//    tft.drawCurve(x, y, 90, 50, 0, CYAN);
//    tft.drawCurve(x, y, 90, 50, 1, MAGENTA);
//    tft.drawCurve(x, y, 90, 50, 2, YELLOW);
//    tft.drawCurve(x, y, 90, 50, 3, RED);
//    tft.fillCircle(x, y, 30, BLUE);
//    delay(5000);
//}
//
//void reportID()
//{
//    uint32_t id = controller.readID();
//    tft.fillScreen(BLACK);
//    tft.setCursor(0, 0);
//    tft.setTextColor(WHITE);
//    tft.setTextSize((tft.width() > 320) ? 2 : 1);
//    tft.print("Controller "); tft.println(controller.name);
//    tft.println();
//    tft.print("ID 0x"); tft.print(controller.ID, HEX);
//    tft.print(" readID() 0x"); tft.println(id, HEX);
//    tft.println();
//}
//
//// Original sketch header
///***************************************************


