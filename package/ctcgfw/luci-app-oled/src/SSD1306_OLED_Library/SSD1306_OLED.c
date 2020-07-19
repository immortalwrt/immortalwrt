/*
 * MIT License

Copyright (c) 2017 DeeplyEmbedded

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

 * SSD1306_OLED.c
 *
 *  Created on  : Sep 26, 2017
 *  Author      : Vinay Divakar
 *  Description : SSD1306 OLED Driver, Graphics API's.
 *  Website     : www.deeplyembedded.org
 */

/* Lib Includes */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include "I2C.h"
#include "SSD1306_OLED.h"
#include "gfxfont.h"

/* Enable or Disable DEBUG Prints */
//#define SSD1306_DBG

/* MACROS */
#define SWAP(x,y)     {short temp; temp = x; x = y; y = temp;}
#define pgm_read_byte(addr) (*(const unsigned char *)(addr))
#define pgm_read_word(addr) (*(const unsigned long *)(addr))
#define pgm_read_dword(addr) (*(const unsigned long *)(addr))
#define pgm_read_pointer(addr) ((void *)pgm_read_word(addr))

/* static Variables */
static unsigned char _rotation = 0,textsize = 0;
static short _width = SSD1306_LCDWIDTH;
static short _height = SSD1306_LCDHEIGHT;
static short cursor_x = 0, cursor_y = 0, textcolor = 0, textbgcolor = 0;
static bool _cp437 = false, wrap = true;

/* static struct objects */
static GFXfontPtr gfxFont;

/* Externs - I2C.c */
extern I2C_DeviceT I2C_DEV_2;

/* Chunk Buffer */
static unsigned char chunk[17] = {0};

/* Memory buffer for displaying data on LCD - This is an Apple - Fruit */
static unsigned char screen[DISPLAY_BUFF_SIZE] ={0};

/* Static Functions */
static void transfer();
static void drawFastVLine(short x, short y,short h, short color);
static void writeFastVLine(short x, short y, short h, short color);
static void drawFastHLine(short x, short y,short w, short color);
static void writeFastHLine(short x, short y, short w, short color);
static short print(const unsigned char *buffer, short size);

// Standard ASCII 5x7 font
static const unsigned char ssd1306_font5x7[] = {
                                                0x00, 0x00, 0x00, 0x00, 0x00, //space
                                                0x3E, 0x5B, 0x4F, 0x5B, 0x3E,
                                                0x3E, 0x6B, 0x4F, 0x6B, 0x3E,
                                                0x1C, 0x3E, 0x7C, 0x3E, 0x1C,
                                                0x18, 0x3C, 0x7E, 0x3C, 0x18,
                                                0x1C, 0x57, 0x7D, 0x57, 0x1C,
                                                0x1C, 0x5E, 0x7F, 0x5E, 0x1C,
                                                0x00, 0x18, 0x3C, 0x18, 0x00,
                                                0xFF, 0xE7, 0xC3, 0xE7, 0xFF,
                                                0x00, 0x18, 0x24, 0x18, 0x00,
                                                0xFF, 0xE7, 0xDB, 0xE7, 0xFF,
                                                0x30, 0x48, 0x3A, 0x06, 0x0E,
                                                0x26, 0x29, 0x79, 0x29, 0x26,
                                                0x40, 0x7F, 0x05, 0x05, 0x07,
                                                0x40, 0x7F, 0x05, 0x25, 0x3F,
                                                0x5A, 0x3C, 0xE7, 0x3C, 0x5A,
                                                0x7F, 0x3E, 0x1C, 0x1C, 0x08,
                                                0x08, 0x1C, 0x1C, 0x3E, 0x7F,
                                                0x14, 0x22, 0x7F, 0x22, 0x14,
                                                0x5F, 0x5F, 0x00, 0x5F, 0x5F,
                                                0x06, 0x09, 0x7F, 0x01, 0x7F,
                                                0x00, 0x66, 0x89, 0x95, 0x6A,
                                                0x60, 0x60, 0x60, 0x60, 0x60,
                                                0x94, 0xA2, 0xFF, 0xA2, 0x94,
                                                0x08, 0x04, 0x7E, 0x04, 0x08,//up INDEX 24
                                                0x10, 0x20, 0x7E, 0x20, 0x10,//down INDEX 25
                                                0x08, 0x08, 0x2A, 0x1C, 0x08,
                                                0x08, 0x1C, 0x2A, 0x08, 0x08,
                                                0x1E, 0x10, 0x10, 0x10, 0x10,
                                                0x0C, 0x1E, 0x0C, 0x1E, 0x0C,
                                                0x30, 0x38, 0x3E, 0x38, 0x30,
                                                0x06, 0x0E, 0x3E, 0x0E, 0x06,
                                                0x00, 0x00, 0x00, 0x00, 0x00,
                                                0x00, 0x00, 0x5F, 0x00, 0x00,
                                                0x00, 0x07, 0x00, 0x07, 0x00,
                                                0x14, 0x7F, 0x14, 0x7F, 0x14,
                                                0x24, 0x2A, 0x7F, 0x2A, 0x12,
                                                0x23, 0x13, 0x08, 0x64, 0x62,
                                                0x36, 0x49, 0x56, 0x20, 0x50,
                                                0x00, 0x08, 0x07, 0x03, 0x00,
                                                0x00, 0x1C, 0x22, 0x41, 0x00,
                                                0x00, 0x41, 0x22, 0x1C, 0x00,
                                                0x2A, 0x1C, 0x7F, 0x1C, 0x2A,
                                                0x08, 0x08, 0x3E, 0x08, 0x08,
                                                0x00, 0x80, 0x70, 0x30, 0x00,
                                                0x08, 0x08, 0x08, 0x08, 0x08,
                                                0x00, 0x00, 0x60, 0x60, 0x00,
                                                0x20, 0x10, 0x08, 0x04, 0x02,
                                                0x3E, 0x51, 0x49, 0x45, 0x3E,
                                                0x00, 0x42, 0x7F, 0x40, 0x00,
                                                0x72, 0x49, 0x49, 0x49, 0x46,
                                                0x21, 0x41, 0x49, 0x4D, 0x33,
                                                0x18, 0x14, 0x12, 0x7F, 0x10,
                                                0x27, 0x45, 0x45, 0x45, 0x39,
                                                0x3C, 0x4A, 0x49, 0x49, 0x31,
                                                0x41, 0x21, 0x11, 0x09, 0x07,
                                                0x36, 0x49, 0x49, 0x49, 0x36,
                                                0x46, 0x49, 0x49, 0x29, 0x1E,
                                                0x00, 0x00, 0x14, 0x00, 0x00,
                                                0x00, 0x40, 0x34, 0x00, 0x00,
                                                0x00, 0x08, 0x14, 0x22, 0x41,
                                                0x14, 0x14, 0x14, 0x14, 0x14,
                                                0x00, 0x41, 0x22, 0x14, 0x08,
                                                0x02, 0x01, 0x59, 0x09, 0x06,
                                                0x3E, 0x41, 0x5D, 0x59, 0x4E,
                                                0x7C, 0x12, 0x11, 0x12, 0x7C,
                                                0x7F, 0x49, 0x49, 0x49, 0x36,
                                                0x3E, 0x41, 0x41, 0x41, 0x22,//C
                                                0x7F, 0x41, 0x41, 0x41, 0x3E,//D
                                                0x7F, 0x49, 0x49, 0x49, 0x41,//E
                                                0x7F, 0x09, 0x09, 0x09, 0x01,//F
                                                0x3E, 0x41, 0x41, 0x51, 0x73,
                                                0x7F, 0x08, 0x08, 0x08, 0x7F,
                                                0x00, 0x41, 0x7F, 0x41, 0x00,
                                                0x20, 0x40, 0x41, 0x3F, 0x01,
                                                0x7F, 0x08, 0x14, 0x22, 0x41,
                                                0x7F, 0x40, 0x40, 0x40, 0x40,
                                                0x7F, 0x02, 0x1C, 0x02, 0x7F,
                                                0x7F, 0x04, 0x08, 0x10, 0x7F,
                                                0x3E, 0x41, 0x41, 0x41, 0x3E,
                                                0x7F, 0x09, 0x09, 0x09, 0x06,
                                                0x3E, 0x41, 0x51, 0x21, 0x5E,
                                                0x7F, 0x09, 0x19, 0x29, 0x46,
                                                0x26, 0x49, 0x49, 0x49, 0x32,
                                                0x03, 0x01, 0x7F, 0x01, 0x03,
                                                0x3F, 0x40, 0x40, 0x40, 0x3F,
                                                0x1F, 0x20, 0x40, 0x20, 0x1F,
                                                0x3F, 0x40, 0x38, 0x40, 0x3F,
                                                0x63, 0x14, 0x08, 0x14, 0x63,
                                                0x03, 0x04, 0x78, 0x04, 0x03,
                                                0x61, 0x59, 0x49, 0x4D, 0x43,
                                                0x00, 0x7F, 0x41, 0x41, 0x41,
                                                0x02, 0x04, 0x08, 0x10, 0x20,
                                                0x00, 0x41, 0x41, 0x41, 0x7F,
                                                0x04, 0x02, 0x01, 0x02, 0x04,
                                                0x40, 0x40, 0x40, 0x40, 0x40,
                                                0x00, 0x03, 0x07, 0x08, 0x00,
                                                0x20, 0x54, 0x54, 0x78, 0x40,
                                                0x7F, 0x28, 0x44, 0x44, 0x38,
                                                0x38, 0x44, 0x44, 0x44, 0x28,
                                                0x38, 0x44, 0x44, 0x28, 0x7F,
                                                0x38, 0x54, 0x54, 0x54, 0x18,
                                                0x00, 0x08, 0x7E, 0x09, 0x02,
                                                0x18, 0xA4, 0xA4, 0x9C, 0x78,
                                                0x7F, 0x08, 0x04, 0x04, 0x78,
                                                0x00, 0x44, 0x7D, 0x40, 0x00,
                                                0x20, 0x40, 0x40, 0x3D, 0x00,
                                                0x7F, 0x10, 0x28, 0x44, 0x00,
                                                0x00, 0x41, 0x7F, 0x40, 0x00,
                                                0x7C, 0x04, 0x78, 0x04, 0x78,
                                                0x7C, 0x08, 0x04, 0x04, 0x78,
                                                0x38, 0x44, 0x44, 0x44, 0x38,
                                                0xFC, 0x18, 0x24, 0x24, 0x18,
                                                0x18, 0x24, 0x24, 0x18, 0xFC,
                                                0x7C, 0x08, 0x04, 0x04, 0x08,
                                                0x48, 0x54, 0x54, 0x54, 0x24,
                                                0x04, 0x04, 0x3F, 0x44, 0x24,
                                                0x3C, 0x40, 0x40, 0x20, 0x7C,
                                                0x1C, 0x20, 0x40, 0x20, 0x1C,
                                                0x3C, 0x40, 0x30, 0x40, 0x3C,
                                                0x44, 0x28, 0x10, 0x28, 0x44,
                                                0x4C, 0x90, 0x90, 0x90, 0x7C,
                                                0x44, 0x64, 0x54, 0x4C, 0x44,
                                                0x00, 0x08, 0x36, 0x41, 0x00,
                                                0x00, 0x00, 0x77, 0x00, 0x00,
                                                0x00, 0x41, 0x36, 0x08, 0x00,
                                                0x02, 0x01, 0x02, 0x04, 0x02,
                                                0x3C, 0x26, 0x23, 0x26, 0x3C,
                                                0x1E, 0xA1, 0xA1, 0x61, 0x12,
                                                0x3A, 0x40, 0x40, 0x20, 0x7A,
                                                0x38, 0x54, 0x54, 0x55, 0x59,
                                                0x21, 0x55, 0x55, 0x79, 0x41,
                                                0x22, 0x54, 0x54, 0x78, 0x42, // a-umlaut
                                                0x21, 0x55, 0x54, 0x78, 0x40,
                                                0x20, 0x54, 0x55, 0x79, 0x40,
                                                0x0C, 0x1E, 0x52, 0x72, 0x12,
                                                0x39, 0x55, 0x55, 0x55, 0x59,
                                                0x39, 0x54, 0x54, 0x54, 0x59,
                                                0x39, 0x55, 0x54, 0x54, 0x58,
                                                0x00, 0x00, 0x45, 0x7C, 0x41,
                                                0x00, 0x02, 0x45, 0x7D, 0x42,
                                                0x00, 0x01, 0x45, 0x7C, 0x40,
                                                0x7D, 0x12, 0x11, 0x12, 0x7D, // A-umlaut
                                                0xF0, 0x28, 0x25, 0x28, 0xF0,
                                                0x7C, 0x54, 0x55, 0x45, 0x00,
                                                0x20, 0x54, 0x54, 0x7C, 0x54,
                                                0x7C, 0x0A, 0x09, 0x7F, 0x49,
                                                0x32, 0x49, 0x49, 0x49, 0x32,
                                                0x3A, 0x44, 0x44, 0x44, 0x3A, // o-umlaut
                                                0x32, 0x4A, 0x48, 0x48, 0x30,
                                                0x3A, 0x41, 0x41, 0x21, 0x7A,
                                                0x3A, 0x42, 0x40, 0x20, 0x78,
                                                0x00, 0x9D, 0xA0, 0xA0, 0x7D,
                                                0x3D, 0x42, 0x42, 0x42, 0x3D, // O-umlaut
                                                0x3D, 0x40, 0x40, 0x40, 0x3D,
                                                0x3C, 0x24, 0xFF, 0x24, 0x24,
                                                0x48, 0x7E, 0x49, 0x43, 0x66,
                                                0x2B, 0x2F, 0xFC, 0x2F, 0x2B,
                                                0xFF, 0x09, 0x29, 0xF6, 0x20,
                                                0xC0, 0x88, 0x7E, 0x09, 0x03,
                                                0x20, 0x54, 0x54, 0x79, 0x41,
                                                0x00, 0x00, 0x44, 0x7D, 0x41,
                                                0x30, 0x48, 0x48, 0x4A, 0x32,
                                                0x38, 0x40, 0x40, 0x22, 0x7A,
                                                0x00, 0x7A, 0x0A, 0x0A, 0x72,
                                                0x7D, 0x0D, 0x19, 0x31, 0x7D,
                                                0x26, 0x29, 0x29, 0x2F, 0x28,
                                                0x26, 0x29, 0x29, 0x29, 0x26,
                                                0x30, 0x48, 0x4D, 0x40, 0x20,
                                                0x38, 0x08, 0x08, 0x08, 0x08,
                                                0x08, 0x08, 0x08, 0x08, 0x38,
                                                0x2F, 0x10, 0xC8, 0xAC, 0xBA,
                                                0x2F, 0x10, 0x28, 0x34, 0xFA,
                                                0x00, 0x00, 0x7B, 0x00, 0x00,
                                                0x08, 0x14, 0x2A, 0x14, 0x22,
                                                0x22, 0x14, 0x2A, 0x14, 0x08,
                                                0x55, 0x00, 0x55, 0x00, 0x55, // #176 (25% block) missing in old code
                                                0xAA, 0x55, 0xAA, 0x55, 0xAA, // 50% block
                                                0xFF, 0x55, 0xFF, 0x55, 0xFF, // 75% block
                                                0x00, 0x00, 0x00, 0xFF, 0x00,
                                                0x10, 0x10, 0x10, 0xFF, 0x00,
                                                0x14, 0x14, 0x14, 0xFF, 0x00,
                                                0x10, 0x10, 0xFF, 0x00, 0xFF,
                                                0x10, 0x10, 0xF0, 0x10, 0xF0,
                                                0x14, 0x14, 0x14, 0xFC, 0x00,
                                                0x14, 0x14, 0xF7, 0x00, 0xFF,
                                                0x00, 0x00, 0xFF, 0x00, 0xFF,
                                                0x14, 0x14, 0xF4, 0x04, 0xFC,
                                                0x14, 0x14, 0x17, 0x10, 0x1F,
                                                0x10, 0x10, 0x1F, 0x10, 0x1F,
                                                0x14, 0x14, 0x14, 0x1F, 0x00,
                                                0x10, 0x10, 0x10, 0xF0, 0x00,
                                                0x00, 0x00, 0x00, 0x1F, 0x10,
                                                0x10, 0x10, 0x10, 0x1F, 0x10,
                                                0x10, 0x10, 0x10, 0xF0, 0x10,
                                                0x00, 0x00, 0x00, 0xFF, 0x10,
                                                0x10, 0x10, 0x10, 0x10, 0x10,
                                                0x10, 0x10, 0x10, 0xFF, 0x10,
                                                0x00, 0x00, 0x00, 0xFF, 0x14,
                                                0x00, 0x00, 0xFF, 0x00, 0xFF,
                                                0x00, 0x00, 0x1F, 0x10, 0x17,
                                                0x00, 0x00, 0xFC, 0x04, 0xF4,
                                                0x14, 0x14, 0x17, 0x10, 0x17,
                                                0x14, 0x14, 0xF4, 0x04, 0xF4,
                                                0x00, 0x00, 0xFF, 0x00, 0xF7,
                                                0x14, 0x14, 0x14, 0x14, 0x14,
                                                0x14, 0x14, 0xF7, 0x00, 0xF7,
                                                0x14, 0x14, 0x14, 0x17, 0x14,
                                                0x10, 0x10, 0x1F, 0x10, 0x1F,
                                                0x14, 0x14, 0x14, 0xF4, 0x14,
                                                0x10, 0x10, 0xF0, 0x10, 0xF0,
                                                0x00, 0x00, 0x1F, 0x10, 0x1F,
                                                0x00, 0x00, 0x00, 0x1F, 0x14,
                                                0x00, 0x00, 0x00, 0xFC, 0x14,
                                                0x00, 0x00, 0xF0, 0x10, 0xF0,
                                                0x10, 0x10, 0xFF, 0x10, 0xFF,
                                                0x14, 0x14, 0x14, 0xFF, 0x14,
                                                0x10, 0x10, 0x10, 0x1F, 0x00,
                                                0x00, 0x00, 0x00, 0xF0, 0x10,
                                                0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                                                0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
                                                0xFF, 0xFF, 0xFF, 0x00, 0x00,
                                                0x00, 0x00, 0x00, 0xFF, 0xFF,
                                                0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
                                                0x38, 0x44, 0x44, 0x38, 0x44,
                                                0xFC, 0x4A, 0x4A, 0x4A, 0x34, // sharp-s or beta
                                                0x7E, 0x02, 0x02, 0x06, 0x06,
                                                0x02, 0x7E, 0x02, 0x7E, 0x02,
                                                0x63, 0x55, 0x49, 0x41, 0x63,
                                                0x38, 0x44, 0x44, 0x3C, 0x04,
                                                0x40, 0x7E, 0x20, 0x1E, 0x20,
                                                0x06, 0x02, 0x7E, 0x02, 0x02,
                                                0x99, 0xA5, 0xE7, 0xA5, 0x99,
                                                0x1C, 0x2A, 0x49, 0x2A, 0x1C,
                                                0x4C, 0x72, 0x01, 0x72, 0x4C,
                                                0x30, 0x4A, 0x4D, 0x4D, 0x30,
                                                0x30, 0x48, 0x78, 0x48, 0x30,
                                                0xBC, 0x62, 0x5A, 0x46, 0x3D,
                                                0x3E, 0x49, 0x49, 0x49, 0x00,
                                                0x7E, 0x01, 0x01, 0x01, 0x7E,
                                                0x2A, 0x2A, 0x2A, 0x2A, 0x2A,
                                                0x44, 0x44, 0x5F, 0x44, 0x44,
                                                0x40, 0x51, 0x4A, 0x44, 0x40,
                                                0x40, 0x44, 0x4A, 0x51, 0x40,
                                                0x00, 0x00, 0xFF, 0x01, 0x03,
                                                0xE0, 0x80, 0xFF, 0x00, 0x00,
                                                0x08, 0x08, 0x6B, 0x6B, 0x08,
                                                0x36, 0x12, 0x36, 0x24, 0x36,
                                                0x06, 0x0F, 0x09, 0x0F, 0x06,
                                                0x00, 0x00, 0x18, 0x18, 0x00,
                                                0x00, 0x00, 0x10, 0x10, 0x00,
                                                0x30, 0x40, 0xFF, 0x01, 0x01,
                                                0x00, 0x1F, 0x01, 0x01, 0x1E,
                                                0x00, 0x19, 0x1D, 0x17, 0x12,
                                                0x00, 0x3C, 0x3C, 0x3C, 0x3C,
                                                0x00, 0x00, 0x00, 0x00, 0x00  // #255 NBSP
};

