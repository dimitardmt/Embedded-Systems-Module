#include "stm32f4xx.h"
#include "cmsis_os.h"

SPI_HandleTypeDef SPI_Params; //Declares the structure handle for the parameters of the SPI, saves having to do mutiple initialisations 
uint8_t SwitchThreadsBool = 0; //Declares a variable to determine whether the button has been pressed or not

void Initialise_LED_BUTTON(void){
	
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN; 		// Enable Port D  
	GPIOD->MODER |= GPIO_MODER_MODER12_0; 		// green LED
	GPIOD->MODER |= GPIO_MODER_MODER13_0; 		// range LED
	GPIOD->MODER |= GPIO_MODER_MODER14_0; 		// red LED
	GPIOD->MODER |= GPIO_MODER_MODER15_0;		// blue LED

	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN; 		//Enable Port A 

	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN; 		// Enable timer 2 
	TIM2->CR1 &= ~0x00000016; 					//Set the counter to an upcounter
	TIM2->CR1 &= ~0x0000008; 					//Enable pulse mode
	TIM2->PSC = 8400-1; 						//Set the prescalar value to 8400
	TIM2->ARR = 10000-1; 						//Sets the auto-reload value
	TIM2->EGR = 1; 								//Re-initialise the counter
	TIM2->CR1 |= 1; 
}

void Initialise_SPI(uint16_t dataSize, uint32_t dataTimeout){
	
	uint8_t dataToSend[1]; 						//Declares a dataToSend array that will store the required register address
	
	GPIO_InitTypeDef GPIOA_Params; 				//Declares the structure handle for GPIOA
	GPIO_InitTypeDef GPIOE_Params; 				//Declares the structure handle for GPIOE
	GPIO_InitTypeDef GPIOE_Params_I; 			//Declares the structure handle for GPIOA set up as interrupts
	
	
	RCC->APB2ENR |= RCC_APB2ENR_SPI1EN; //Enable the clock for SPI1
	SPI_Params.Instance = SPI1; //Selects which SPI to use
	SPI_Params.Init.Mode = SPI_MODE_MASTER; //Sets STM32F407 as the master
	SPI_Params.Init.NSS = SPI_NSS_SOFT; //Sets the accelerometer to be controlled by the software 
	SPI_Params.Init.Direction = SPI_DIRECTION_2LINES; //Enables full-duplex
	SPI_Params.Init.DataSize = SPI_DATASIZE_8BIT; //Sets the data packet size of the SPI to 8-bit
	SPI_Params.Init.CLKPolarity = SPI_POLARITY_HIGH; //Sets the idle polarity for the clock line high  
	SPI_Params.Init.CLKPhase = SPI_PHASE_2EDGE; //Sets the data line to change phase on the second transition of the clock line
	SPI_Params.Init.FirstBit = SPI_FIRSTBIT_MSB; //Sets the transmission to be sent MSB first
	SPI_Params.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32; //Determine the SPI colck rate from the main APB2 clock, APB2 clock = 84MHz /32 = 2.625MHz 
	HAL_SPI_Init(&SPI_Params);  
	
//Parameters for the SCL SDO and SDI pins of port A
	
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN; //Enable the clock
	GPIOA_Params.Pin = GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7; //Selects pins 5, 6 and 7, the clock ,data line and output data  
	GPIOA_Params.Alternate = GPIO_AF5_SPI1; //Selects SPI1
	GPIOA_Params.Mode = GPIO_MODE_AF_PP; //Push pull mode
	GPIOA_Params.Speed = GPIO_SPEED_FAST; //Fast speed
	GPIOA_Params.Pull = GPIO_NOPULL; //No pull-up or pull down activation
	HAL_GPIO_Init(GPIOA, &GPIOA_Params); 
	
// Parameters for the SPI pins of port E 
	
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN; //Enable the clock for GPIOE
	GPIOE_Params.Pin = GPIO_PIN_3; //Selects Pin 3, the CS line used to initiate data communication
	GPIOE_Params.Mode = GPIO_MODE_OUTPUT_PP; //Selects normal push pull mode
	GPIOE_Params.Speed = GPIO_SPEED_FAST; //Selects fast speed
	GPIOE_Params.Pull = GPIO_PULLUP; //Selects pull-up activiation 
	HAL_GPIO_Init(GPIOE, &GPIOE_Params); //Configures GPIOE using the declared parameters
	
// Parameters for the interrupts of port E
	
	GPIOE_Params_I.Pin = GPIO_PIN_0; //Enable the interupt line 
	GPIOE_Params_I.Mode = GPIO_MODE_IT_RISING; //Sets the interupts to first have a falling edge followed by a rising one
	GPIOE_Params_I.Speed = GPIO_SPEED_FAST; //Selects fast speed
	HAL_GPIO_Init(GPIOE, &GPIOE_Params_I);


	GPIOE->BSRR = GPIO_PIN_3; //Enable line high
	__HAL_SPI_ENABLE(&SPI_Params); //Enable the the SPI
	
//Get the data from the LIS3DSH accelerometer
	
	dataToSend[0] = 0x20; //Address for control register 4 on the LIS3DSH to send data
	GPIOE->BSRR = GPIO_PIN_3<<16; //Communication enable line on the SPI to low 
	HAL_SPI_Transmit(&SPI_Params,dataToSend,dataSize,dataTimeout); //Transmit the address of the register to be read on the LIS3DSH

	dataToSend[0] = 0x13; //Enable X and Y axis
	HAL_SPI_Transmit(&SPI_Params,dataToSend,dataSize,dataTimeout); //Data to CR4 on LIS3DSH via the SPI channel
	GPIOE->BSRR = GPIO_PIN_3; //Sets the communication enable line high on the SPI
	
//Interrupts on the LIS3DSH 
	
	dataToSend[0] = 0x23; //Address for control register 3 on the LIS3DSH
	GPIOE->BSRR = GPIO_PIN_3<<16; //Communication enable line on the SPI to low 
	HAL_SPI_Transmit(&SPI_Params,dataToSend,dataSize,dataTimeout); //Transmit the address of the register to be read on the LIS3DSH

	dataToSend[0] = 0xC8; //Interrupts only when data is available. 
	HAL_SPI_Transmit(&SPI_Params,dataToSend,dataSize,dataTimeout); //Data to CR3 on LIS3DSH via the SPI channel
	GPIOE->BSRR = GPIO_PIN_3;  //Sets the communication enable line high on the SPI
	
}

