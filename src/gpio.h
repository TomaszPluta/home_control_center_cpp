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


#define GPIO_PortA       ((uint8_t)0x00)
#define GPIO_PortB       ((uint8_t)0x01)
#define GPIO_PortD       ((uint8_t)0x03)
#define GPIO_PortC       ((uint8_t)0x02)
#define GPIO_PortE       ((uint8_t)0x04)
#define GPIO_PortF       ((uint8_t)0x05)
#define GPIO_PortG       ((uint8_t)0x06)


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


 void EnableExtiGeneral(uint8_t gpioPortNb, uint8_t pinNb, bool rise, bool fall);

#ifdef __cplusplus
}
#endif

#endif /* GPIO_H_ */
