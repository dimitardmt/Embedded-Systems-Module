#include "stm32f4xx.h"

void Initialise_LED_BUTTON(void); //initialise the LEDs and TIM2
void Initialise_SPI(uint16_t, uint32_t); //initialise SPI and GPIOs
void InitialiseIRQ(void); //initialise NVIC

extern SPI_HandleTypeDef SPI_Params; //Declare SPI handle
extern uint8_t SwitchThreadsBool; //Declare the external flag for thread switches