/****************************************************************
 * Function Name : clearDisplay
 * Description   : Clear the display memory buffer
 * Returns       : NONE.
 * Params        : NONE.
 ****************************************************************/
void clearDisplay()
{
    memset(screen, 0x00, DISPLAY_BUFF_SIZE);
}

/****************************************************************
 * Function Name : display_Init_seq
 * Description   : Performs SSD1306 OLED Initialization Sequence
 * Returns       : NONE.
 * Params        : NONE.
 ****************************************************************/
void display_Init_seq()
{
    /* Add the reset code, If needed */

    /* Send display OFF command */
    if(i2c_write_register(I2C_DEV_2.fd_i2c, SSD1306_CNTRL_CMD, SSD1306_DISPLAY_OFF) == I2C_TWO_BYTES)
    {
#ifdef SSD1306_DBG
        printf("Display OFF Command Passed\r\n");
#endif
    }
    else
    {
#ifdef SSD1306_DBG
        printf("Display OFF Command Failed\r\n");
#endif
        exit(1);
    }

    /* Set display clock frequency */
    if(i2c_write_register(I2C_DEV_2.fd_i2c, SSD1306_CNTRL_CMD, SSD1306_SET_DISP_CLK) == I2C_TWO_BYTES)
    {
#ifdef SSD1306_DBG
        printf("Display CLK Command Passed\r\n");
#endif
    }
    else
    {
#ifdef SSD1306_DBG
        printf("Display CLK Command Failed\r\n");
#endif
        exit(1);
    }

    /* Send display CLK command parameter */
    if(i2c_write_register(I2C_DEV_2.fd_i2c, SSD1306_CNTRL_CMD, SSD1306_DISPCLK_DIV) == I2C_TWO_BYTES)
    {
#ifdef SSD1306_DBG
        printf("Display CLK Command Parameter Passed\r\n");
#endif
    }
    else
    {
#ifdef SSD1306_DBG
        printf("Display CLK Command Parameter Failed\r\n");
#endif
        exit(1);
    }

    /* Set display multiplex */
    if(i2c_write_register(I2C_DEV_2.fd_i2c, SSD1306_CNTRL_CMD, SSD1306_SET_MULTIPLEX) == I2C_TWO_BYTES)
    {
#ifdef SSD1306_DBG
        printf("Display MULT Command Passed\r\n");
#endif
    }
    else
    {
#ifdef SSD1306_DBG
        printf("Display MULT Command Failed\r\n");
#endif
        exit(1);
    }

    /* Send display MULT command parameter */
    if(i2c_write_register(I2C_DEV_2.fd_i2c, SSD1306_CNTRL_CMD, SSD1306_MULT_DAT) == I2C_TWO_BYTES)
    {
#ifdef SSD1306_DBG
        printf("Display MULT Command Parameter Passed\r\n");
#endif
    }
    else
    {
#ifdef SSD1306_DBG
        printf("Display MULT Command Parameter Failed\r\n");
#endif
        exit(1);
    }

    /* Set display OFFSET */
    if(i2c_write_register(I2C_DEV_2.fd_i2c, SSD1306_CNTRL_CMD, SSD1306_SET_DISP_OFFSET) == I2C_TWO_BYTES)
    {
#ifdef SSD1306_DBG
        printf("Display OFFSET Command Passed\r\n");
#endif
    }
    else
    {
#ifdef SSD1306_DBG
        printf("Display OFFSET Command Failed\r\n");
#endif
        exit(1);
    }

    /* Send display OFFSET command parameter */
    if(i2c_write_register(I2C_DEV_2.fd_i2c, SSD1306_CNTRL_CMD, SSD1306_DISP_OFFSET_VAL) == I2C_TWO_BYTES)
    {
#ifdef SSD1306_DBG
        printf("Display OFFSET Command Parameter Passed\r\n");
#endif
    }
    else
    {
#ifdef SSD1306_DBG
        printf("Display OFFSET Command Parameter Failed\r\n");
#endif
        exit(1);
    }

    /* Set display START LINE - Check this command if something weird happens */
    if(i2c_write_register(I2C_DEV_2.fd_i2c, SSD1306_CNTRL_CMD, SSD1306_SET_DISP_START_LINE) == I2C_TWO_BYTES)
    {
#ifdef SSD1306_DBG
        printf("Display START LINE Command Passed\r\n");
#endif
    }
    else
    {
#ifdef SSD1306_DBG
        printf("Display START LINE Command Failed\r\n");
#endif
        exit(1);
    }

    /* Enable CHARGEPUMP*/
    if(i2c_write_register(I2C_DEV_2.fd_i2c, SSD1306_CNTRL_CMD, SSD1306_CONFIG_CHARGE_PUMP) == I2C_TWO_BYTES)
    {
#ifdef SSD1306_DBG
        printf("Display CHARGEPUMP Command Passed\r\n");
#endif
    }
    else
    {
#ifdef SSD1306_DBG
        printf("Display CHARGEPUMP Command Failed\r\n");
#endif
        exit(1);
    }

    /* Send display CHARGEPUMP command parameter */
    if(i2c_write_register(I2C_DEV_2.fd_i2c, SSD1306_CNTRL_CMD, SSD1306_CHARGE_PUMP_EN) == I2C_TWO_BYTES)
    {
#ifdef SSD1306_DBG
        printf("Display CHARGEPUMP Command Parameter Passed\r\n");
#endif
    }
    else
    {
#ifdef SSD1306_DBG
        printf("Display CHARGEPUMP Command Parameter Failed\r\n");
#endif
        exit(1);
    }

    /* Set display MEMORYMODE */
    if(i2c_write_register(I2C_DEV_2.fd_i2c, SSD1306_CNTRL_CMD, SSD1306_SET_MEM_ADDR_MODE) == I2C_TWO_BYTES)
    {
#ifdef SSD1306_DBG
        printf("Display MEMORYMODE Command Passed\r\n");
#endif
    }
    else
    {
#ifdef SSD1306_DBG
        printf("Display MEMORYMODE Command Failed\r\n");
#endif
        exit(1);
    }

    /* Send display HORIZONTAL MEMORY ADDR MODE command parameter */
    if(i2c_write_register(I2C_DEV_2.fd_i2c, SSD1306_CNTRL_CMD, SSD1306_HOR_MM) == I2C_TWO_BYTES)
    {
#ifdef SSD1306_DBG
        printf("Display HORIZONTAL MEMORY ADDR MODE Command Parameter Passed\r\n");
#endif
    }
    else
    {
#ifdef SSD1306_DBG
        printf("Display HORIZONTAL MEMORY ADDR MODE Command Parameter Failed\r\n");
#endif
        exit(1);
    }

    /* Set display SEG_REMAP */
    if(i2c_write_register(I2C_DEV_2.fd_i2c, SSD1306_CNTRL_CMD, SSD1306_SEG_REMAP) == I2C_TWO_BYTES)
    {
#ifdef SSD1306_DBG
        printf("Display SEG_REMAP Command Passed\r\n");
#endif
    }
    else
    {
#ifdef SSD1306_DBG
        printf("Display SEG_REMAP Command Failed\r\n");
#endif
        exit(1);
    }

    /* Set display DIR */
    if(i2c_write_register(I2C_DEV_2.fd_i2c, SSD1306_CNTRL_CMD, SSD1306_SET_COMSCANDEC) == I2C_TWO_BYTES)
    {
#ifdef SSD1306_DBG
        printf("Display DIR Command Passed\r\n");
#endif
    }
    else
    {
#ifdef SSD1306_DBG
        printf("Display DIR Command Failed\r\n");
#endif
        exit(1);
    }

    /* Set display COM */
    if(i2c_write_register(I2C_DEV_2.fd_i2c, SSD1306_CNTRL_CMD, SSD1306_SET_COMPINS) == I2C_TWO_BYTES)
    {
#ifdef SSD1306_DBG
        printf("Display COM Command Passed\r\n");
#endif
    }
    else
    {
#ifdef SSD1306_DBG
        printf("Display COM Command Failed\r\n");
#endif
        exit(1);
    }

    /* Send display COM command parameter */
    if(i2c_write_register(I2C_DEV_2.fd_i2c, SSD1306_CNTRL_CMD, SSD1306_CONFIG_COM_PINS) == I2C_TWO_BYTES)
    {
#ifdef SSD1306_DBG
        printf("Display COM Command Parameter Passed\r\n");
#endif
    }
    else
    {
#ifdef SSD1306_DBG
        printf("Display COM Command Parameter Failed\r\n");
#endif
        exit(1);
    }

    /* Set display CONTRAST */
    if(i2c_write_register(I2C_DEV_2.fd_i2c, SSD1306_CNTRL_CMD, SSD1306_SET_CONTRAST) == I2C_TWO_BYTES)
    {
#ifdef SSD1306_DBG
        printf("Display CONTRAST Command Passed\r\n");
#endif
    }
    else
    {
#ifdef SSD1306_DBG
        printf("Display CONTRAST Command Failed\r\n");
#endif
        exit(1);
    }

    /* Send display CONTRAST command parameter */
    if(i2c_write_register(I2C_DEV_2.fd_i2c, SSD1306_CNTRL_CMD, SSD1306_CONTRAST_VAL) == I2C_TWO_BYTES)
    {
#ifdef SSD1306_DBG
        printf("Display CONTRAST Command Parameter Passed\r\n");
#endif
    }
    else
    {
#ifdef SSD1306_DBG
        printf("Display CONTRAST Command Parameter Failed\r\n");
#endif
        exit(1);
    }

    /* Set display PRECHARGE */
    if(i2c_write_register(I2C_DEV_2.fd_i2c, SSD1306_CNTRL_CMD, SSD1306_SET_PRECHARGE) == I2C_TWO_BYTES)
    {
#ifdef SSD1306_DBG
        printf("Display PRECHARGE Command Passed\r\n");
#endif
    }
    else
    {
#ifdef SSD1306_DBG
        printf("Display PRECHARGE Command Failed\r\n");
#endif
        exit(1);
    }

    /* Send display PRECHARGE command parameter */
    if(i2c_write_register(I2C_DEV_2.fd_i2c, SSD1306_CNTRL_CMD, SSD1306_PRECHARGE_VAL) == I2C_TWO_BYTES)
    {
#ifdef SSD1306_DBG
        printf("Display PRECHARGE Command Parameter Passed\r\n");
#endif
    }
    else
    {
#ifdef SSD1306_DBG
        printf("Display PRECHARGE Command Parameter Failed\r\n");
#endif
        exit(1);
    }

    /* Set display VCOMH */
    if(i2c_write_register(I2C_DEV_2.fd_i2c, SSD1306_CNTRL_CMD, SSD1306_SET_VCOMDETECT) == I2C_TWO_BYTES)
    {
#ifdef SSD1306_DBG
        printf("Display VCOMH Command Passed\r\n");
#endif
    }
    else
    {
#ifdef SSD1306_DBG
        printf("Display VCOMH Command Failed\r\n");
#endif
        exit(1);
    }

    /* Send display VCOMH command parameter */
    if(i2c_write_register(I2C_DEV_2.fd_i2c, SSD1306_CNTRL_CMD, SSD1306_VCOMH_VAL) == I2C_TWO_BYTES)
    {
#ifdef SSD1306_DBG
        printf("Display VCOMH Command Parameter Passed\r\n");
#endif
    }
    else
    {
#ifdef SSD1306_DBG
        printf("Display VCOMH Command Parameter Failed\r\n");
#endif
        exit(1);
    }

    /* Set display ALL-ON */
    if(i2c_write_register(I2C_DEV_2.fd_i2c, SSD1306_CNTRL_CMD, SSD1306_DISPLAYALLON_RESUME) == I2C_TWO_BYTES)
    {
#ifdef SSD1306_DBG
        printf("Display ALL-ON Command Passed\r\n");
#endif
    }
    else
    {
#ifdef SSD1306_DBG
        printf("Display ALL-ON Command Failed\r\n");
#endif
        exit(1);
    }

    /* Set display to NORMAL-DISPLAY */
    if(i2c_write_register(I2C_DEV_2.fd_i2c, SSD1306_CNTRL_CMD, SSD1306_NORMAL_DISPLAY) == I2C_TWO_BYTES)
    {
#ifdef SSD1306_DBG
        printf("Display NORMAL-DISPLAY Command Passed\r\n");
#endif
    }
    else
    {
#ifdef SSD1306_DBG
        printf("Display NORMAL-DISPLAY Command Failed\r\n");
#endif
        exit(1);
    }

    /* Set display to DEACTIVATE_SCROLL */
    if(i2c_write_register(I2C_DEV_2.fd_i2c, SSD1306_CNTRL_CMD, SSD1306_DEACTIVATE_SCROLL) == I2C_TWO_BYTES)
    {
#ifdef SSD1306_DBG
        printf("Display DEACTIVATE_SCROLL Command Passed\r\n");
#endif
    }
    else
    {
#ifdef SSD1306_DBG
        printf("Display DEACTIVATE_SCROLL Command Failed\r\n");
#endif
        exit(1);
    }

    /* Set display to TURN-ON */
    if(i2c_write_register(I2C_DEV_2.fd_i2c, SSD1306_CNTRL_CMD, SSD1306_DISPLAYON) == I2C_TWO_BYTES)
    {
#ifdef SSD1306_DBG
        printf("Display TURN-ON Command Passed\r\n");
#endif
    }
    else
    {
#ifdef SSD1306_DBG
        printf("Display TURN-ON Command Failed\r\n");
#endif
        exit(1);
    }
}

