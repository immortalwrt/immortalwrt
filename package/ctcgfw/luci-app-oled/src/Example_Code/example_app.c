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

 * example_app.c
 *
 *  Created on  : Sep 6, 2017
 *  Author      : Vinay Divakar
 *  Website     : www.deeplyembedded.org
 */

/* Lib Includes */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include "SSD1306_OLED.h"
#include "example_app.h"

#define BUFMAX SSD1306_LCDWIDTH*SSD1306_LCDHEIGHT

/* MACRO's */
#define LOGO16_GLCD_HEIGHT 16
#define LOGO16_GLCD_WIDTH  16
#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2



#define TIMESIZE 64
//temperature
#define TEMPPATH "/sys/class/thermal/thermal_zone0/temp"
#define TEMPSIZE 5
//cpu
#define FREQSIZE 8 
#define FREQPATH "cat /sys/devices/system/cpu/cpu[04]/cpufreq/cpuinfo_cur_freq"
//ip
#define IPPATH "ifconfig br-lan|grep 'inet addr:'|cut -d: -f2|awk '{print $1}'"
#define IPSIZE 20
//netspeed
#define NETPATH "cat /tmp/netspeed"

/* Extern volatile */
extern volatile unsigned char flag;

/* Bit Map - Taken from Adafruit SSD1306 OLED Library  */
static const unsigned char logo16_glcd_bmp[] =
{
 0b00000000, 0b11000000,
 0b00000001, 0b11000000,
 0b00000001, 0b11000000,
 0b00000011, 0b11100000,
 0b11110011, 0b11100000,
 0b11111110, 0b11111000,
 0b01111110, 0b11111111,
 0b00110011, 0b10011111,
 0b00011111, 0b11111100,
 0b00001101, 0b01110000,
 0b00011011, 0b10100000,
 0b00111111, 0b11100000,
 0b00111111, 0b11110000,
 0b01111100, 0b11110000,
 0b01110000, 0b01110000,
 0b00000000, 0b00110000
};

FILE *fp;
char content_buff[BUFMAX];
char buf[BUFMAX];
int display_offset =7;
/* draw many lines */
void testdrawline() 
{
    short i = 0;
    for (i=0; i<SSD1306_LCDWIDTH; i+=4)
    {
        drawLine(0, 0, i, SSD1306_LCDHEIGHT-1, WHITE);
        Display();
        usleep(1000);
    }
    for (i=0; i<SSD1306_LCDHEIGHT; i+=4)
    {
        drawLine(0, 0, SSD1306_LCDWIDTH-1, i, WHITE);
        Display();
        usleep(1000);
    }
    usleep(250000);

    clearDisplay();
    for (i=0; i<SSD1306_LCDWIDTH; i+=4)
    {
        drawLine(0, SSD1306_LCDHEIGHT-1, i, 0, WHITE);
        Display();
        usleep(1000);
    }
    for (i=SSD1306_LCDHEIGHT-1; i>=0; i-=4)
    {
        drawLine(0, SSD1306_LCDHEIGHT-1, SSD1306_LCDWIDTH-1, i, WHITE);
        Display();
        usleep(1000);
    }
    usleep(250000);

    clearDisplay();
    for (i=SSD1306_LCDWIDTH-1; i>=0; i-=4)
    {
        drawLine(SSD1306_LCDWIDTH-1, SSD1306_LCDHEIGHT-1, i, 0, WHITE);
        Display();
        usleep(1000);
    }
    for (i=SSD1306_LCDHEIGHT-1; i>=0; i-=4)
    {
        drawLine(SSD1306_LCDWIDTH-1, SSD1306_LCDHEIGHT-1, 0, i, WHITE);
        Display();
        usleep(1000);
    }
    usleep(250000);

    clearDisplay();
    for (i=0; i<SSD1306_LCDHEIGHT; i+=4)
    {
        drawLine(SSD1306_LCDWIDTH-1, 0, 0, i, WHITE);
        Display();
        usleep(1000);
    }
    for (i=0; i<SSD1306_LCDWIDTH; i+=4) {
        drawLine(SSD1306_LCDWIDTH-1, 0, i, SSD1306_LCDHEIGHT-1, WHITE);
        Display();
        usleep(1000);
    }
    usleep(250000);
}

