
#include <string>
#include <stdint.h>
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "mqtt_client.h"
#include "tiny_broker.h"
#include "string.h"

#include "gpio.h"
#include "platform.h"

#include "__rfm12b.h"
#include "__rfm12b_platform.h"

#include "tm_stm32_ili9341.h"
#include "spi.h"

#include "sd.h"
#include "/home/tomek/STM32_workspace_9.0/home_control_center_cpp/src/FATFs/diskio.h"


#define BROKER_ADDR		(1)


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




void ReadTouch(void)
{

#define XPT_2046_READ_X (0xD2)
#define XPT_2046_READ_Y (0x92)


	uint16_t TouchXval;
	uint16_t TouchYval;;

	CS_LOW

	SpiTransmit(0xD2);
	TouchXval = SpiTransmit(0x00);

	SpiTransmit(0x92);
	TouchYval = SpiTransmit(0x00);

	CS_HIGH

}





extern "C" void EXTI15_10_IRQHandler (void);

void EXTI15_10_IRQHandler (void){
	ReadTouch();
	EXTI_ClearFlag(EXTI_Line12);
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
	Rfm12bStartSending(obj, (uint8_t *)buff, buffLen, (uint8_t)sockaddr->sin_addr.s_addr);
	return buffLen;
}

int broker_rec(void *cntx, sockaddr_t * sockaddr, uint8_t* buf, uint16_t  buf_len){
	rfm12bObj_t * obj = (rfm12bObj_t*) cntx;
	uint8_t byteNb = obj->completedRxBuff.dataNb;
	if (byteNb > 0){
		byteNb = (byteNb < buf_len) ? byteNb : buf_len;
		memcpy (buf, (const void*) obj->completedRxBuff.data, byteNb);
		obj->completedRxBuff.dataNb = 0;
		sockaddr->sin_addr.s_addr = obj->completedRxBuff.rxFromAddr;
	}

	return byteNb;
}

int broker_discon(void *context, sockaddr_t * sockaddr){
	return 1;
}


void _delay_ms(uint32_t x){
	;
}



std::string intToString(uint32_t intVal){
	/*due to not available "to_string()" c++11 func*/
	const uint8_t intSize = 8;
	char charVal[intSize];
	snprintf(charVal, intSize, "%u", intVal);
	std::string strVal(charVal);
	return strVal;
}




uint16_t SpiTrans( uint16_t cmd )
{
	NSEL_RFM12_LOW;
	/* Loop while DR register in not emplty */
	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);

	/* Send byte through the SPI1 peripheral */
	SPI_I2S_SendData(SPI2, cmd);

	/* Wait to receive a byte */
	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET);

	/* Return the byte read from the SPI bus */
	uint16_t recData = SPI_I2S_ReceiveData(SPI2);

	NSEL_RFM12_HIGH;

	return recData;
}



void LCD_BMP(const char * nazwa_pliku)
{

	  TM_ILI9341_DrawPixel(80, 80, ILI9341_COLOR_RED);


	u32 i = 0, j = 0, liczba_pikseli = 0, liczba_bajtow =0;
	u16 piksel;
	u8 temp[4];
	UINT ile_bajtow;
	FRESULT fresult;
	FIL plik;
	// Otwarcie do odczytu pliku bitmapy
	fresult = f_open (&plik, (const char *) nazwa_pliku, FA_READ);
	// Opuszczenie dwoch pierwszych bajtow
	fresult = f_read (&plik, &temp[0], 2, &ile_bajtow);
	// rozmiar pliku w bajtach
	fresult = f_read (&plik, (u8*) &liczba_bajtow, 4, &ile_bajtow);
	// Opuszczenie 4 bajtow
	fresult = f_read (&plik, &temp[0], 4, &ile_bajtow);
	// Odczytanie przesuniecia (offsetu) od poczatku pliku do
	// poczatku bajtow opisujacych obraz
	fresult = f_read (&plik, (u8*) &i, 4, &ile_bajtow);
	// Opuszczenie liczby bajtow od aktualnego miejsca
	// do poczatku danych obrazu, wartosc 14, bo odczytane zostalo
	// juz z pliku 2+4+4+4=14 bajtow
	for(j = 0; j < (i - 14); j++){
		fresult = f_read (&plik, &temp[0], 1, &ile_bajtow);
	}
	// Liczba pikseli obrazu = (rozmiar pliku - offset)/2 bajty na pisel
	liczba_pikseli = (liczba_bajtow - i)/2;
	// Ustawienie parametrow pracy LCD (m. in. format BGR 5-6-5)
//	LCD_WriteReg(R3, 0x1008);
//	LCD_WriteRAM_Prepare();
	// Odczyt bajtow z karty SD i wyslanie danych do LCD
	TM_ILI9341_SetCursorPosition(20, 20, 20, 20);
	TM_ILI9341_SendCommand(0x2C);


	//for(i = 0; i < liczba_pikseli; i++)

		for(i = 0; i < 100; i++)
	{
		fresult = f_read (&plik, (u8*) &piksel, 2, &ile_bajtow);
	//	LCD_WriteRAM(piksel);
		//TM_ILI9341_DrawPixel(i, 30, piksel);

		  TM_ILI9341_DrawPixel(30, 30, ILI9341_COLOR_RED);
		  TM_ILI9341_DrawPixel(i, 30, piksel);

//			TM_ILI9341_SendData(piksel >> 8);
//			TM_ILI9341_SendData(piksel & 0xFF);


	}
	//LCD_CtrlLinesWrite(GPIOB, CtrlPin_NCS, Bit_SET);
	// Przywrocenie ustawien LCD
//	LCD_WriteReg(R3, 0x1018);
	// Zamyka plik
	fresult = f_close (&plik);
}