/****************************************************************
 * Function Name : transfer
 * Description   : Transfer the frame buffer onto the display
 * Returns       : NONE.
 * Params        : NONE.
 ****************************************************************/
void transfer()
{
    short loop_1 = 0, loop_2 = 0;
    short index = 0x00;
    for (loop_1 = 0; loop_1 < 1024; loop_1++)
    {
        chunk[0] = 0x40;
        for(loop_2 = 1; loop_2 < 17; loop_2++)
            chunk[loop_2] = screen[index++];
        if(i2c_multiple_writes(I2C_DEV_2.fd_i2c, 17, chunk) == 17)
        {
#ifdef SSD1306_DBG
            printf("Chunk written to RAM - Completed\r\n");
#endif
        }
        else
        {
#ifdef SSD1306_DBG
            printf("Chunk written to RAM - Failed\r\n");
#endif
            exit(1);
        }

        memset(chunk,0x00,17);
        if(index == 1024)
            break;
    }
}


/****************************************************************
 * Function Name : Display
 * Description   : 1. Resets the column and page addresses.
 *                 2. Displays the contents of the memory buffer.
 * Returns       : NONE.
 * Params        : NONE.
 * Note          : Each new form can be preceded by a clearDisplay.
 ****************************************************************/
void Display()
{
    Init_Col_PG_addrs(SSD1306_COL_START_ADDR,SSD1306_COL_END_ADDR,
                      SSD1306_PG_START_ADDR,SSD1306_PG_END_ADDR);
    transfer();
}

