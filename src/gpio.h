/*
 * gpio.h
 *
 *  Created on: 13.08.2018
 *      Author: tomek
 */

#ifndef GPIO_H_
#define GPIO_H_

#ifdef __cplusplus
extern "C" {
#endif


#include <stdbool.h>
#include "stm32f10x.h"

bool IsGpioHigh(GPIO_TypeDef * gpioPort, uint8_t pinNb);

void EnableGpioClk (GPIO_TypeDef * gpioPort);

void SetGpioAsInput (GPIO_TypeDef * gpioPort, uint8_t pinNb);

void SetGpioAsInFloating (GPIO_TypeDef * gpioPort, uint8_t pinNb);

void SetGpioAsOutAltPushPUll (GPIO_TypeDef * gpioPort, uint8_t pinNb);

void SetGpioAsInPullUp (GPIO_TypeDef * gpioPort, uint8_t pinNb);

void SetGpioAsOutput(GPIO_TypeDef * gpioPort, uint8_t pinNb);


 void EnableExti(GPIO_TypeDef * gpioPort, uint8_t pinNb, bool rise, bool fall);

#ifdef __cplusplus
}
#endif

#endif /* GPIO_H_ */
