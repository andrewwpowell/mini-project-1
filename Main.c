#include <stdint.h>
//#include <stdio.h>
#include "PLL.h"
#include "LCD.h"
#include "os.h"
#include "joystick.h"
#include "FIFO.h"
#include "PORTE.h"
#include "tm4c123gh6pm.h"

// Constants
#define BGCOLOR     					LCD_BLACK
#define CROSSSIZE            			5

//------------------Defines and Variables-------------------
uint16_t origin[2]; // the original ADC value of x,y if the joystick is not touched
int16_t x = 63;  // horizontal position of the crosshair, initially 63
int16_t y = 63;  // vertical position of the crosshair, initially 63
int16_t prevx = 63;
int16_t	prevy = 63;
uint8_t select;  // joystick push
uint16_t rawX, rawY; // To hold raw adc values


//---------------------User debugging-----------------------

#define TEST_TIMER 0	// Change to 1 if testing the timer
#define TEST_PERIOD 4000000  // Defined by user
#define PERIOD 4000000  		// Defined by user

unsigned long Count;   		// number of times thread loops


//--------------------------------------------------------------
void CrossHair_Init(void){
	BSP_LCD_FillScreen(LCD_BLACK);	// Draw a black screen
	BSP_Joystick_Input(&origin[0], &origin[1], &select); // Initial values of the joystick, used as reference
}

//******** Producer *************** 
void Producer(void){
#if TEST_TIMER
	PE1 ^= 0x02;	// heartbeat
	Count++;	// Increment dummy variable			
#else
	// Variable to hold updated x and y values
	int16_t newX = x;
	int16_t newY = y;
	int16_t deltaX = 0;
	int16_t deltaY = 0;
	
	uint8_t select;	// To hold pushbutton status
	rxDataType data;
	BSP_Joystick_Input(&rawX, &rawY, &select);

	//How to use raw values from adc to get new position?
	
	//Values at disposal : origin[2], x, y, prevx, prevy, newX, newY, deltaX, deltaY, rawX, rawY
	
	//Calculate next position suing raw values and put into FIFO
	
	//Boundary Conditions?
	
	//If raw values are within a close enough range of origin values do not move 
	//the crosshair to reduce sensitivity
	
	//Ensures joystick has been moved significantly
	if(rawX - origin[0] > 100 || rawX - origin[0] < -100 || rawY - origin[1] > 100 || rawY - origin[1] < -100) {
		
		deltaX = rawX/31 - x;
		deltaY = (127 - rawY/31) - y;
	
		newX = x + deltaX;
		newY = y + deltaY;
		
		//Save previous coordinates of crosshair
		prevx = x;
		prevy = y;
		
		//Boundary Conditions
		if(newX < 0) {
			x = 0;
		}
		else if(newX > 127) {
			x = 127;
		}
		else
			x = newX;
		
		if(newY < 0) {
			y = 0;
		}
		else if(newY > 127) {
			y = 127;
		}
		else
			y = newY;
		
		data.x = x;
		data.y = y;
		
		RxFifo_Put(data);
	}
#endif
}

//******** Consumer *************** 
void Consumer(void){
	rxDataType data;
	// Your Code Here
	BSP_LCD_DrawCrosshair(prevx,prevy,LCD_BLACK);
	
	RxFifo_Get(&data);
	//Display x value at bottom of screen
	BSP_LCD_Message(1,12,5,"X: ",data.x);
	//Display y value at bottom of screen
	BSP_LCD_Message(1,12,10,"Y: ",data.y);
	//Display Crosshair
	BSP_LCD_DrawCrosshair(data.x,data.y,LCD_RED);
}

//******** Main *************** 
int main(void){
  PLL_Init(Bus80MHz);       // set system clock to 80 MHz
#if TEST_TIMER
	PortE_Init();       // profile user threads
	Count = 0;
	OS_AddPeriodicThread(&Producer, TEST_PERIOD, 1);
	while(1){}
#else
  	BSP_LCD_Init();        // initialize LCD
	BSP_Joystick_Init();   // initialize Joystick
  	CrossHair_Init();      
 	RxFifo_Init();
	OS_AddPeriodicThread(&Producer,PERIOD, 1);
	while(1){
		Consumer();
	}
#endif
} 