//NVIC

void InitialiseIRQ(void){	
	HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0); //Sets priority of EXTIO to lowest
	HAL_NVIC_ClearPendingIRQ(EXTI0_IRQn); //Clears flags on EXTIO
	NVIC->ISER[0] = 1<<6; //Enables interupts on EXTIO
}

// EXTI0 handle
void EXTI0_IRQHandler(void){
	
	uint8_t dataToSend[1]; //Declares a dataToSend array that will store the required register address
	uint16_t dataSize=1; //1 address at a time
	uint32_t dataTimeout=1000; //Timeout for the SPI
	uint8_t X_Reg_H; //X-Axis data
	uint8_t X_Reg_H_Data; //X-Axis no sign
	uint8_t Y_Reg_H; //Y-Axis data
	uint8_t Y_Reg_H_Data; //Y-Axis no sign

//X axis data

		dataToSend[0] = 0x80|0x29; //Address for the X-Axis information
		GPIOE->BSRR = GPIO_PIN_3<<16; //SPI low so communication can be initialised
		HAL_SPI_Transmit(&SPI_Params,dataToSend,dataSize,dataTimeout); //SPI transmit to LIS3DSH accelerometer
		dataToSend[0] = 0x00; //Reset so data can be received
		HAL_SPI_Receive(&SPI_Params,dataToSend,dataSize,dataTimeout); //Recieve data LIS3DSH using SPI 
		GPIOE->BSRR = GPIO_PIN_3; //SPI high so communication can be stopped
		X_Reg_H = *SPI_Params.pRxBuffPtr; //Get the accelerometer data for X-Axis variable 
	


//Y axis data	

		dataToSend[0] = 0x80|0x2B; //Address for the Y-Axis information
		GPIOE->BSRR = GPIO_PIN_3<<16; //SPI low so communication can be initialised
		HAL_SPI_Transmit(&SPI_Params,dataToSend,dataSize,dataTimeout); //SPI transmit to LIS3DSH accelerometer
		dataToSend[0] = 0x00; //Reset so data can be received
		HAL_SPI_Receive(&SPI_Params,dataToSend,dataSize,dataTimeout); //Recieve data LIS3DSH using SPI 
		GPIOE->BSRR = GPIO_PIN_3; //SPI high so communication can be stopped
		Y_Reg_H = *SPI_Params.pRxBuffPtr; //Get the accelerometer data for Y-Axis variable 



		X_Reg_H_Data = X_Reg_H; 
		X_Reg_H_Data &= ~ 0x80; //Remove sign bit 
		Y_Reg_H_Data = Y_Reg_H; 
		Y_Reg_H_Data &= ~ 0x80; //Remove sign bit 
 

	
//Logic for using the LED to visualise the tilt of the accelerometer
	
		if (((Y_Reg_H_Data < 110) && (Y_Reg_H_Data > 20))) { //If the board stars getting inclined the LED should display it
			if((Y_Reg_H&0x80) == 0x80){ 					//Switch on the blue LED and switch off orange LED
				GPIOD->BSRR |= (1<<15);
				GPIOD->BSRR |= (1<<(13+16));
			} else { 										//Switch on the orange LED and switch off the blue one
				GPIOD->BSRR |= (1<<13);
				GPIOD->BSRR |= (1<<(15+16));
			} 
		} else { 											//turn off Y-axis LEDs
			GPIOD->BSRR |= (1<<(13+16));
			GPIOD->BSRR |= (1<<(15+16));
		}
	
		if (((X_Reg_H_Data < 110) && (X_Reg_H_Data > 20))) { //If the board stars getting inclined the LED should display it
			if((X_Reg_H&0x80) == 0x80){ 					//Switch on the green LED and switch off red LEDIf sign bit is high
				GPIOD->BSRR |= (1<<12);
				GPIOD->BSRR |= (1<<(14+16));
			} else {										//Switch on the red LED and switch off the green one
				GPIOD->BSRR |= (1<<14);
				GPIOD->BSRR |= (1<<(12+16));
			} 
		} else {											//Turn off X-axis LEDS
			GPIOD->BSRR |= (1<<(12+16));
			GPIOD->BSRR |= (1<<(14+16));
		}
	
		HAL_NVIC_ClearPendingIRQ(EXTI0_IRQn); //Clear EXTIO interrupt from NVIC
		EXTI->PR|=(1<<1); //Clear bit flag for EXTI0 interrupt 
}
