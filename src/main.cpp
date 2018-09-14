

#include <stdint.h>
#include "nrf24.h"
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "mqtt_client.h"
#include "tiny_broker.h"
#include "string.h"

#include "gpio.h"
#include "platform.h"


#include "__rfm12b.h"
#include "__rfm12b_platform.h"



uint8_t temp;
uint8_t q = 0;
uint8_t data_array[32];
uint8_t big_array[1500];
uint8_t layer3_buff[230];
uint8_t tx_address[5] = {0xD7,0xD7,0xD7,0xD7,0xD7};
uint8_t rx_address[5] = {0xE7,0xE7,0xE7,0xE7,0xE7};
/* ----------------------------------------------------------- */

rfm12bBuff_t sendBuffIRQ;
volatile uint8_t rxBuff[1024];
volatile uint16_t pos;

extern "C" void EXTI9_5_IRQHandler (void);



void EXTI9_5_IRQHandler (void){
	EXTI_ClearITPendingBit(EXTI_Line5);

	Rfm12bIrqCallback (&sendBuffIRQ);

}



int mqtt_net_disconnect_cb(void *context){
	return 0;
}

MqttNet net;

static 	void gpio_init(void){
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

	GPIO_InitTypeDef PORT;
	PORT.GPIO_Mode = GPIO_Mode_Out_PP;
	PORT.GPIO_Speed = GPIO_Speed_2MHz;
	PORT.GPIO_Pin = GPIO_Pin_13;
	GPIO_Init(GPIOC, &PORT);
	GPIOC->ODR |= GPIO_Pin_13;

	PORT.GPIO_Mode =  GPIO_Mode_IPU;
	PORT.GPIO_Speed = GPIO_Speed_2MHz;
	PORT.GPIO_Pin = GPIO_Pin_11;
	GPIO_Init(GPIOB, &PORT);
	GPIOC->ODR |= GPIO_Pin_13;
}



int broker_conn(void *cntx, sockaddr_t * sockaddr){
	return 1;
}

int broker_send(void *cntx, sockaddr_t * sockaddr, const uint8_t* buf, uint16_t buf_len){
  	l3_send_packet (0, (uint8_t*) buf, buf_len);
    _delay_ms(50);
    return 1;
}

int broker_rec(void *cntx, sockaddr_t * sockaddr, uint8_t* buf, uint16_t  buf_len){
	if(nrf24_dataReady())
	{

		nrf24_getData(data_array);
		 uint16_t pcktLen = l3_receive_packet(data_array, buf, buf_len);
		if (pcktLen){
			//sockaddr->sin_addr = Mqtt_get_rx_address();
			return true;
			GPIOC->BRR = GPIO_Pin_13;
		}
	}
	_delay_ms(10);
	return false;
}

int broker_discon(void *context, sockaddr_t * sockaddr){
	return 1;
}




int main(){


 	EnableGpioClk(LOG_UART_PORT);
 	SetGpioAsOutAltPushPUll(LOG_UART_PORT, LOG_UART_PIN_TX);
 	SetGpioAsInFloating(LOG_UART_PORT, LOG_UART_PIN_RX);
 	EnableUart(USART1);




 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
 	GPIO_InitTypeDef PORT;
 	PORT.GPIO_Mode = GPIO_Mode_Out_PP;
 	PORT.GPIO_Speed = GPIO_Speed_2MHz;
 	PORT.GPIO_Pin = GPIO_Pin_13;
 	GPIO_Init(GPIOC, &PORT);
 	GPIOC->ODR |= GPIO_Pin_13;




 	Rfm12bInit();
 	_delay_ms(1000);	//wymagane opoznienie
 	uint8_t sst =   Rfm12bWriteCmd(0x0000);
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
 	EnableExti(GPIOB, 5, false, true);
 	SetGpioAsInPullUp(GPIOB, 5);
	SetGpioAsInPullUp(GPIOB, 11);

 	rfm12bFifoReset();
 	rfm12bSwitchRx();

 	 NVIC_EnableIRQ(EXTI9_5_IRQn);

 	//  NVIC_DisableIRQ(EXTI9_5_IRQn);

	 	while (1){




	 		  if (!(GPIOB->IDR & (1<<11))){

//	 			  NVIC_DisableIRQ(EXTI9_5_IRQn);
	 			  uint8_t buff[] = "helloWorld1helloWorld2helloWorld3";
	 			  Rfm12bStartSending(&sendBuffIRQ, buff, 30);

//	 			  uint16_t status = Rfm12bWriteCmd(0x0000);//??
//	 			  rfm12bSwitchTx();

	 		//	 NVIC_EnableIRQ(EXTI9_5_IRQn);

	 			//  _delay_ms(50);

	 			//  Rfm12bSendBuff(buff, 30);
	 			 //RF12_TXPACKET(buff, 30);


//	 			 NVIC_EnableIRQ(EXTI9_5_IRQn);
//	 			rfm12bSwitchRx();
	 			 _delay_ms(250);
	 		//	  _delay_ms(20);


	 		  }


	 	}

}