/* draw rectangles */
void testdrawrect() 
{
    short i = 0;
    for (i=0; i<SSD1306_LCDHEIGHT/2; i+=2)
    {
        drawRect(i, i,SSD1306_LCDWIDTH-2*i, SSD1306_LCDHEIGHT-2*i, WHITE);
        Display();
        usleep(1000);
    }
}

/* draw multiple rectangles */
void testfillrect() 
{
    unsigned char color = 1;
    short i = 0;
    for (i=0; i<SSD1306_LCDHEIGHT/2; i+=3)
    {
        // alternate colors
        fillRect(i, i, SSD1306_LCDWIDTH-i*2, SSD1306_LCDHEIGHT-i*2, color%2);
        Display();
        usleep(1000);
        color++;
    }
}

/* draw mulitple circles */
void testdrawcircle()
{
    short i = 0;
    for (i=0; i<SSD1306_LCDHEIGHT; i+=2)
    {
        drawCircle(SSD1306_LCDWIDTH/2,SSD1306_LCDHEIGHT/2, i, WHITE);
        Display();
        usleep(1000);
    }
}

/*draw a white circle, 10 pixel radius */
void testdrawroundrect() 
{
    short i = 0;
    for (i=0; i<SSD1306_LCDHEIGHT/2-2; i+=2) {
        drawRoundRect(i, i,SSD1306_LCDWIDTH-2*i, SSD1306_LCDHEIGHT-2*i, SSD1306_LCDHEIGHT/4, WHITE);
        Display();
        usleep(1000);
    }
}

/* Fill the round rectangle */
void testfillroundrect() 
{
    short color = WHITE,i = 0;
    for (i=0; i<SSD1306_LCDHEIGHT/2-2; i+=2)
    {
        fillRoundRect(i, i, SSD1306_LCDWIDTH-2*i, SSD1306_LCDHEIGHT-2*i, SSD1306_LCDHEIGHT/4, color);
        if (color == WHITE)
            color = BLACK;
        else
            color = WHITE;
        Display();
        usleep(1000);
    }
}

/* Draw triangles */
void testdrawtriangle() 
{
    short i = 0;
    for (i=0; i<MIN(SSD1306_LCDWIDTH,SSD1306_LCDHEIGHT)/2; i+=5)
    {
        drawTriangle(SSD1306_LCDWIDTH/2, SSD1306_LCDHEIGHT/2-i,
                     SSD1306_LCDWIDTH/2-i,SSD1306_LCDHEIGHT /2+i,
                     SSD1306_LCDWIDTH/2+i, SSD1306_LCDHEIGHT/2+i, WHITE);
        Display();
        usleep(1000);
    }
}

/* Fill triangles */
void testfilltriangle() 
{
    unsigned char color = WHITE;
    short i = 0;
    for (i=MIN(SSD1306_LCDWIDTH,SSD1306_LCDHEIGHT)/2; i>0; i-=5)
    {
        fillTriangle(SSD1306_LCDWIDTH/2, SSD1306_LCDHEIGHT/2-i,
                     SSD1306_LCDWIDTH/2-i, SSD1306_LCDHEIGHT/2+i,
                     SSD1306_LCDWIDTH/2+i, SSD1306_LCDHEIGHT/2+i, WHITE);
        if (color == WHITE)
            color = BLACK;
        else
            color = WHITE;
        Display();
        usleep(1000);
    }
}