int main(){

	spiInit();


	SetGpioAsInFloating(GPIOA, 12);
	EnableExtiGeneral(0, 12, false, true);


	uint8_t buffSD[2048];
	memset(buffSD, 0, 2048);
	disk_initialize(0);

//	uint8_t inputBuff[128];
//	memset(inputBuff, 'x', 128);
//	disk_write(0,inputBuff,0, 1);
//	disk_read(0,buffSD,0, 1);

	FATFS fatDrive;
	bool res = f_mount( 0, &fatDrive);
	FIL fp;
	res = f_open(&fp, "dataFile.txt", FA_OPEN_ALWAYS | FA_WRITE | FA_READ);
	uint8_t writeBuff[] = "-- Start new log -- ";
	UINT writtenBytes;

	uint8_t readBuff[16];
	UINT readBytes;
	res = f_read(&fp,readBuff, 16, &readBytes);

	 TM_ILI9341_Init();
	LCD_BMP("bgg.bmp");





	  TM_ILI9341_DrawPixel(10, 10, ILI9341_COLOR_ORANGE);
	  TM_ILI9341_DrawPixel(11, 11, ILI9341_COLOR_ORANGE);
	  TM_ILI9341_DrawPixel(12, 12, ILI9341_COLOR_ORANGE);
	  TM_ILI9341_DrawPixel(20, 20, ILI9341_COLOR_YELLOW);
	  TM_ILI9341_DrawPixel(30, 30, ILI9341_COLOR_BLUE);

//
//		FATFS fatDrive;
//		f_mount( 0, &fatDrive);
//
//		FIL fp;
//
//		uint8_t write_buff[] = "start log file";
//		UINT writtenBytes;
//		f_open(&fp, "data.log",FA_CREATE_NEW | FA_WRITE);
//		f_write(&fp, write_buff, sizeof(write_buff), &writtenBytes);
//		f_close(&fp);
//


	 TM_ILI9341_Puts(0,0, "Temp.  22.46", &TM_Font_16x26, ILI9341_COLOR_BLUE, ILI9341_COLOR_BLACK);
	// TM_ILI9341_Puts(0,25, "Hum.  57%", &TM_Font_16x26, ILI9341_COLOR_CYAN, ILI9341_COLOR_BLACK);
//	 TM_ILI9341_Puts(0,50, "Out1  ON", &TM_Font_16x26, ILI9341_COLOR_GREEN, ILI9341_COLOR_BLACK);
//	 TM_ILI9341_Puts(0,75, "Out2  OFF", &TM_Font_16x26, ILI9341_COLOR_GRAY, ILI9341_COLOR_BLACK);





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


 	Rrm12bObjInit (&rfm12bObj, BROKER_ADDR);



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


uint16_t i =0;
	 	while (1){
	 		if (broker_receive(&broker, frameBuff, &sockaddr)){
	 			if (frameBuff[42] == 'T'){
	 				TM_ILI9341_Puts(0,0, "Temp.  99.88", &TM_Font_16x26, ILI9341_COLOR_BLUE, ILI9341_COLOR_BLACK);
	 			} else{
	 				TM_ILI9341_Puts(0,25, "cnt: ", &TM_Font_16x26, ILI9341_COLOR_BLUE, ILI9341_COLOR_BLACK);
	 				i++;
	 				std::string cnt;
	 				cnt = intToString(i);
	 				frameBuff[86]=0;
	 				TM_ILI9341_Puts(100,25, (char*) cnt.data(), &TM_Font_16x26, ILI9341_COLOR_BLUE, ILI9341_COLOR_BLACK);
	 				TM_ILI9341_Puts(100,50, (char*) &frameBuff[16], &TM_Font_16x26, ILI9341_COLOR_BLUE, ILI9341_COLOR_BLACK);

	 			}






	 			broker_packets_dispatcher(&broker, frameBuff, &sockaddr);

	 		}

	 		  if (!(GPIOB->IDR & (1<<11))){
	 			  uint8_t buff[] = "abcdefghijabcdefghijabcdefghij";
	 			  Rfm12bStartSending(&rfm12bObj, buff, 30, 2);
	 			 _delay_ms(250);


	 		  }




	 	}

}