/****************************************************************
 * Function Name : Init_Col_PG_addrs
 * Description   : Sets the column and page, start and
 *                 end addresses.
 * Returns       : NONE.
 * Params        : @col_start_addr: Column start address
 *                 @col_end_addr: Column end address
 *                 @pg_start_addr: Page start address
 *                 @pg_end_addr: Page end address
 ****************************************************************/
void Init_Col_PG_addrs(unsigned char col_start_addr, unsigned char col_end_addr,
                       unsigned char pg_start_addr, unsigned char pg_end_addr)
{
    /* Send COLMN address setting command  */
    if(i2c_write_register(I2C_DEV_2.fd_i2c, SSD1306_CNTRL_CMD, SSD1306_SET_COL_ADDR) == I2C_TWO_BYTES)
    {
#ifdef SSD1306_DBG
        printf("Display COLMN Command Passed\r\n");
#endif
    }
    else
    {
#ifdef SSD1306_DBG
        printf("Display COLMN Command Failed\r\n");
#endif
        exit(1);
    }

    /* Set COLMN start address */
    if(i2c_write_register(I2C_DEV_2.fd_i2c, SSD1306_CNTRL_CMD, col_start_addr) == I2C_TWO_BYTES)
    {
#ifdef SSD1306_DBG
        printf("Display COLMN Start Address param Passed\r\n");
#endif
    }
    else
    {
#ifdef SSD1306_DBG
        printf("Display COLMN Start Address param  Failed\r\n");
#endif
        exit(1);
    }

    /* Set COLMN end address */
    if(i2c_write_register(I2C_DEV_2.fd_i2c, SSD1306_CNTRL_CMD, col_end_addr) == I2C_TWO_BYTES)
    {
#ifdef SSD1306_DBG
        printf("Display COLMN End Address param Passed\r\n");
#endif
    }
    else
    {
#ifdef SSD1306_DBG
        printf("Display COLMN End Address param  Failed\r\n");
#endif
        exit(1);
    }

    /* Send PAGE address setting command  */
    if(i2c_write_register(I2C_DEV_2.fd_i2c, SSD1306_CNTRL_CMD, SSD1306_PAGEADDR) == I2C_TWO_BYTES)
    {
#ifdef SSD1306_DBG
        printf("Display PAGE Command Passed\r\n");
#endif
    }
    else
    {
#ifdef SSD1306_DBG
        printf("Display PAGE Command Failed\r\n");
#endif
        exit(1);
    }

    /* Set PAGE start address */
    if(i2c_write_register(I2C_DEV_2.fd_i2c, SSD1306_CNTRL_CMD, pg_start_addr) == I2C_TWO_BYTES)
    {
#ifdef SSD1306_DBG
        printf("Display PAGE Start Address param Passed\r\n");
#endif
    }
    else
    {
#ifdef SSD1306_DBG
        printf("Display PAGE Start Address param  Failed\r\n");
#endif
        exit(1);
    }

    /* Set PAGE end address */
    if(i2c_write_register(I2C_DEV_2.fd_i2c, SSD1306_CNTRL_CMD, pg_end_addr) == I2C_TWO_BYTES)
    {
#ifdef SSD1306_DBG
        printf("Display PAGE End Address param Passed\r\n");
#endif
    }
    else
    {
#ifdef SSD1306_DBG
        printf("Display PAGE End Address param  Failed\r\n");
#endif
        exit(1);
    }
}

/****************************************************************
 * Function Name : setRotation
 * Description   : Set the display rotation
 * Returns       : NONE.
 * Params        : @x: Display rotation parameter
 ****************************************************************/
void setRotation(unsigned char x)
{
    _rotation = x & 3;
    switch(_rotation)
    {
    case 0:
    case 2:
        _width = SSD1306_LCDWIDTH;
        _height = SSD1306_LCDHEIGHT;
        break;
    case 1:
    case 3:
        _width = SSD1306_LCDHEIGHT;
        _height = SSD1306_LCDWIDTH;
        break;
    }
}

/****************************************************************
 * Function Name : startscrollright
 * Description   : Activate a right handed scroll for rows start
 *                 through stop
 * Returns       : NONE.
 * Params        : @start: Start location
 *                 @stop: Stop location
 * HINT.         : the display is 16 rows tall. To scroll the whole
 *                 display, run: display.scrollright(0x00, 0x0F)
 ****************************************************************/
void startscrollright(unsigned char start, unsigned char stop)
{
    /* Send SCROLL horizontal right command  */
    if(i2c_write_register(I2C_DEV_2.fd_i2c, SSD1306_CNTRL_CMD, SSD1306_RIGHT_HORIZONTAL_SCROLL) == I2C_TWO_BYTES)
    {
#ifdef SSD1306_DBG
        printf("Display HORIZONTAL SCROLL RIGHT Command Passed\r\n");
#endif
    }
    else
    {
#ifdef SSD1306_DBG
        printf("Display HORIZONTAL SCROLL RIGHT Command Failed\r\n");
#endif
        exit(1);
    }


    if(i2c_write_register(I2C_DEV_2.fd_i2c, SSD1306_CNTRL_CMD, 0x00) == I2C_TWO_BYTES)
    {
#ifdef SSD1306_DBG
        printf("HORI_SR Param_1 Passed\r\n");
#endif
    }
    else
    {
#ifdef SSD1306_DBG
        printf("HORI_SR Param_1 Failed\r\n");
#endif
        exit(1);
    }

    if(i2c_write_register(I2C_DEV_2.fd_i2c, SSD1306_CNTRL_CMD, start) == I2C_TWO_BYTES)
    {
#ifdef SSD1306_DBG
        printf("HORI_SR Param_2 Passed\r\n");
#endif
    }
    else
    {
#ifdef SSD1306_DBG
        printf("HORI_SR Param_2 Passed\r\n");
#endif
        exit(1);
    }

    if(i2c_write_register(I2C_DEV_2.fd_i2c, SSD1306_CNTRL_CMD, 0x00) == I2C_TWO_BYTES)
    {
#ifdef SSD1306_DBG
        printf("HORI_SR Param_3 Passed\r\n");
#endif
    }
    else
    {
#ifdef SSD1306_DBG
        printf("HORI_SR Param_3 Failed\r\n");
#endif
        exit(1);
    }

    if(i2c_write_register(I2C_DEV_2.fd_i2c, SSD1306_CNTRL_CMD, stop) == I2C_TWO_BYTES)
    {
#ifdef SSD1306_DBG
        printf("HORI_SR Param_4 Passed\r\n");
#endif
    }
    else
    {
#ifdef SSD1306_DBG
        printf("HORI_SR Param_4 Passed\r\n");
#endif
        exit(1);
    }

    if(i2c_write_register(I2C_DEV_2.fd_i2c, SSD1306_CNTRL_CMD, 0x00) == I2C_TWO_BYTES)
    {
#ifdef SSD1306_DBG
        printf("HORI_SR Param_5 Passed\r\n");
#endif
    }
    else
    {
#ifdef SSD1306_DBG
        printf("HORI_SR Param_5 Failed\r\n");
#endif
        exit(1);
    }

    if(i2c_write_register(I2C_DEV_2.fd_i2c, SSD1306_CNTRL_CMD, 0xFF) == I2C_TWO_BYTES)
    {
#ifdef SSD1306_DBG
        printf("HORI_SR Param_6 Passed\r\n");
#endif
    }
    else
    {
#ifdef SSD1306_DBG
        printf("HORI_SR Param_6 Passed\r\n");
#endif
        exit(1);
    }
    /* Send SCROLL Activate command  */
    if(i2c_write_register(I2C_DEV_2.fd_i2c, SSD1306_CNTRL_CMD, SSD1306_ACTIVATE_SCROLL) == I2C_TWO_BYTES)
    {
#ifdef SSD1306_DBG
        printf("SCROLL Activate Command Passed\r\n");
#endif
    }
    else
    {
#ifdef SSD1306_DBG
        printf("SCROLL Activate Command Failed\r\n");
#endif
        exit(1);
    }
}

/****************************************************************
 * Function Name : startscrollleft
 * Description   : Activate a left handed scroll for rows start
 *                 through stop
 * Returns       : NONE.
 * Params        : @start: Start location
 *                 @stop: Stop location
 * HINT.         : the display is 16 rows tall. To scroll the whole
 *                 display, run: display.scrollright(0x00, 0x0F)
 ****************************************************************/
void startscrollleft(unsigned char start, unsigned char stop)
{
    /* Send SCROLL horizontal left command  */
    if(i2c_write_register(I2C_DEV_2.fd_i2c, SSD1306_CNTRL_CMD, SSD1306_LEFT_HORIZONTAL_SCROLL) == I2C_TWO_BYTES)
    {
#ifdef SSD1306_DBG
        printf("Display HORIZONTAL SCROLL LEFT Command Passed\r\n");
#endif
    }
    else
    {
#ifdef SSD1306_DBG
        printf("Display HORIZONTAL SCROLL LEFT Command Failed\r\n");
#endif
        exit(1);
    }


    if(i2c_write_register(I2C_DEV_2.fd_i2c, SSD1306_CNTRL_CMD, 0x00) == I2C_TWO_BYTES)
    {
#ifdef SSD1306_DBG
        printf("HORI_SR Param_1 Passed\r\n");
#endif
    }
    else
    {
#ifdef SSD1306_DBG
        printf("HORI_SR Param_1 Failed\r\n");
#endif
        exit(1);
    }

    if(i2c_write_register(I2C_DEV_2.fd_i2c, SSD1306_CNTRL_CMD, start) == I2C_TWO_BYTES)
    {
#ifdef SSD1306_DBG
        printf("HORI_SR Param_2 Passed\r\n");
#endif
    }
    else
    {
#ifdef SSD1306_DBG
        printf("HORI_SR Param_2 Passed\r\n");
#endif
        exit(1);
    }

    if(i2c_write_register(I2C_DEV_2.fd_i2c, SSD1306_CNTRL_CMD, 0x00) == I2C_TWO_BYTES)
    {
#ifdef SSD1306_DBG
        printf("HORI_SR Param_3 Passed\r\n");
#endif
    }
    else
    {
#ifdef SSD1306_DBG
        printf("HORI_SR Param_3 Failed\r\n");
#endif
        exit(1);
    }

    if(i2c_write_register(I2C_DEV_2.fd_i2c, SSD1306_CNTRL_CMD, stop) == I2C_TWO_BYTES)
    {
#ifdef SSD1306_DBG
        printf("HORI_SR Param_4 Passed\r\n");
#endif
    }
    else
    {
#ifdef SSD1306_DBG
        printf("HORI_SR Param_4 Passed\r\n");
#endif
        exit(1);
    }

    if(i2c_write_register(I2C_DEV_2.fd_i2c, SSD1306_CNTRL_CMD, 0x00) == I2C_TWO_BYTES)
    {
#ifdef SSD1306_DBG
        printf("HORI_SR Param_5 Passed\r\n");
#endif
    }
    else
    {
#ifdef SSD1306_DBG
        printf("HORI_SR Param_5 Failed\r\n");
#endif
        exit(1);
    }

    if(i2c_write_register(I2C_DEV_2.fd_i2c, SSD1306_CNTRL_CMD, 0xFF) == I2C_TWO_BYTES)
    {
#ifdef SSD1306_DBG
        printf("HORI_SR Param_6 Passed\r\n");
#endif
    }
    else
    {
#ifdef SSD1306_DBG
        printf("HORI_SR Param_6 Passed\r\n");
#endif
        exit(1);
    }
    /* Send SCROLL Activate command  */
    if(i2c_write_register(I2C_DEV_2.fd_i2c, SSD1306_CNTRL_CMD, SSD1306_ACTIVATE_SCROLL) == I2C_TWO_BYTES)
    {
#ifdef SSD1306_DBG
        printf("SCROLL Activate Command Passed\r\n");
#endif
    }
    else
    {
#ifdef SSD1306_DBG
        printf("SCROLL Activate Command Failed\r\n");
#endif
        exit(1);
    }
}