/* Display a bunch of characters and emoticons */
void testdrawchar()
{
    unsigned char i = 0;
    setTextSize(1);
    setTextColor(WHITE);
    setCursor(0,0);

    for (i=0; i < 168; i++)
    {
        if (i == '\n')
            continue;
        oled_write(i);
        if ((i > 0) && (i % 21 == 0))
            println();
    }
    Display();
    usleep(1000);
}

/* Display "scroll" and scroll around */
void testscrolltext(char* str)
{
    setTextSize(2);
    setTextColor(WHITE);
    setCursor(10,8);
    sprintf(buf,"%s",str);
    print_strln(buf);
    Display();
    usleep(1000);
    startscrollright(0x00, 0x0F);
    usleep(5000000);
    stopscroll();
    usleep(1000000);
    startscrollleft(0x00, 0x0F);
    usleep(5000000);
    stopscroll();
    usleep(1000000);
    startscrolldiagright(0x00, 0x07);
    usleep(5000000);
    startscrolldiagleft(0x00, 0x07);
    usleep(5000000);
    stopscroll();
}


 
/* Display Texts */
void display_texts()
{
    setTextSize(1);
    setTextColor(WHITE);
    setCursor(10,0);
    print_str("HELLO FELLAS!");
    println();
    printFloat_ln(3.141592, 4); //Print 4 No's after the decimal Pt.
    printNumber_L_ln(-1234, DEC);
    printNumber_UC_ln(170, BIN);
    setTextSize(2);
    setTextColor(WHITE);
    print_str("0x");
    printNumber_UL_ln(0xDEADBEEF, HEX);
}

/* Display miniature bitmap */
void display_bitmap()
{
    drawBitmap(30, 16,  logo16_glcd_bmp, 16, 16, 1);
}

/* Invert Display and Normalize it */
void display_invert_normal()
{
    invertDisplay(SSD1306_INVERT_DISPLAY);
    usleep(1000000);
    invertDisplay(SSD1306_NORMALIZE_DISPLAY);
    usleep(1000000);
}

/* Draw a bitmap and 'animate' movement */
void testdrawbitmap(const unsigned char *bitmap, unsigned char w, unsigned char h)
{
    unsigned char icons[NUMFLAKES][3], f = 0;

    // initialize
    for (f=0; f< NUMFLAKES; f++)
    {
        icons[f][XPOS] = rand() % SSD1306_LCDWIDTH;
        icons[f][YPOS] = 0;
        icons[f][DELTAY] = (rand() % 5) + 1;

        /* Looks kinna ugly to me - Un-Comment if you need it */
        //print_str("x: ");
        //printNumber_UC(icons[f][XPOS], DEC);
        //print_str("y: ");
        //printNumber_UC(icons[f][YPOS], DEC);
        //print_str("dy: ");
        //printNumber_UC(icons[f][DELTAY], DEC);
    }

    while (flag != 5)
    {
        // draw each icon
        for (f=0; f< NUMFLAKES; f++)
        {
            drawBitmap(icons[f][XPOS], icons[f][YPOS], bitmap, w, h, WHITE);
        }
        Display();
        usleep(200000);

        // then erase it + move it
        for (f=0; f< NUMFLAKES; f++)
        {
            drawBitmap(icons[f][XPOS], icons[f][YPOS], bitmap, w, h, BLACK);

            // move it
            icons[f][YPOS] += icons[f][DELTAY];

            // if its gone, reinit
            if (icons[f][YPOS] > SSD1306_LCDHEIGHT)
            {
                icons[f][XPOS] = rand() % SSD1306_LCDWIDTH;
                icons[f][YPOS] = 0;
                icons[f][DELTAY] = (rand() % 5) + 1;
            }
        }
    }
}

/* Draw bitmap and animate */
void testdrawbitmap_eg()
{
    setTextSize(1);
    setTextColor(WHITE);
    setCursor(10,0);
    testdrawbitmap(logo16_glcd_bmp, LOGO16_GLCD_HEIGHT, LOGO16_GLCD_WIDTH);
}

