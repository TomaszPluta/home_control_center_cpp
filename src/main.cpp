

#include <stdint.h>
#include "nrf24.h"
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "mqtt_client.h"
#include "tiny_broker.h"
#include "string.h"

#include "gpio.h"
#include "platform.h"


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
  	nrf24_send(data_array);
    _delay_ms(50);
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





int main()
{




	nrf24_init();
	nRF24_restore_defaults();


	/* Channel #2 , payload length: 4 */
	const uint8_t channel = 2;
	const uint8_t payload_len = 32;
	nrf24_config(channel, payload_len);

	/* Set the device addresses */
	nrf24_tx_address(tx_address);
	nrf24_rx_address(rx_address);

	gpio_init();


	broker_net_t broker_net;
	broker_net.connect = broker_conn;
	broker_net.send = broker_send;
	broker_net.receive = broker_rec;
	broker_net.disconnect = broker_discon;
	broker_t broker;
	broker_init_by_given_net(&broker, &broker_net);

	sockaddr_t sockaddr;
	uint8_t frameBuff[MAX_FRAME_SIZE];


	EnableGpioClk(LOG_UART_PORT);
	SetGpioAsOutAltPushPUll(LOG_UART_PORT, LOG_UART_PIN_TX);
	SetGpioAsInFloating(LOG_UART_PORT, LOG_UART_PIN_RX);
	EnableUart(USART1);




	SendString("Start center");
	while(1)
	{
		if (broker_receive(&broker, frameBuff, &sockaddr)){
			SendString("Received frame:");
			SendBuffer((const char*)frameBuff, MAX_FRAME_SIZE);
			broker_packets_dispatcher(&broker, frameBuff, &sockaddr);
		}
	}
}
