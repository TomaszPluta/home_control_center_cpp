/*
 * gpio.h
 *
 *  Created on: 13.08.2018
 *      Author: tomek
 */

#ifndef GPIO_H_
#define GPIO_H_

#include <stdbool.h>
#include "stm32f10x.h"

bool IsGpioHigh(GPIO_TypeDef * gpioPort, uint8_t pinNb);

void EnableGpioClk (GPIO_TypeDef * gpioPort);

void SetGpioAsInput (GPIO_TypeDef * gpioPort, uint8_t pinNb);

void SetGpioAsInFloating (GPIO_TypeDef * gpioPort, uint8_t pinNb);

void SetGpioAsOutAltPushPUll (GPIO_TypeDef * gpioPort, uint8_t pinNb);


void SetGpioAsOutput(GPIO_TypeDef * gpioPort, uint8_t pinNb);


#endif /* GPIO_H_ */