/* Intro */
void deeplyembedded_credits()
{
    setTextSize(1);
    setTextColor(WHITE);
    setCursor(1,0);
    print_strln("deeplyembedded.org");
    println();
    print_strln("Author:Vinay Divakar");
    println();
    println();
    print_strln("THANK YOU");
}


void testdate(int mode, int y)
{
    time_t rawtime;
    time_t curtime;
    uint8_t timebuff[TIMESIZE];
    curtime = time(NULL);
    time(&rawtime);
    switch (mode) 
    {
	case CENTER:
		setTextSize(2);
        	strftime(timebuff,80,"%H:%M",localtime(&rawtime));
        	sprintf(buf,"%s",timebuff);
        	setCursor((127-strlen(buf)*11)/2-4, y);
	break;    	
	case FULL:
		setTextSize(1);
        	strftime(timebuff,80,"%Y-%m-%d %H:%M:%S",localtime(&rawtime));
        	sprintf(buf,"%s",timebuff);
		setCursor(display_offset, y);
    }
    print_strln(buf);
}


void testlanip(int mode, int y)
{
    setTextSize(1);
    if((fp=popen(IPPATH,"r"))!=NULL)
    {
        fscanf(fp,"%s",content_buff);
        fclose(fp);
        //ipbuff[strlen(ipbuff)-1]=32;
        switch(mode) 
	{
	    case CENTER:
		setTextSize(1);
		sprintf(buf,"%s",content_buff);  
		setCursor((127-strlen(buf)*6)/2, y+4);
		break;

	    case FULL:
		setTextSize(1);
		sprintf(buf,"IP:%s",content_buff);
		setCursor(display_offset, y);
	}
        print_strln(buf);
    }

}


void testcputemp(int mode, int y)
{
    if((fp=fopen(TEMPPATH,"r"))!=NULL)
    {
        fgets(content_buff,TEMPSIZE,fp);
        fclose(fp);
        switch (mode) 
	{
	    case CENTER:
		setTextSize(2);
		sprintf(buf, "%.2f",atoi(content_buff)/100.0); 
		setCursor((127-(strlen(buf)+2)*11)/2-4, y);
		print_str(buf);
		oled_write(0);
		oled_write(67);
        	drawCircle(getCursorX()-16, getCursorY()+3, 2, WHITE);
		break;
	    case FULL:
		setTextSize(1);
		sprintf(buf,"CPU TEMP:%.2f",atoi(content_buff)/100.0); 
		setCursor(display_offset, y);
		print_str(buf); 
		oled_write(0);
		oled_write(67);
       		drawCircle(getCursorX()-8, getCursorY()+1, 1, WHITE);
	}
        
    }

}


void testcpufreq(int mode, int y)
{
    if((fp=popen(FREQPATH,"r")) != NULL)
    {
        fgets(content_buff,FREQSIZE,fp);
        fclose(fp);
        switch(mode) 
	{
	    case CENTER:
		setTextSize(2);
		sprintf(buf,"%4dMHz",atoi(content_buff)/1000); 
		setCursor((127-strlen(buf)*11)/2-4, y);
		break;
            case FULL:
		setTextSize(1);
		sprintf(buf,"CPU FREQ:%4dMHz",atoi(content_buff)/1000);
		setCursor(display_offset, y);
	}
        print_strln(buf);
    }

}

