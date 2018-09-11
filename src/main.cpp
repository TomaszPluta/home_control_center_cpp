

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



volatile static int debug_var;
uint8_t temp;
uint8_t q = 0;
uint8_t data_array[32];
uint8_t big_array[1500];
uint8_t layer3_buff[230];
uint8_t tx_address[5] = {0xD7,0xD7,0xD7,0xD7,0xD7};
uint8_t rx_address[5] = {0xE7,0xE7,0xE7,0xE7,0xE7};
/* ----------------------------------------------------------- */


volatile uint8_t rxBuff[1024];
volatile uint16_t pos;

extern "C" void EXTI9_5_IRQHandler (void);


void EXTI9_5_IRQHandler (void){
	EXTI_ClearITPendingBit(EXTI_Line5);




		uint16_t status = Rfm12bWriteCmd(0x0000);

		if (status & RFM12_STATUS_FFIT ){
			uint8_t rx = rfm12bReadFifo();
				if (pos <1024){
					rxBuff[pos] = rx;
					pos++;
					if (pos==30){
						 GPIOC->ODR ^= GPIO_Pin_13;
						asm volatile ("nop");
						rfm12bFifoReset();
					 	pos =0;

					}
			}
		}

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

//		RFM12B_GPIO_Init();

	 	Rfm12bInit();
	// 	Rfm12bInitNode();
		rfm12bSwitchTx();
//

		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
		EnableExti(GPIOB, 5, false, true);
		SetGpioAsInPullUp(GPIOB, 5);
		SetGpioAsInPullUp(GPIOB, 11);



	 	while (1){




	 		  if (!(GPIOB->IDR & (1<<11))){

	 			 NVIC_DisableIRQ(EXTI9_5_IRQn);
	 			  rfm12bSwitchTx();

	 			  _delay_ms(50);

	 			  uint8_t buff[] = "helloWorld1helloWorld2helloWorld3";
	 			//  Rfm12bSendBuff(buff, 30);
	 			 RF12_TXPACKET(buff, 30);
	 			 NVIC_EnableIRQ(EXTI9_5_IRQn);
	 			  _delay_ms(250);
	 			 rfm12bSwitchRx();
	 			  _delay_ms(20);


	 		  }


//	 		uint8_t buff[] = "helloworld";
//	 		RFM12B_SendData(buff, 10);
//			_delay_ms(500);
//	 		uint16_t status = RFM12B_RDSTATUS();
//
//
//	 		if (status & RFM12_STATUS_RGIT ){
//	 			asm volatile ("nop");
//	 		}
//	 		if (status & RFM12_STATUS_FFIT ){
//	 			asm volatile ("nop");
//	 		}
//	 		if (status & RFM12_STATUS_POR ){
//	 			asm volatile ("nop");
//	 		}
//	 		if (status & RFM12_STATUS_RGUR ){
//	 			asm volatile ("nop");
//	 		}
//	 		if (status & RFM12_STATUS_FFOV ){
//	 			asm volatile ("nop");
//	 		}
//	 		if (status & RFM12_STATUS_WKUP ){
//	 			asm volatile ("nop");
//	 		}
//	 		if (status & RFM12_STATUS_EXT ){
//	 			asm volatile ("nop");
//	 		}
//	 		if (status & RFM12_STATUS_LBD ){
//	 			asm volatile ("nop");
//	 		}
//	 		if (status & RFM12_STATUS_FFEM ){
//	 			asm volatile ("nop");
//	 		}
//	 		if (status & RFM12_STATUS_ATS ){
//	 			asm volatile ("nop");
//	 		}
//	 		if (status & RFM12_STATUS_RSSI ){
//	 			asm volatile ("nop");
//	 		}
//	 		if (status & RFM12_STATUS_DQD ){
//	 			asm volatile ("nop");
//	 		}
//	 		if (status & RFM12_STATUS_CRL ){
//	 			asm volatile ("nop");
//	 		}
//	 		if (status & RFM12_STATUS_ATGL ){
//	 			asm volatile ("nop");
//	 		}


//			_delay_ms(100);


	 	}

}
