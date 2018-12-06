
#include "cmsis_os.h"																				//CMSIS RTOS header file
#include "stm32f4xx.h"
#include "my_headers.h"

void Tilt_Indicator_Thread (void const *argument); // Declares the tilt indicator thread 
osThreadId tid_Tilt_Indicator_Thread; // ID of the tilt indicator thread
osThreadDef (Tilt_Indicator_Thread, osPriorityNormal, 1, 0); // Defines the tilt indicator thread, with normal priority

void Red_LED_Blinking_Thread (void const *argument); // Declares the blinking  LED thread 
osThreadId tid_Red_LED_Blinking_Thread; //ID of the blinking  LED 
osThreadDef (Red_LED_Blinking_Thread, osPriorityNormal, 1, 0); // Defines the blink red LED thread, with normal priority

// Thread 1 - blinking the red LED 

// Initialisation of the thread
int Init_Red_LED_Blinking_Thread(void){
    tid_Red_LED_Blinking_Thread = osThreadCreate(osThread(Red_LED_Blinking_Thread), NULL);  //Creates blinking red LED object and assigns the tid
    if(!tid_Red_LED_Blinking_Thread) return(-1); //Checks that the thread object has been created
    return(0); //If not created return 0
}



void Red_LED_Blinking_Thread(void const *argument){ 
    uint16_t ButtonPressedTimer= 0; //Declares a ButtonPressedTimer variable, to ensure that a contact of the button has been made. 
                                    //This was done because a quick tap on the button was not registered and this way a better solutions was created.
    osSignalClear(tid_Red_LED_Blinking_Thread,0x01);  //Clear flag on the blinkinkg red LED thread
    
    while(1){
            osSignalWait(0x01, osWaitForever); //Suspend current thread until a thread flag has been reset
            osSignalSet(tid_Red_LED_Blinking_Thread, 0x01); //Set the flag of blink red LED thread to break the wait
            
            while((TIM2->SR&0x0001)!=1){ //1 second delay using TIM2
                GPIOD->BSRR = 1<<14; //Switch on the red LED
                
                while(((GPIOA->IDR & 0x00000001) == 0x00000001) & (SwitchThreadsBool == 1)) { //While button is pressed 
                    ButtonPressedTimer++;                                                  //Count for how long the button has been pressed
            
                    if (ButtonPressedTimer > 250) {                                            //If the button has been pressed for longer(ensuring that it has been detected) prepare to switch threads
                        SwitchThreadsBool = 0;              
                    }
                }
                
                if (((GPIOA->IDR & 0x00000001) != 0x00000001) & (SwitchThreadsBool == 0)) { //If the button has not been pressed and switch thread has been enabled
                    ButtonPressedTimer = 0;                                                 //Reset counter
                    InitialiseIRQ();                                                         //Enable NVIC 
                    osSignalSet(tid_Tilt_Indicator_Thread, 0x01);                           //Set the flag of tilt indicator thread to break thread
                    osSignalWait(0x01, osWaitForever);                                      //Suspend current thread until another one is set
                }
            }
            
            TIM2->SR &= ~1;                                                                 //Resets the flag 
            
            while((TIM2->SR&0x0001)!=1){                                                         //1 second delay using TIM2
                GPIOD->BSRR = 1<<(14+16);                                                       //Switch on the red LED
                while(((GPIOA->IDR & 0x00000001) == 0x00000001) & (SwitchThreadsBool == 1)) {   //While button is pressed 
                    ButtonPressedTimer++;                                                       //Count for how long the button has been pressed
    
                    if (ButtonPressedTimer > 250) {     //If the button has been pressed for longer(ensuring that it has been detected) prepare to switch threads
                        SwitchThreadsBool = 0;
                    }
                }
                
                if (((GPIOA->IDR & 0x00000001) != 0x00000001) & (SwitchThreadsBool == 0)) {  //If the button has not been pressed and switch thread has been enabled
                    ButtonPressedTimer = 0;                                                  //Reset counter
                    InitialiseIRQ();                                                        //Enable NVIC 
                    osSignalSet(tid_Tilt_Detector_Thread, 0x01);                            //Set the flag of tilt indicator thread to break thread
                    osSignalWait(0x01, osWaitForever);                                       //Suspend current thread until appropriate thread is set
                }
            }       
            
            TIM2->SR &= ~1;                                                                      //Resets the flag 
            
        osThreadYield();
    }
}


// Thread 2 - Detecting the tilt indication using the LEDs
 
// Initialisation of the thread
 
int Init_Tilt_Indicator_Thread(void){
  tid_Tilt_Indicator_Thread = osThreadCreate (osThread(Tilt_Indicator_Thread), NULL); //Creates tilt indicator object declared above and assigns the tid
  if (!tid_Tilt_Indicator_Thread) return(-1); //Checks that the thread object has been created
  return(0); //If not created return 0
}



void Tilt_Indicator_Thread (void const *argument) {
	

	uint16_t ButtonPressedTimer= 0; //Declares a ButtonPressedTimer variable, to ensure that a contact of the button has been made. 
                                    //This was done because a quick tap on the button was not registered and this way a better solutions was created.
	
	osSignalSet(tid_Tilt_Indicator_Thread, 0x01); //Clear flag on the tilt indicator thread
		
  while (1) {

		
		while(((GPIOA->IDR & 0x00000001) == 0x00000001) & (SwitchThreadsBool == 0)) { //While button is pressed 
			ButtonPressedTimer++;			                                         //Count for how long the button has been pressed
			if (ButtonPressedTimer > 250) {			                                   //If the button has been pressed for longer(ensuring that it has been detected) prepare to switch threads
				SwitchThreadsBool = 1;
			}
		}
			
		if (((GPIOA->IDR & 0x00000001) != 0x00000001) & (SwitchThreadsBool == 1)) { //If the button has not been pressed and switch thread has been enabled
			
			ButtonPressedTimer = 0;													//Reset counter
			HAL_NVIC_DisableIRQ(EXTI0_IRQn);								        //Disable NVIC 
			GPIOD->BSRR = 1<<(12+16);												//Switch off green LED
			GPIOD->BSRR = 1<<(13+16);												//Switch off orange LED
			GPIOD->BSRR = 1<<(14+16);												//Switch off red LED
			GPIOD->BSRR = 1<<(15+16);												//Switch off blue LED
			osSignalSet(tid_Red_LED_Blinking_Thread, 0x01);		                    //Set the flag of blinking LED thread
			osSignalWait(0x01, osWaitForever);							             //Suspend current thread until appropriate thread is set
		}
    osThreadYield ();                                     
  }
}


