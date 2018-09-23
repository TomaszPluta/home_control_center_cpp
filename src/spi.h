/*
 * spi.h
 *
 *  Created on: 23.09.2018
 *      Author: tomek
 */

#ifndef SPI_H_
#define SPI_H_


#ifdef __cplusplus
extern "C" {
#endif


#define CS_LOW              GPIO_ResetBits(GPIOA, GPIO_Pin_11);
#define CS_HIGH             GPIO_SetBits(GPIOA, GPIO_Pin_11);



void spiInit(void);


uint16_t SPiTransmit( uint16_t cmd );

#ifdef __cplusplus
}
#endif


#endif /* SPI_H_ */
