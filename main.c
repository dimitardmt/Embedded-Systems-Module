/*----------------------------------------------------------------------------
 * CMSIS-RTOS 'main' function template
 *---------------------------------------------------------------------------*/

#define osObjectsPublic                     // define objects in main module
#include "osObjects.h"                      // RTOS object definitions
#include "stm32f4xx.h"
#include "my_headers.h"
#include "Thread.h"




/*
 * main: initialize and start the system
 */
 
int main (void) {
  osKernelInitialize ();                    //Initialize CMSIS-RTOS

	// Initialise everything else
	
	uint16_t data_size = 1;										//1 address at a time for the LIS3DSH accelerometer address
	uint32_t data_timeout = 1000;								//Timeout for the SPI
		
	Initialise_LED_BUTTON();									//Initialise the LED and Button
	
	Initialise_SPI(data_size, data_timeout);  					//Initialise the SPI
	
	InitialiseIRQ();											//Initialise the NVIC
	
	Init_Tilt_Indicator_Thread();								//Initialise the Tilt indicator thread
	
	Init_Red_LED_Blinking_Thread();								//Initialise the blinking RED LED thread

  osKernelStart ();                         					// start thread execution 
	while(1){};													//While loop so the program doesn't stop
}