void testnetspeed(int mode, int y)
{
    int rx,tx;
    if((fp=popen(NETPATH,"r")) != NULL)
    {
        fscanf(fp,"%d %d", &rx, &tx);
        fclose(fp);
        rx = rx;
        tx = tx;
        switch(mode)
        {
        case SPLIT:
            setTextSize(2);
            if (tx < 1000) sprintf(buf, "%03dB", tx);
            else if (tx > 1000000) sprintf(buf, "%03dM", tx/1000000);
            else sprintf(buf, "%03dK", tx/1000);
            setCursor((127-(strlen(buf)+1)*11)/2,0);
            oled_write(24);
            print_str(buf);

            if (rx < 1000) sprintf(buf, "%03dB", rx);
            else if (rx > 1000000) sprintf(buf, "%03dM", rx/1000000);
            else sprintf(buf, "%03dK", rx/1000);
            setCursor((127-(strlen(buf)+1)*11)/2,16);
            oled_write(25);
            print_str(buf);
            break;
        case MERGE:
            setTextSize(1);
            if (tx < 1000) sprintf(buf, "%03dB ", tx);
            else if (tx > 1000000) sprintf(buf, "%03dM", tx/1000000);
            else sprintf(buf, "%03dK ", tx/1000);
            setCursor((127-(2*strlen(buf)-1)*6)/2-4, y+4);
            oled_write(24);
            print_str(buf);

            if (rx < 1000) sprintf(buf, "%03dB", rx);
            else if (rx > 1000000) sprintf(buf, "%03dM", rx/1000000);
            else sprintf(buf, "%03dK", rx/1000);
            oled_write(25);
            print_str(buf);
            break;
        case FULL:
            setTextSize(1);
            setCursor(display_offset, y);
            oled_write(24);
            if (tx < 1000) sprintf(buf, "%03dB ", tx);
            else if (tx > 1000000) sprintf(buf, "%03dM", tx/1000000);
            else sprintf(buf, "%03dK ", tx/1000);
            print_str(buf);

            oled_write(25);
            if (rx < 1000) sprintf(buf, "%03dB", rx);
            else if (rx > 1000000) sprintf(buf, "%03dM", rx/1000000);
            else sprintf(buf, "%03dK", rx/1000);
            print_str(buf);
        }
    }
}
void testcpu(int y)
{
//freq
    setTextSize(1);
    setCursor(display_offset, y);
    if((fp=popen(FREQPATH,"r")) != NULL)
    {
        fgets(content_buff,FREQSIZE,fp);
        fclose(fp);
        sprintf(buf,"CPU:%4dMHz ", atoi(content_buff)/1000);
        print_str(buf);
    }


//temp
    if((fp=fopen(TEMPPATH,"r"))!=NULL)
    {
        fgets(content_buff,TEMPSIZE,fp);
        fclose(fp);
    	sprintf(buf, "%.2f",atoi(content_buff)/100.0); 
    	print_str(buf);
    	oled_write(0);
    	oled_write(67);
	drawCircle(getCursorX()-8, getCursorY()+1, 1, WHITE);
    }
}

void testprintinfo()
{
    setTextSize(1);
    setTextColor(WHITE); 
    setCursor(0,0);    
//DATE
   
    time_t rawtime;
    time_t curtime;
    uint8_t timebuff[TIMESIZE];
    curtime = time(NULL);
    time(&rawtime);
    strftime(timebuff,80,"%Y-%m-%d_%w %H:%M:%S",localtime(&rawtime));
    sprintf(buf,"%s",timebuff);
    print_strln(buf);
    
    //br-lan ip
    if((fp=popen(IPPATH,"r"))!=NULL)
    {
        fscanf(fp,"%s",content_buff);
        fclose(fp);
        //ipbuff[strlen(ipbuff)-1]=32;
        sprintf(buf,"IP:%s",content_buff);
        print_strln(buf);
    }

    //CPU temp
    if((fp=popen(FREQPATH,"r")) != NULL)
    {
        fgets(content_buff,FREQSIZE,fp);
        fclose(fp);
        sprintf(buf,"CPU freq:%d MHz ",atoi(content_buff)/1000);
        print_strln(buf);
    }

    //cpu freq
    if((fp=fopen(TEMPPATH,"r"))!=NULL)
    {
        fgets(content_buff,TEMPSIZE,fp);
        fclose(fp);
        sprintf(buf,"CPU temp:%.2f C",atoi(content_buff)/100.0);
        print_strln(buf);
    }



}

