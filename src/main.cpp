

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
volatile rfm12bObj_t rfm12bObj;


void EXTI9_5_IRQHandler (void){
	EXTI_ClearITPendingBit(EXTI_Line5);

	Rfm12bIrqCallback (&rfm12bObj);

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

int broker_send(void *cntx, sockaddr_t * sockaddr, const uint8_t* buff, uint16_t buffLen){
	rfm12bObj_t * obj = (rfm12bObj_t*) cntx;
	Rfm12bStartSending(obj, (uint8_t *)buff, buffLen);
	return buffLen;
}

int broker_rec(void *cntx, sockaddr_t * sockaddr, uint8_t* buf, uint16_t  buf_len){
	rfm12bObj_t * obj = (rfm12bObj_t*) cntx;
	uint8_t byteNb = obj->completedRxBuff.dataNb;
	if (byteNb > 0){
		byteNb = (byteNb < buf_len) ? byteNb : buf_len;
		memcpy (buf, (const void*) obj->completedRxBuff.data, byteNb);
	}
	obj->completedRxBuff.dataNb = 0;
	return byteNb;
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


 	Rrm12bObjInit (&rfm12bObj);



	broker_net_t broker_net;
	broker_net.context = (void*) &rfm12bObj;
	broker_net.connect = broker_conn;
	broker_net.send = broker_send;
	broker_net.receive = broker_rec;
	broker_net.disconnect = broker_discon;
	broker_t broker;
	broker_init_by_given_net(&broker, &broker_net);

	sockaddr_t sockaddr;
	uint8_t frameBuff[MAX_FRAME_SIZE];



	 	while (1){
	 		if (broker_receive(&broker, frameBuff, &sockaddr)){
	 			broker_packets_dispatcher(&broker, frameBuff, &sockaddr);

	 		}

	 		  if (!(GPIOB->IDR & (1<<11))){
	 			  uint8_t buff[] = "helloWorld1helloWorld2helloWorld3";
	 			  Rfm12bStartSending(&rfm12bObj, buff, 30);
	 			 _delay_ms(250);


	 		  }


	 	}

}
