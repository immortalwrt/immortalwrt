/*
 * Main.c
 *
 *  Created on  : Sep 6, 2017
 *  Author      : Vinay Divakar
 *  Description : Example usage of the SSD1306 Driver API's
 *  Website     : www.deeplyembedded.org
 */

/* Lib Includes */ 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

/* Header Files */
#include "I2C.h"
#include "SSD1306_OLED.h"
#include "example_app.h"

/* Oh Compiler-Please leave me as is */
volatile unsigned char flag = 0;

/* Alarm Signal Handler */
void ALARMhandler(int sig)
{
    /* Set flag */
    flag = 5;
}

void BreakDeal(int sig)
{
    clearDisplay();
    usleep(1000000);    
    Display();
    exit(0);
}


int main(int argc, char* argv[])
{	
    int date=atoi(argv[1]);
    int lanip=atoi(argv[2]);
    int cputemp=atoi(argv[3]);
    int cpufreq=atoi(argv[4]);
    int netspeed=atoi(argv[5]);
    int time=atoi(argv[6]);
    int drawline=atoi(argv[7]);
    int drawrect=atoi(argv[8]);
    int fillrect=atoi(argv[9]);
    int drawcircle=atoi(argv[10]);
    int drawroundcircle=atoi(argv[11]);
    int fillroundcircle=atoi(argv[12]);
    int drawtriangle=atoi(argv[13]);
    int filltriangle=atoi(argv[14]);
    int displaybitmap=atoi(argv[15]);
    int displayinvertnormal=atoi(argv[16]);
    int drawbitmapeg=atoi(argv[17]);
    int scroll=atoi(argv[18]);
    char *text=argv[19];   
    char *eth = argv[20];
    int needinit=atoi(argv[21]);


    /* Initialize I2C bus and connect to the I2C Device */
    if(init_i2c_dev(I2C_DEV0_PATH, SSD1306_OLED_ADDR) == 0)
    {
        printf("(Main)i2c-2: Bus Connected to SSD1306\r\n");
    }
    else
    {
        printf("(Main)i2c-2: OOPS! Something Went Wrong\r\n");
        exit(1);
    }

    /* Register the Alarm Handler */
    signal(SIGALRM, ALARMhandler);
    signal(SIGINT, BreakDeal);
    //signal(SIGTERM, BreakDeal);
/* Run SDD1306 Initialization Sequence */
    if (needinit==1) {display_Init_seq();}

    /* Clear display */
    clearDisplay();

    // draw a single pixel
//    drawPixel(0, 1, WHITE);
//    Display();
//    usleep(1000000);
//    clearDisplay();

    // draw many lines
    while(1){

	if(scroll){
	    testscrolltext(text);
	    usleep(1000000);
	    clearDisplay();
	}	

        if(drawline){
            testdrawline();
            usleep(1000000);
            clearDisplay();
        }


        // draw rectangles
        if(drawrect){
            testdrawrect();
            usleep(1000000);
            clearDisplay();
        }

        // draw multiple rectangles
        if(fillrect){
            testfillrect();
            usleep(1000000);
            clearDisplay();
        }

        // draw mulitple circles
        if(drawcircle){
            testdrawcircle();
            usleep(1000000);
            clearDisplay();
        }


        // draw a white circle, 10 pixel radius
        if(drawroundcircle){
            testdrawroundrect();
            usleep(1000000);
            clearDisplay();
        }


        // Fill the round rectangle
        if(fillroundcircle){
            testfillroundrect();
            usleep(1000000);
            clearDisplay();
        }

        // Draw triangles
        if(drawtriangle){
            testdrawtriangle();
            usleep(1000000);
            clearDisplay();
        }
        // Fill triangles
        if(filltriangle){
            testfilltriangle();
            usleep(1000000);
            clearDisplay();
        }

        // Display miniature bitmap
        if(displaybitmap){
            display_bitmap();
            Display();
            usleep(1000000);
        };
        // Display Inverted image and normalize it back
        if(displayinvertnormal){
            display_invert_normal();
            clearDisplay();
            usleep(1000000);
            Display();
		
        }

        // Generate Signal after 20 Seconds

        // draw a bitmap icon and 'animate' movement
        if(drawbitmapeg){
	    alarm(10);
	    flag=0;
            testdrawbitmap_eg();
            clearDisplay();
            usleep(1000000);
            Display();
        }

        
        //setCursor(0,0);   
	setTextColor(WHITE); 
        // info display
	int sum = date+lanip+cpufreq+cputemp+netspeed;
	if (sum == 0) {clearDisplay(); return 0;}
	 for(int i = 1; i < time; i++){	
	   if (sum == 1){//only one item for display
	   	if (date) testdate(CENTER, 8);
	   	if (lanip) testlanip(CENTER, 8);
	   	if (cpufreq) testcpufreq(CENTER, 8);
	   	if (cputemp) testcputemp(CENTER, 8);
	   	if (netspeed) testnetspeed(SPLIT,0);
		Display();
        	usleep(1000000);
        	clearDisplay();
	   }else if (sum == 2){//two items for display
		if(date) {testdate(CENTER, 16*(date-1));}
            	if(lanip) {testlanip(CENTER, 16*(date+lanip-1));}
            	if(cpufreq) {testcpufreq(CENTER, 16*(date+lanip+cpufreq-1));}
            	if(cputemp) {testcputemp(CENTER, 16*(date+lanip+cpufreq+cputemp-1));}
            	if(netspeed) {testnetspeed(MERGE, 16*(date+lanip+cpufreq+cputemp+netspeed-1));}
		Display();
        	usleep(1000000);
        	clearDisplay();
	   }
	   else{//more than two items for display
            	if(date) {testdate(FULL, 8*(date-1));}
            	if(lanip) {testlanip(FULL, 8*(date+lanip-1));}
		if(cpufreq && cputemp) {
			testcpu(8*(date+lanip));
			if(netspeed) {testnetspeed(FULL, 8*(date+lanip+1+netspeed-1));}
		}
		else{
            		if(cpufreq) {testcpufreq(FULL, 8*(date+lanip+cpufreq-1));}
            		if(cputemp) {testcputemp(FULL, 8*(date+lanip+cpufreq+cputemp-1));}
			if(netspeed) {testnetspeed(FULL, 8*(date+lanip+cpufreq+cputemp+netspeed-1));}
		}
            	
        	Display();
        	usleep(1000000);
        	clearDisplay();
        }
	   }
    }

}