/****************************************************************
 * Function Name : startscrolldiagright
 * Description   : Activate a diagonal scroll for rows start
 *                 through stop
 * Returns       : NONE.
 * Params        : @start: Start location
 *                 @stop: Stop location
 * HINT.         : the display is 16 rows tall. To scroll the whole
 *                 display, run: display.scrollright(0x00, 0x0F)
 ****************************************************************/
void startscrolldiagright(unsigned char start, unsigned char stop)
{
    /* Send SCROLL diagonal right command  */
    if(i2c_write_register(I2C_DEV_2.fd_i2c, SSD1306_CNTRL_CMD, SSD1306_SET_VERTICAL_SCROLL_AREA) == I2C_TWO_BYTES)
    {
#ifdef SSD1306_DBG
        printf("Display DIAGONAL SCROLL RIGHT Command Passed\r\n");
#endif
    }
    else
    {
#ifdef SSD1306_DBG
        printf("Display DIAGONAL SCROLL RIGHT Command Failed\r\n");
#endif
        exit(1);
    }


    if(i2c_write_register(I2C_DEV_2.fd_i2c, SSD1306_CNTRL_CMD, 0x00) == I2C_TWO_BYTES)
    {
#ifdef SSD1306_DBG
        printf("DIAG_SR Param_1 Passed\r\n");
#endif
    }
    else
    {
#ifdef SSD1306_DBG
        printf("DIAG_SR Param_1 Failed\r\n");
#endif
        exit(1);
    }

    if(i2c_write_register(I2C_DEV_2.fd_i2c, SSD1306_CNTRL_CMD, SSD1306_LCDHEIGHT) == I2C_TWO_BYTES)
    {
#ifdef SSD1306_DBG
        printf("DIAG_SR Param_2 Passed\r\n");
#endif
    }
    else
    {
#ifdef SSD1306_DBG
        printf("DIAG_SR Param_2 Failed\r\n");
#endif
        exit(1);
    }

    if(i2c_write_register(I2C_DEV_2.fd_i2c, SSD1306_CNTRL_CMD, SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL) == I2C_TWO_BYTES)
    {
#ifdef SSD1306_DBG
        printf("Cmd Passed\r\n");
#endif
    }
    else
    {
#ifdef SSD1306_DBG
        printf("Cmd Failed\r\n");
#endif
        exit(1);
    }

    if(i2c_write_register(I2C_DEV_2.fd_i2c, SSD1306_CNTRL_CMD, 0x00) == I2C_TWO_BYTES)
    {
#ifdef SSD1306_DBG
        printf("DIAG_SR Param_3 Passed\r\n");
#endif
    }
    else
    {
#ifdef SSD1306_DBG
        printf("DIAG_SR Param_3 Failed\r\n");
#endif
        exit(1);
    }

    if(i2c_write_register(I2C_DEV_2.fd_i2c, SSD1306_CNTRL_CMD, start) == I2C_TWO_BYTES)
    {
#ifdef SSD1306_DBG
        printf("DIAG_SR Param_4 Passed\r\n");
#endif
    }
    else
    {
#ifdef SSD1306_DBG
        printf("DIAG_SR Param_4 Failed\r\n");
#endif
        exit(1);
    }

    if(i2c_write_register(I2C_DEV_2.fd_i2c, SSD1306_CNTRL_CMD, 0x00) == I2C_TWO_BYTES)
    {
#ifdef SSD1306_DBG
        printf("DIAG_SR Param_5 Passed\r\n");
#endif
    }
    else
    {
#ifdef SSD1306_DBG
        printf("DIAG_SR Param_5 Failed\r\n");
#endif
        exit(1);
    }

    if(i2c_write_register(I2C_DEV_2.fd_i2c, SSD1306_CNTRL_CMD, stop) == I2C_TWO_BYTES)
    {
#ifdef SSD1306_DBG
        printf("DIAG_SR Param_6 Passed\r\n");
#endif
    }
    else
    {
#ifdef SSD1306_DBG
        printf("DIAG_SR Param_6 Failed\r\n");
#endif
        exit(1);
    }

    if(i2c_write_register(I2C_DEV_2.fd_i2c, SSD1306_CNTRL_CMD, 0x01) == I2C_TWO_BYTES)
    {
#ifdef SSD1306_DBG
        printf("DIAG_SR Param_5 Passed\r\n");
#endif
    }
    else
    {
#ifdef SSD1306_DBG
        printf("DIAG_SR Param_5 Failed\r\n");
#endif
        exit(1);
    }

    /* Send SCROLL Activate command  */
    if(i2c_write_register(I2C_DEV_2.fd_i2c, SSD1306_CNTRL_CMD, SSD1306_ACTIVATE_SCROLL) == I2C_TWO_BYTES)
    {
#ifdef SSD1306_DBG
        printf("SCROLL Activate Command Passed\r\n");
#endif
    }
    else
    {
#ifdef SSD1306_DBG
        printf("SCROLL Activate Command Failed\r\n");
#endif
        exit(1);
    }
}

/****************************************************************
 * Function Name : startscrolldiagleft
 * Description   : Activate a diagonal scroll for rows start
 *                 through stop
 * Returns       : NONE.
 * Params        : @start: Start location
 *                 @stop: Stop location
 * HINT.         : the display is 16 rows tall. To scroll the whole
 *                 display, run: display.scrollright(0x00, 0x0F)
 ****************************************************************/
void startscrolldiagleft(unsigned char start, unsigned char stop)
{
    /* Send SCROLL diagonal right command  */
    if(i2c_write_register(I2C_DEV_2.fd_i2c, SSD1306_CNTRL_CMD, SSD1306_SET_VERTICAL_SCROLL_AREA) == I2C_TWO_BYTES)
    {
#ifdef SSD1306_DBG
        printf("Display DIAGONAL SCROLL RIGHT Command Passed\r\n");
#endif
    }
    else
    {
#ifdef SSD1306_DBG
        printf("Display DIAGONAL SCROLL RIGHT Command Failed\r\n");
#endif
        exit(1);
    }


    if(i2c_write_register(I2C_DEV_2.fd_i2c, SSD1306_CNTRL_CMD, 0x00) == I2C_TWO_BYTES)
    {
#ifdef SSD1306_DBG
        printf("DIAG_SR Param_1 Passed\r\n");
#endif
    }
    else
    {
#ifdef SSD1306_DBG
        printf("DIAG_SR Param_1 Failed\r\n");
#endif
        exit(1);
    }

    if(i2c_write_register(I2C_DEV_2.fd_i2c, SSD1306_CNTRL_CMD, SSD1306_LCDHEIGHT) == I2C_TWO_BYTES)
    {
#ifdef SSD1306_DBG
        printf("DIAG_SR Param_2 Passed\r\n");
#endif
    }
    else
    {
#ifdef SSD1306_DBG
        printf("DIAG_SR Param_2 Failed\r\n");
#endif
        exit(1);
    }

    if(i2c_write_register(I2C_DEV_2.fd_i2c, SSD1306_CNTRL_CMD, SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL) == I2C_TWO_BYTES)
    {
#ifdef SSD1306_DBG
        printf("Cmd Passed\r\n");
#endif
    }
    else
    {
#ifdef SSD1306_DBG
        printf("Cmd Failed\r\n");
#endif
        exit(1);
    }

    if(i2c_write_register(I2C_DEV_2.fd_i2c, SSD1306_CNTRL_CMD, 0x00) == I2C_TWO_BYTES)
    {
#ifdef SSD1306_DBG
        printf("DIAG_SR Param_3 Passed\r\n");
#endif
    }
    else
    {
#ifdef SSD1306_DBG
        printf("DIAG_SR Param_3 Failed\r\n");
#endif
        exit(1);
    }

    if(i2c_write_register(I2C_DEV_2.fd_i2c, SSD1306_CNTRL_CMD, start) == I2C_TWO_BYTES)
    {
#ifdef SSD1306_DBG
        printf("DIAG_SR Param_4 Passed\r\n");
#endif
    }
    else
    {
#ifdef SSD1306_DBG
        printf("DIAG_SR Param_4 Failed\r\n");
#endif
        exit(1);
    }

    if(i2c_write_register(I2C_DEV_2.fd_i2c, SSD1306_CNTRL_CMD, 0x00) == I2C_TWO_BYTES)
    {
#ifdef SSD1306_DBG
        printf("DIAG_SR Param_5 Passed\r\n");
#endif
    }
    else
    {
#ifdef SSD1306_DBG
        printf("DIAG_SR Param_5 Failed\r\n");
#endif
        exit(1);
    }

    if(i2c_write_register(I2C_DEV_2.fd_i2c, SSD1306_CNTRL_CMD, stop) == I2C_TWO_BYTES)
    {
#ifdef SSD1306_DBG
        printf("DIAG_SR Param_6 Passed\r\n");
#endif
    }
    else
    {
#ifdef SSD1306_DBG
        printf("DIAG_SR Param_6 Failed\r\n");
#endif
        exit(1);
    }

    if(i2c_write_register(I2C_DEV_2.fd_i2c, SSD1306_CNTRL_CMD, 0x01) == I2C_TWO_BYTES)
    {
#ifdef SSD1306_DBG
        printf("DIAG_SR Param_5 Passed\r\n");
#endif
    }
    else
    {
#ifdef SSD1306_DBG
        printf("DIAG_SR Param_5 Failed\r\n");
#endif
        exit(1);
    }

    /* Send SCROLL Activate command  */
    if(i2c_write_register(I2C_DEV_2.fd_i2c, SSD1306_CNTRL_CMD, SSD1306_ACTIVATE_SCROLL) == I2C_TWO_BYTES)
    {
#ifdef SSD1306_DBG
        printf("SCROLL Activate Command Passed\r\n");
#endif
    }
    else
    {
#ifdef SSD1306_DBG
        printf("SCROLL Activate Command Failed\r\n");
#endif
        exit(1);
    }
}

/****************************************************************
 * Function Name : stopscroll
 * Description   : Stop scrolling
 * Returns       : NONE.
 * Params        : NONE.
 ****************************************************************/
