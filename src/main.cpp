

#include <stdint.h>
#include "nrf24.h"
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "mqtt_client.h"
#include "tiny_broker.h"
#include "string.h"

#include "gpio.h"
#include "platform.h"

#include "rfm12b.h"
#include "rfm12bMCK.h"

#include "rfm12bAM.h"


volatile static int debug_var;
uint8_t temp;
uint8_t q = 0;
uint8_t data_array[32];
uint8_t big_array[1500];
uint8_t layer3_buff[230];
uint8_t tx_address[5] = {0xD7,0xD7,0xD7,0xD7,0xD7};
uint8_t rx_address[5] = {0xE7,0xE7,0xE7,0xE7,0xE7};
/* ----------------------------------------------------------- */







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



//
//
//void RFM12B_GPIO_Init(void) {
//
//	SPI_InitTypeDef  SPI_InitStructure;
//	GPIO_InitTypeDef GPIO_InitStructure;
//
//	/* Enable SPI1 and GPIOA clocks */
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
//
//
//	//SPI - SCK, MISO, MOSI
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
//	GPIO_Init(GPIOA, &GPIO_InitStructure);
//
//
//	//CS hdwr
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
//	GPIO_Init(GPIOB, &GPIO_InitStructure);
//
//
//	/* SPI1 configuration */
//	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
//	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
//	SPI_InitStructure.SPI_DataSize = SPI_DataSize_16b;
//	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
//	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
//	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
//	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;
//	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
//	SPI_InitStructure.SPI_CRCPolynomial = 7;
//	SPI_SSOutputCmd(SPI1, ENABLE);
//	SPI_Init(SPI1, &SPI_InitStructure);
//
//	/* Enable SPI1  */
//	SPI_Cmd(SPI1, ENABLE);
//
//
//	GPIO_InitTypeDef PORT;
//
//	// Enable the nRF24L01 GPIO peripherals
//	RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
//
//	// Configure CSN pin
//	PORT.GPIO_Mode = GPIO_Mode_Out_PP;
//	PORT.GPIO_Speed = GPIO_Speed_2MHz;
//	PORT.GPIO_Pin = GPIO_Pin_1;
//	GPIO_Init(GPIOB, &PORT);
//	GPIO_SetBits(GPIOB, GPIO_Pin_1);
//}
//
//
//
//uint16_t SpiTransfer(uint16_t data) {
//
//	 // Wait until TX buffer is empty
//	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
//	// Send byte to SPI (TXE cleared)
//	SPI_I2S_SendData(SPI1, data);
//	// Wait while receive buffer is empty
//	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
//
//	// Return received byte
//	return SPI_I2S_ReceiveData(SPI1);
//}
//
//#define RFM12B_NSEL_L()              GPIO_ResetBits(GPIOB, GPIO_Pin_1);
//#define RFM12B_NSEL_H()               GPIO_SetBits(GPIOB, GPIO_Pin_1);




int main(){



	 	EnableGpioClk(LOG_UART_PORT);
	 	SetGpioAsOutAltPushPUll(LOG_UART_PORT, LOG_UART_PIN_TX);
	 	SetGpioAsInFloating(LOG_UART_PORT, LOG_UART_PIN_RX);
	 	EnableUart(USART1);

		RFM12B_GPIO_Init();


		//RFM12B_TXInit();


		rfInitAM();





	 	while (1){

	 		rfSendString("helloWorld");
	 		_delay_ms(500);

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
