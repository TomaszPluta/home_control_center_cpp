


	 		 /*
	 		  * gpio.c
	 		  *
	 		  *  Created on: 13.08.2018
	 		  *      Author: tomek
	 		  */

	 		 #include <stdint.h>
	 		 #include <stdbool.h>
	 		 #include <stm32f10x.h>
	 		 #include "gpio.h"


	 		 #define PIN_MODE_MSK			(0x3)
	 		 #define PIN_CNF_MSK				(0x3)
	 		 #define PIN_CNF_FLOAT			(0x1)
	 		 #define PIN_CNF_PUSH_PULL		(0x2)
	 		 #define PIN_MODE_OUT			(0x1)



	 		 void EnableGpioClk (GPIO_TypeDef * gpioPort){
	 		 	if (gpioPort == GPIOA){
	 		 		RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
	 		 	} else if (gpioPort == GPIOB){
	 		 		RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
	 		 	} else if (gpioPort == GPIOC){
	 		 		RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;
	 		 	} else if (gpioPort == GPIOD){
	 		 		RCC->APB2ENR |= RCC_APB2ENR_IOPDEN;
	 		 	}
	 		 }

	 		 void SetGpioAsInput (GPIO_TypeDef * gpioPort, uint8_t pinNb){//
	 		 	if (pinNb < 8){
	 		 		gpioPort->CRL &= ~(PIN_MODE_MSK<<pinNb);
	 		 	} else {
	 		 		gpioPort->CRH &= ~(PIN_MODE_MSK<<(pinNb-8)*4);
	 		 	}
	 		 }


	 		 void SetGpioAsInFloating (GPIO_TypeDef * gpioPort, uint8_t pinNb){
	 		 	if (pinNb < 8){
	 		 		gpioPort->CRL &= ~(PIN_MODE_MSK<<pinNb);
	 		 		gpioPort->CRL &= ~(PIN_CNF_MSK<<pinNb);
	 		 		gpioPort->CRL |= (PIN_CNF_FLOAT<<pinNb);

	 		 	} else {
	 		 		gpioPort->CRH &= ~(PIN_MODE_MSK<<(pinNb-8)*4);
	 		 		gpioPort->CRH &= ~(PIN_CNF_MSK<<(pinNb-8)*4);
	 		 		gpioPort->CRH |= (PIN_CNF_FLOAT<<(((pinNb-8)*4)+2));
	 		 	}
	 		 }




	 		 void SetGpioAsInPullUp (GPIO_TypeDef * gpioPort, uint8_t pinNb){
	 		 	if (pinNb < 8){
	 		 		gpioPort->CRL &= ~(PIN_MODE_MSK<<(pinNb*4));
	 		 		gpioPort->CRL &= ~(PIN_CNF_MSK<<((pinNb*4)+2));
	 		 		gpioPort->CRL |= (PIN_CNF_PUSH_PULL<<((pinNb*4)+2));
	 		 		gpioPort->ODR |= (1<<pinNb);

	 		 	} else {
	 		 		gpioPort->CRH &= ~(PIN_MODE_MSK<<(pinNb-8)*4);
	 		 		gpioPort->CRH &= ~(PIN_CNF_MSK<<((pinNb-8)*4)+2);
	 		 		gpioPort->CRH |= (PIN_CNF_PUSH_PULL<<(((pinNb-8)*4)+2));
	 		 		gpioPort->ODR |=  (1<<pinNb);
	 		 	}
	 		 }




	 		 void SetGpioAsOutAltPushPUll (GPIO_TypeDef * gpioPort, uint8_t pinNb){
	 		 	if (pinNb < 8){
	 		 //		gpioPort->CRL &= ~(PIN_MODE_MSK<<pinNb);
	 		 //		gpioPort->CRL &= ~(PIN_CNF_MSK<<pinNb);
	 		 //
	 		 //		gpioPort->CRL |= (PIN_MODE_OUT<<pinNb);
	 		 //		gpioPort->CRL &= ~(PIN_CNF_PUSH_PULL<<pinNb);

	 		 	} else {
	 		 		gpioPort->CRH &= ~(PIN_MODE_MSK<<(pinNb-8)*4);
	 		 		gpioPort->CRH &= ~(PIN_CNF_MSK<<(((pinNb-8)*4)+2));

	 		 		gpioPort->CRH |= (PIN_MODE_OUT<<(pinNb-8)*4);
	 		 		gpioPort->CRH |= (PIN_CNF_PUSH_PULL<<(((pinNb-8)*4)+2));
	 		 	}
	 		 }



	 		 void SetGpioAsOutput(GPIO_TypeDef * gpioPort, uint8_t pinNb){
	 		 //	if (pinNb < 8){
	 		 //		gpioPort->CRL |= (PIN_MODE_MSK<<pinNb);
	 		 //	} else {
	 		 //		gpioPort->CRH |= (PIN_MODE_MSK<<((pinNb-8)*4));
	 		 //	}
	 		 }


	 		 bool IsGpioHigh(GPIO_TypeDef * gpioPort, uint8_t pinNb){
	 		 	return (gpioPort->IDR & (1<<pinNb));
	 		 }



	 		 void EnableExti(GPIO_TypeDef * gpioPort, uint8_t pinNb, bool rise, bool fall){
	 		 	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;
	 		 	AFIO->EXTICR[1] |= AFIO_EXTICR2_EXTI5_PB;
	 		 	EXTI->IMR  |= (1<<pinNb);
	 		 	if (rise){
	 		 		EXTI->RTSR |= (1<<pinNb);
	 		 	}
	 		 	if (fall){
	 		 		EXTI->FTSR |= (1<<pinNb);
	 		 		//SetGpioAsInPullUp(GPIOB, 5);
	 		 	}
	 		 	NVIC_EnableIRQ(EXTI9_5_IRQn);


	 		 }




	 		 void _delay_ms(int n) {

	 		 	int i, j;
	 		 	j= n*1000;
	 		 	while(j--) {
	 		 		i=2;
	 		 		while(i--);
	 		 	}
	 		 }