void stopscroll()
{
    if(i2c_write_register(I2C_DEV_2.fd_i2c, SSD1306_CNTRL_CMD, SSD1306_DEACTIVATE_SCROLL) == I2C_TWO_BYTES)
    {
        printf("De-activate SCROLL Command Passed\r\n");
    }
    else
    {
        printf("De-activate SCROLL Command Passed Failed\r\n");
        exit(1);
    }
}

/****************************************************************
 * Function Name : invertDisplay
 * Description   : Invert or Normalize the display
 * Returns       : NONE.
 * Params        : @i: 0x00 to Normal and 0x01 for Inverting
 ****************************************************************/
void invertDisplay(unsigned char i)
{
    if (i)
    {
        if(i2c_write_register(I2C_DEV_2.fd_i2c, SSD1306_CNTRL_CMD, SSD1306_INVERTDISPLAY) == I2C_TWO_BYTES)
        {
            printf("Display Inverted - Passed\r\n");
        }
        else
        {
            printf("Display Inverted - Failed\r\n");
            exit(1);
        }
    }
    else
    {
        if(i2c_write_register(I2C_DEV_2.fd_i2c, SSD1306_CNTRL_CMD, SSD1306_NORMAL_DISPLAY) == I2C_TWO_BYTES)
        {
            printf("Display Normal - Passed\r\n");
        }
        else
        {
            printf("Display Normal - Failed\r\n");
            exit(1);
        }
    }
}

/****************************************************************
 * Function Name : drawPixel
 * Description   : Draw a pixel
 * Returns       : -1 on error and 0 on success
 * Params        : @x: X - Co-ordinate
 *                 @y: Y - Co-ordinate
 *                 @color: Color
 ****************************************************************/
signed char drawPixel(short x, short y, short color)
{
    /* Return if co-ordinates are out of display dimension's range */
    if ((x < 0) || (x >= _width) || (y < 0) || (y >= _height))
        return -1;
    switch(_rotation)
    {
    case 1:
        SWAP(x,y);
        x = _width - x - 1;
        break;
    case 2:
        x = _width - x - 1;
        y = _height - y - 1;
        break;
    case 3:
        SWAP(x,y);
        y = _height - y - 1;
        break;
    }

    /* x is the column */
    switch(color)
    {
    case WHITE:   screen[x+ (y/8)*SSD1306_LCDWIDTH] |=  (1 << (y&7)); break;
    case BLACK:   screen[x+ (y/8)*SSD1306_LCDWIDTH] &= ~(1 << (y&7)); break;
    case INVERSE: screen[x+ (y/8)*SSD1306_LCDWIDTH] ^=  (1 << (y&7)); break;
    }
    return 0;
}

/****************************************************************
 * Function Name : writeLine
 * Description   : Bresenham's algorithm
 * Returns       : NONE
 * Params        : @x0: X0 Co-ordinate
 *                 @y0: Y0 Co-ordinate
 *                 @x1: X1 Co-ordinate
 *                 @y1: Y1 Co-ordinate
 *                 @color: Pixel color
 ****************************************************************/
void writeLine(short x0, short y0, short x1, short y1, short color)
{
    short steep = 0, dx = 0, dy = 0, err = 0, ystep = 0;
    steep = abs(y1 - y0) > abs(x1 - x0);
    if (steep)
    {
        SWAP(x0, y0);
        SWAP(x1, y1);
    }

    if (x0 > x1)
    {
        SWAP(x0, x1);
        SWAP(y0, y1);
    }
    dx = x1 - x0;
    dy = abs(y1 - y0);

    err = dx / 2;

    if (y0 < y1)
    {
        ystep = 1;
    } else
    {
        ystep = -1;
    }

    for (; x0<=x1; x0++)
    {
        if (steep)
        {
            drawPixel(y0, x0, color);
        } else
        {
            drawPixel(x0, y0, color);
        }
        err -= dy;
        if (err < 0)
        {
            y0 += ystep;
            err += dx;
        }
    }
}

/* (x,y) is topmost point; if unsure, calling function
should sort endpoints or call writeLine() instead */
void drawFastVLine(short x, short y,short h, short color)
{
    //startWrite();
    writeLine(x, y, x, y+h-1, color);
    //endWrite();
}

/* (x,y) is topmost point; if unsure, calling function
should sort endpoints or call writeLine() instead */
void writeFastVLine(short x, short y, short h, short color)
{
    drawFastVLine(x, y, h, color);
}

/* (x,y) is leftmost point; if unsure, calling function
 should sort endpoints or call writeLine() instead */
void drawFastHLine(short x, short y,short w, short color)
{
    //startWrite();
    writeLine(x, y, x+w-1, y, color);
    //endWrite();
}

// (x,y) is leftmost point; if unsure, calling function
// should sort endpoints or call writeLine() instead
void writeFastHLine(short x, short y, short w, short color)
{
    drawFastHLine(x, y, w, color);
}

/****************************************************************
 * Function Name : drawCircleHelper
 * Description   : Draw a....
 * Returns       : NONE
 * Params        : @x: X Co-ordinate
 *                 @y: Y Co-ordinate
 *                 @w: Width
 *                 @h: height
 *                 @r: Corner radius
 *                 @color: Pixel color
 ****************************************************************/
void drawCircleHelper( short x0, short y0, short r, unsigned char cornername, short color) 
{
    short f     = 1 - r;
    short ddF_x = 1;
    short ddF_y = -2 * r;
    short x     = 0;
    short y     = r;

    while (x<y)
    {
        if (f >= 0)
        {
            y--;
            ddF_y += 2;
            f     += ddF_y;
        }
        x++;
        ddF_x += 2;
        f     += ddF_x;
        if (cornername & 0x4)
        {
            drawPixel(x0 + x, y0 + y, color);
            drawPixel(x0 + y, y0 + x, color);
        }
        if (cornername & 0x2)
        {
            drawPixel(x0 + x, y0 - y, color);
            drawPixel(x0 + y, y0 - x, color);
        }
        if (cornername & 0x8)
        {
            drawPixel(x0 - y, y0 + x, color);
            drawPixel(x0 - x, y0 + y, color);
        }
        if (cornername & 0x1)
        {
            drawPixel(x0 - y, y0 - x, color);
            drawPixel(x0 - x, y0 - y, color);
        }
    }
}

/****************************************************************
 * Function Name : drawLine
 * Description   : Draw line between two points
 * Returns       : NONE
 * Params        : @x0: X0 Starting X Co-ordinate
 *                 @y0: Y0 Starting Y Co-ordinate
 *                 @x1: X1 Ending X Co-ordinate
 *                 @y1: Y1 Ending Y Co-ordinate
 *                 @color: Pixel color
 ****************************************************************/
void drawLine(short x0, short y0, short x1, short y1, short color)
{
    if(x0 == x1)
    {
        if(y0 > y1)
            SWAP(y0, y1);
        drawFastVLine(x0, y0, y1 - y0 + 1, color);
    }
    else if(y0 == y1)
    {
        if(x0 > x1) SWAP(x0, x1);
        drawFastHLine(x0, y0, x1 - x0 + 1, color);
    }
    else
    {
        //startWrite();
        writeLine(x0, y0, x1, y1, color);
        //endWrite();
    }
}

/****************************************************************
 * Function Name : drawRect
 * Description   : Draw a rectangle
 * Returns       : NONE
 * Params        : @x: Corner X Co-ordinate
 *                 @y: Corner Y Co-ordinate
 *                 @w: Width in pixels
 *                 @h: Height in pixels
 *                 @color: Pixel color
 ****************************************************************/
void drawRect(short x, short y, short w, short h, short color)
{
    //startWrite();
    writeFastHLine(x, y, w, color);
    writeFastHLine(x, y+h-1, w, color);
    writeFastVLine(x, y, h, color);
    writeFastVLine(x+w-1, y, h, color);
    //endWrite();
}

/****************************************************************
 * Function Name : fillRect
 * Description   : Fill the rectangle
 * Returns       : NONE
 * Params        : @x: Starting X Co-ordinate
 *                 @y: Starting Y Co-ordinate
 *                 @w: Width in pixels
 *                 @h: Height in pixels
 *                 @color: Pixel color
 ****************************************************************/
void fillRect(short x, short y, short w, short h, short color)
{
    short i = 0;
    //startWrite();
    for (i=x; i<x+w; i++)
    {
        writeFastVLine(i, y, h, color);
    }
    //endWrite();
}

/****************************************************************
 * Function Name : drawCircle
 * Description   : Draw a circle
 * Returns       : NONE
 * Params        : @x: Center X Co-ordinate
 *                 @y: Center Y Co-ordinate
 *                 @r: Radius in pixels
 *                 @color: Pixel color
 ****************************************************************/
void drawCircle(short x0, short y0, short r, short color)
{
    short f = 1 - r;
    short ddF_x = 1;
    short ddF_y = -2 * r;
    short x = 0;
    short y = r;

    //startWrite();
    drawPixel(x0  , y0+r, color);
    drawPixel(x0  , y0-r, color);
    drawPixel(x0+r, y0  , color);
    drawPixel(x0-r, y0  , color);

    while (x<y)
    {
        if (f >= 0)
        {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        drawPixel(x0 + x, y0 + y, color);
        drawPixel(x0 - x, y0 + y, color);
        drawPixel(x0 + x, y0 - y, color);
        drawPixel(x0 - x, y0 - y, color);
        drawPixel(x0 + y, y0 + x, color);
        drawPixel(x0 - y, y0 + x, color);
        drawPixel(x0 + y, y0 - x, color);
        drawPixel(x0 - y, y0 - x, color);
    }
    //endWrite();
}

/****************************************************************
 * Function Name : fillCircleHelper
 * Description   : Used to do circles and roundrects
 * Returns       : NONE
 * Params        : @x: Center X Co-ordinate
 *                 @y: Center Y Co-ordinate
 *                 @r: Radius in pixels
 *                 @cornername: Corner radius in pixels
 *                 @color: Pixel color
 ****************************************************************/
void fillCircleHelper(short x0, short y0, short r, unsigned char cornername, short delta, short color)
{

    short f     = 1 - r;
    short ddF_x = 1;
    short ddF_y = -2 * r;
    short x     = 0;
    short y     = r;

    while (x<y)
    {
        if (f >= 0)
        {
            y--;
            ddF_y += 2;
            f     += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        if (cornername & 0x1)
        {
            writeFastVLine(x0+x, y0-y, 2*y+1+delta, color);
            writeFastVLine(x0+y, y0-x, 2*x+1+delta, color);
        }
        if (cornername & 0x2)
        {
            writeFastVLine(x0-x, y0-y, 2*y+1+delta, color);
            writeFastVLine(x0-y, y0-x, 2*x+1+delta, color);
        }
    }
}

/****************************************************************
 * Function Name : fillCircle
 * Description   : Fill the circle
 * Returns       : NONE
 * Params        : @x0: Center X Co-ordinate
 *                 @y0: Center Y Co-ordinate
 *                 @r: Radius in pixels
 *                 @color: Pixel color
 ****************************************************************/
void fillCircle(short x0, short y0, short r, short color)
{
    //startWrite();
    writeFastVLine(x0, y0-r, 2*r+1, color);
    fillCircleHelper(x0, y0, r, 3, 0, color);
    //endWrite();
}

/****************************************************************
 * Function Name : drawTriangle
 * Description   : Draw a triangle
 * Returns       : NONE
 * Params        : @x0: Corner-1 X Co-ordinate
 *                 @y0: Corner-1 Y Co-ordinate
 *                 @x1: Corner-2 X Co-ordinate
 *                 @y1: Corner-2 Y Co-ordinate
 *                 @x2: Corner-3 X Co-ordinate
 *                 @y2: Corner-3 Y Co-ordinate
 *                 @color: Pixel color
 ****************************************************************/
void drawTriangle(short x0, short y0, short x1, short y1, short x2, short y2, short color)
{
    drawLine(x0, y0, x1, y1, color);
    drawLine(x1, y1, x2, y2, color);
    drawLine(x2, y2, x0, y0, color);
}

/****************************************************************
 * Function Name : fillTriangle
 * Description   : Fill a triangle
 * Returns       : NONE
 * Params        : @x0: Corner-1 X Co-ordinate
 *                 @y0: Corner-1 Y Co-ordinate
 *                 @x1: Corner-2 X Co-ordinate
 *                 @y1: Corner-2 Y Co-ordinate
 *                 @x2: Corner-3 X Co-ordinate
 *                 @y2: Corner-3 Y Co-ordinate
 *                 @color: Pixel color
 ****************************************************************/
void fillTriangle(short x0, short y0, short x1, short y1, short x2, short y2, short color)
{
    short a, b, y, last, dx01, dy01, dx02, dy02, dx12, dy12;
    int sa, sb;

    // Sort coordinates by Y order (y2 >= y1 >= y0)
    if (y0 > y1)
    {
        SWAP(y0, y1);
        SWAP(x0, x1);
    }
    if (y1 > y2)
    {
        SWAP(y2, y1);
        SWAP(x2, x1);
    }
    if (y0 > y1)
    {
        SWAP(y0, y1);
        SWAP(x0, x1);
    }

    //startWrite();
    if(y0 == y2)
    { // Handle awkward all-on-same-line case as its own thing
        a = b = x0;
        if(x1 < a)
            a = x1;
        else if(x1 > b)
            b = x1;
        if(x2 < a)
            a = x2;
        else if(x2 > b)
            b = x2;
        writeFastHLine(a, y0, b-a+1, color);
        // endWrite();
        return;
    }

    dx01 = x1 - x0;
    dy01 = y1 - y0;
    dx02 = x2 - x0;
    dy02 = y2 - y0;
    dx12 = x2 - x1;
    dy12 = y2 - y1;
    sa   = 0;
    sb   = 0;

    // For upper part of triangle, find scanline crossings for segments
    // 0-1 and 0-2.  If y1=y2 (flat-bottomed triangle), the scanline y1
    // is included here (and second loop will be skipped, avoiding a /0
    // error there), otherwise scanline y1 is skipped here and handled
    // in the second loop...which also avoids a /0 error here if y0=y1
    // (flat-topped triangle).
    if(y1 == y2)
        last = y1;   // Include y1 scanline
    else
        last = y1-1; // Skip it

    for(y=y0; y<=last; y++)
    {
        a   = x0 + sa / dy01;
        b   = x0 + sb / dy02;
        sa += dx01;
        sb += dx02;
        /* longhand:
        a = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
        b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
         */
        if(a > b)
            SWAP(a,b);
        writeFastHLine(a, y, b-a+1, color);
    }

    // For lower part of triangle, find scanline crossings for segments
    // 0-2 and 1-2.  This loop is skipped if y1=y2.
    sa = dx12 * (y - y1);
    sb = dx02 * (y - y0);
    for(; y<=y2; y++)
    {
        a   = x1 + sa / dy12;
        b   = x0 + sb / dy02;
        sa += dx12;
        sb += dx02;
        /* longhand:
        a = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
        b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
         */
        if(a > b)
            SWAP(a,b);
        writeFastHLine(a, y, b-a+1, color);
    }
    //endWrite();
}

/****************************************************************
 * Function Name : drawRoundRect
 * Description   : Draw a rounded rectangle
 * Returns       : NONE
 * Params        : @x: X Co-ordinate
 *                 @y: Y Co-ordinate
 *                 @w: Width
 *                 @h: height
 *                 @r: Corner radius
 *                 @color: Pixel color
 ****************************************************************/
void drawRoundRect(short x, short y, short w, short h, short r, short color)
{
    // smarter version
    //startWrite();
    writeFastHLine(x+r  , y    , w-2*r, color); // Top
    writeFastHLine(x+r  , y+h-1, w-2*r, color); // Bottom
    writeFastVLine(x    , y+r  , h-2*r, color); // Left
    writeFastVLine(x+w-1, y+r  , h-2*r, color); // Right
    // draw four corners
    drawCircleHelper(x+r    , y+r    , r, 1, color);
    drawCircleHelper(x+w-r-1, y+r    , r, 2, color);
    drawCircleHelper(x+w-r-1, y+h-r-1, r, 4, color);
    drawCircleHelper(x+r    , y+h-r-1, r, 8, color);
    //endWrite();
}

/****************************************************************
 * Function Name : fillRoundRect
 * Description   :  Fill a rounded rectangle
 * Returns       : NONE
 * Params        : @x: X Co-ordinate
 *                 @y: Y Co-ordinate
 *                 @w: Width
 *                 @h: height
 *                 @r: Corner radius
 *                 @color: Pixel color
 ****************************************************************/
void fillRoundRect(short x, short y, short w, short h, short r, short color)
{
    // smarter version
    //startWrite();
    fillRect(x+r, y, w-2*r, h, color);

    // draw four corners
    fillCircleHelper(x+w-r-1, y+r, r, 1, h-2*r-1, color);
    fillCircleHelper(x+r    , y+r, r, 2, h-2*r-1, color);
    //endWrite();
}

/*----------------------------------------------------------------------------
 * BITMAP API's
 ----------------------------------------------------------------------------*/

/****************************************************************
 * Function Name : drawBitmap
 * Description   : Draw a bitmap
 * Returns       : NONE
 * Params        : @x: X Co-ordinate
 *                 @y: Y Co-ordinate
 *                 @bitmap: bitmap to display
 *                 @w: Width
 *                 @h: height
 *                 @color: Pixel color
 ****************************************************************/
void drawBitmap(short x, short y, const unsigned char bitmap[], short w, short h, short color)
{
    short byteWidth = 0, j = 0, i = 0;
    unsigned char byte = 0;
    byteWidth = (w + 7) / 8; // Bitmap scanline pad = whole byte

    for(j=0; j<h; j++, y++)
    {
        for(i=0; i<w; i++)
        {
            if(i & 7)
                byte <<= 1;
            else
                byte   = pgm_read_byte(&bitmap[j * byteWidth + i / 8]);
            if(byte & 0x80)
                drawPixel(x+i, y, color);
        }
    }
}

/*----------------------------------------------------------------------------
 * TEXT AND CHARACTER HANDLING API's
 ----------------------------------------------------------------------------*/

/****************************************************************
 * Function Name : setCursor
 * Description   : Sets the cursor on f(x,y)
 * Returns       : NONE.
 * Params        : @x - X-Cordinate
 *                 @y - Y-Cordinate
 ****************************************************************/
void setCursor(short x, short y) 
{
    cursor_x = x;
    cursor_y = y;
}

/****************************************************************
 * Function Name : getCursorX
 * Description   : Get cursor at X- Cordinate
 * Returns       : x cordinate value.
 ****************************************************************/
short getCursorX() 
{
    return cursor_x;
}

/****************************************************************
 * Function Name : getCursorY
 * Description   : Get cursor at Y- Cordinate
 * Returns       : y cordinate value.
 ****************************************************************/
short getCursorY() 
{
    return cursor_y;
}

/****************************************************************
 * Function Name : setTextSize
 * Description   : Set text size
 * Returns       : @s - font size
 ****************************************************************/
void setTextSize(unsigned char s) 
{
    textsize = (s > 0) ? s : 1;
}

/****************************************************************
 * Function Name : setTextColor
 * Description   : Set text color
 * Returns       : @c - Color
 ****************************************************************/
void setTextColor(short c) 
{
    // For 'transparent' background, we'll set the bg
    // to the same as fg instead of using a flag
    textcolor = textbgcolor = c;
}

/****************************************************************
 * Function Name : setTextWrap
 * Description   : Wraps the text
 * Returns       : @w - enable or disbale wrap
 ****************************************************************/
void setTextWrap(bool w) 
{
    wrap = w;
}

/****************************************************************
 * Function Name : getRotation
 * Description   : Get the rotation value
 * Returns       : NONE.
 ****************************************************************/
unsigned char getRotation()
{
    return _rotation;
}

/****************************************************************
 * Function Name : drawBitmap
 * Description   : Draw a character
 * Returns       : NONE
 * Params        : @x: X Co-ordinate
 *                 @y: Y Co-ordinate
 *                 @c: Character
 *                 @size: Scaling factor
 *                 @bg: Background color
 *                 @color: Pixel color
 ****************************************************************/
void drawChar(short x, short y, unsigned char c, short color, short bg, unsigned char size)
{
    unsigned char line = 0, *bitmap = NULL, w = 0, h = 0, xx = 0, yy = 0, bits = 0, bit = 0;
    char i = 0, j = 0, xo = 0, yo = 0;
    short bo = 0, xo16 = 0, yo16 = 0;
    GFXglyphPtr glyph;
    if(!gfxFont)
    {
        // 'Classic' built-in font
        if((x >= _width) || (y >= _height) || ((x + 6 * size - 1) < 0) || ((y + 8 * size - 1) < 0))
            return;

        // Handle 'classic' charset behavior
        if(!_cp437 && (c >= 176))
            c++;

        // Char bitmap = 5 columns
        for(i=0; i<5; i++ )
        {
            line = pgm_read_byte(&ssd1306_font5x7[c * 5 + i]);
            for(j=0; j<8; j++, line >>= 1)
            {
                if(line & 1)
                {
                    if(size == 1)
                        drawPixel(x+i, y+j, color);
                    else
                        fillRect(x+i*size, y+j*size, size, size, color);
                }
                else if(bg != color)
                {
                    if(size == 1)
                        drawPixel(x+i, y+j, bg);
                    else
                        fillRect(x+i*size, y+j*size, size, size, bg);
                }
            }
        }

        // If opaque, draw vertical line for last column
        if(bg != color)
        {
            if(size == 1)
                writeFastVLine(x+5, y, 8, bg);
            else
                fillRect(x+5*size, y, size, 8*size, bg);
        }

    }
    // Custom font
    else
    {
        // Character is assumed previously filtered by write() to eliminate
        // newlines, returns, non-printable characters, etc.  Calling
        // drawChar() directly with 'bad' characters of font may cause mayhem!

        c -= (unsigned char)pgm_read_byte(&gfxFont->first);
        glyph  = &(((GFXglyphT *)pgm_read_pointer(&gfxFont->glyph))[c]);
        bitmap = (unsigned char *)pgm_read_pointer(&gfxFont->bitmap);
        bo = pgm_read_word(&glyph->bitmapOffset);
        w  = pgm_read_byte(&glyph->width);
        h  = pgm_read_byte(&glyph->height);
        xo = pgm_read_byte(&glyph->xOffset);
        yo = pgm_read_byte(&glyph->yOffset);

        if(size > 1)
        {
            xo16 = xo;
            yo16 = yo;
        }

        // Todo: Add character clipping here

        // NOTE: THERE IS NO 'BACKGROUND' COLOR OPTION ON CUSTOM FONTS.
        // THIS IS ON PURPOSE AND BY DESIGN.  The background color feature
        // has typically been used with the 'classic' font to overwrite old
        // screen contents with new data.  This ONLY works because the
        // characters are a uniform size; it's not a sensible thing to do with
        // proportionally-spaced fonts with glyphs of varying sizes (and that
        // may overlap).  To replace previously-drawn text when using a custom
        // font, use the getTextBounds() function to determine the smallest
        // rectangle encompassing a string, erase the area with fillRect(),
        // then draw new text.  This WILL unfortunately 'blink' the text, but
        // is unavoidable.  Drawing 'background' pixels will NOT fix this,
        // only creates a new set of problems.  Have an idea to work around
        // this (a canvas object type for MCUs that can afford the RAM and
        // displays supporting setAddrWindow() and pushColors()), but haven't
        // implemented this yet.
        for(yy=0; yy<h; yy++)
        {
            for(xx=0; xx<w; xx++)
            {
                if(!(bit++ & 7))
                {
                    bits = pgm_read_byte(&bitmap[bo++]);
                }
                if(bits & 0x80)
                {
                    if(size == 1)
                    {
                        drawPixel(x+xo+xx, y+yo+yy, color);
                    }
                    else
                    {
                        fillRect(x+(xo16+xx)*size, y+(yo16+yy)*size,size, size, color);
                    }
                }
                bits <<= 1;
            }
        }
    } // End classic vs custom font
}

/****************************************************************
 * Function Name : write
 * Description   : Base function for text and character handling
 * Returns       : 1
 * Params        : @c: Character
 ****************************************************************/
short oled_write(unsigned char c)
{
    unsigned char first = 0, w = 0, h = 0;
    short xo = 0;
    GFXglyphPtr glyph;
    if(!gfxFont)
    {
        // 'Classic' built-in font
        if(c == '\n')
        {
            // Newline?
            cursor_x  = 0;                     // Reset x to zero,
            cursor_y += textsize * 8;          // advance y one line
        }
        else if(c != '\r')
        {
            // Ignore carriage returns
            if(wrap && ((cursor_x + textsize * 6) > _width))
            {
                // Off right?
                cursor_x  = 0;                 // Reset x to zero,
                cursor_y += textsize * 8;      // advance y one line
            }
            drawChar(cursor_x, cursor_y, c, textcolor, textbgcolor, textsize);
            cursor_x += textsize * 6;          // Advance x one char
        }

    }
    else
    {
        // Custom font
        if(c == '\n')
        {
            cursor_x  = 0;
            cursor_y += (short)textsize *(unsigned char)pgm_read_byte(&gfxFont->yAdvance);
        }
        else if(c != '\r')
        {
            first = pgm_read_byte(&gfxFont->first);
            if((c >= first) && (c <= (unsigned char)pgm_read_byte(&gfxFont->last)))
            {
                glyph = &(((GFXglyphT*)pgm_read_pointer(&gfxFont->glyph))[c - first]);
                w     = pgm_read_byte(&glyph->width);
                h     = pgm_read_byte(&glyph->height);
                if((w > 0) && (h > 0))
                {
                    // Is there an associated bitmap?
                    xo = (char)pgm_read_byte(&glyph->xOffset); // sic
                    if(wrap && ((cursor_x + textsize * (xo + w)) > _width))
                    {
                        cursor_x  = 0;
                        cursor_y += (short)textsize *(unsigned char)pgm_read_byte(&gfxFont->yAdvance);
                    }
                    drawChar(cursor_x, cursor_y, c, textcolor, textbgcolor, textsize);
                }
                cursor_x += (unsigned char)pgm_read_byte(&glyph->xAdvance) * (short)textsize;
            }
        }
    }
    return 1;
}

/****************************************************************
 * Function Name : print
 * Description   : Base function for printing strings
 * Returns       : No. of characters printed
 * Params        : @buffer: Ptr to buffer containing the string
 *                 @size: Length of the string.
 ****************************************************************/
short print(const unsigned char *buffer, short size)
{
    short n = 0;
    while(size--)
    {
        if(oled_write(*buffer++))
            n++;
        else
            break;
    }
    return (n);
}

/****************************************************************
 * Function Name : print_str
 * Description   : Print strings
 * Returns       : No. of characters printed
 * Params        : @strPtr: Ptr to buffer containing the string
 ****************************************************************/
short print_str(const unsigned char *strPtr)
{
    return print(strPtr, strlen(strPtr));
}

/****************************************************************
 * Function Name : println
 * Description   : Move to next line
 * Returns       : No. of characters printed
 * Params        : NONE.
 ****************************************************************/
short println()
{
    return print_str("\r\n");
}

/****************************************************************
 * Function Name : print_strln
 * Description   : Print strings and move to next line
 * Returns       : No. of characters printed
 * Params        : @strPtr: Ptr to buffer containing the string
 ****************************************************************/
short print_strln(const unsigned char *strPtr)
{
    short n = 0;
    n = print(strPtr, strlen(strPtr));
    n += print_str("\r\n");
    return (n);
}

/*----------------------------------------------------------------------------
 * NUMBERS HANDLING API's
 ----------------------------------------------------------------------------*/

/****************************************************************
 * Function Name : printNumber
 * Description   : Base function to print unsigned numbers
 * Returns       : No. of characters printed
 * Params        : @n: Number
 *                 @base: Base e.g. HEX, BIN...
 ****************************************************************/
short printNumber(unsigned long n, unsigned char base)
{
    unsigned long m = 0;
    char c = 0;
    char buf[8 * sizeof(long) + 1]; // Assumes 8-bit chars plus zero byte.
    char *str = &buf[sizeof(buf) - 1];

    *str = '\0';

    // prevent crash if called with base == 1
    if(base < 2)
        base = 10;
    do
    {
        m = n;
        n /= base;
        c = m - base * n;
        *--str = c < 10 ? c + '0' : c + 'A' - 10;
    }
    while(n);
    //return oled_write((unsigned char)str);
    return print_str(str);
}

/****************************************************************
 * Function Name : printNumber_UL
 * Description   : Print unsigned long data types
 * Returns       : No. of characters printed
 * Params        : @n: Number
 *                 @base: Base e.g. HEX, BIN...
 ****************************************************************/
short printNumber_UL(unsigned long n, int base)
{
    if(base == 0)
        return oled_write(n);
    else
        return printNumber(n, base);
}

/****************************************************************
 * Function Name : printNumber_UL_ln
 * Description   : Print unsigned long & advance to next line
 * Returns       : No. of characters printed
 * Params        : @n: Number
 *                 @base: Base e.g. HEX, BIN...
 ****************************************************************/
short printNumber_UL_ln(unsigned long num, int base)
{
    short n = 0;
    n = printNumber(num, base);
    n += println();
    return (n);
}

/****************************************************************
 * Function Name : printNumber_UI
 * Description   : Print unsigned int data types
 * Returns       : No. of characters printed
 * Params        : @n: Number
 *                 @base: Base e.g. HEX, BIN...
 ****************************************************************/
short printNumber_UI(unsigned int n, int base)
{
    return printNumber((unsigned long) n, base);
}

/****************************************************************
 * Function Name : printNumber_UI_ln
 * Description   : Print unsigned int & advance to next line
 * Returns       : No. of characters printed
 * Params        : @n: Number
 *                 @base: Base e.g. HEX, BIN...
 ****************************************************************/
short printNumber_UI_ln(unsigned int n, int base)
{
    short a = 0;
    a = printNumber((unsigned long) n, base);
    a += println();
    return (a);
}

/****************************************************************
 * Function Name : printNumber_UC
 * Description   : Print unsigned char data types
 * Returns       : No. of characters printed
 * Params        : @n: Number
 *                 @base: Base e.g. HEX, BIN...
 ****************************************************************/
short printNumber_UC(unsigned char b, int base)
{
    return printNumber((unsigned long) b, base);
}

/****************************************************************
 * Function Name : printNumber_UC_ln
 * Description   : Print unsigned char & advance to next line
 * Returns       : No. of characters printed
 * Params        : @n: Number
 *                 @base: Base e.g. HEX, BIN...
 ****************************************************************/
short printNumber_UC_ln(unsigned char b, int base)
{
    short n = 0;
    n = printNumber((unsigned long) b, base);
    n += println();
    return (n);
}

/****************************************************************
 * Function Name : printNumber_L
 * Description   : Print Long data types
 * Returns       : No. of characters printed
 * Params        : @n: Number
 *                 @base: Base e.g. HEX, BIN...
 ****************************************************************/
short printNumber_L(long n, int base)
{
    int t = 0;
    if(base == 0)
    {
        return oled_write(n);
    }
    else if(base == 10)
    {
        if(n < 0)
        {
            t = oled_write('-');
            n = -n;
            return printNumber(n, 10) + t;
        }
        return printNumber(n, 10);
    }
    else
    {
        return printNumber(n, base);
    }
}

/****************************************************************
 * Function Name : printNumber_UC_ln
 * Description   : Print long & advance to next line
 * Returns       : No. of characters printed
 * Params        : @n: Number
 *                 @base: Base e.g. HEX, BIN...
 ****************************************************************/
short printNumber_L_ln(long num, int base)
{
    short n = 0;
    n = printNumber_L(num, base);
    n += println();
    return n;
}

/****************************************************************
 * Function Name : printNumber_I
 * Description   : Print int data types
 * Returns       : No. of characters printed
 * Params        : @n: Number
 *                 @base: Base e.g. HEX, BIN...
 ****************************************************************/
short printNumber_I(int n, int base)
{
    return printNumber_L((long) n, base);
}

/****************************************************************
 * Function Name : printNumber_I_ln
 * Description   : Print int & advance to next line
 * Returns       : No. of characters printed
 * Params        : @n: Number
 *                 @base: Base e.g. HEX, BIN...
 ****************************************************************/
short printNumber_I_ln(int n, int base)
{
    short a = 0;
    a = printNumber_L((long) n, base);
    a += println();
    return a;
}

/****************************************************************
 * Function Name : printFloat
 * Description   : Print floating Pt. No's.
 * Returns       : No. of characters printed
 * Params        : @n: Number
 *                 @digits: Resolution
 ****************************************************************/
short printFloat(double number, unsigned char digits)
{
    unsigned char i = 0;
    short n = 0;
    unsigned long int_part = 0;
    double remainder = 0.0;
    int toPrint = 0;

    // Round correctly so that print(1.999, 2) prints as "2.00"
    double rounding = 0.5;

    if(isnan(number))
        return print_str("nan");
    if(isinf(number))
        return print_str("inf");
    if(number > 4294967040.0)
        return print_str("ovf");  // constant determined empirically
    if(number < -4294967040.0)
        return print_str("ovf");  // constant determined empirically

    // Handle negative numbers
    if(number < 0.0)
    {
        n += oled_write('-');
        number = -number;
    }


    for(i = 0; i < digits; ++i)
        rounding /= 10.0;

    number += rounding;

    // Extract the integer part of the number and print it
    int_part = (unsigned long) number;
    remainder = number - (double) int_part;
    n += printNumber_UL(int_part,DEC);

    // Print the decimal point, but only if there are digits beyond
    if(digits > 0)
    {
        n += print_str(".");
    }

    // Extract digits from the remainder one at a time
    while(digits-- > 0)
    {
        remainder *= 10.0;
        toPrint = (int)remainder;
        n += printNumber_I(toPrint,DEC);
        remainder -= toPrint;
    }
    return n;
}

/****************************************************************
 * Function Name : printFloat_ln
 * Description   : Print floating Pt. No and advance to next line
 * Returns       : No. of characters printed
 * Params        : @n: Number
 *                 @digits: Resolution
 ****************************************************************/
short printFloat_ln(double num, int digits)
{
    short n = 0;
    n = printFloat(num, digits);
    n += println();
    return n;
}
