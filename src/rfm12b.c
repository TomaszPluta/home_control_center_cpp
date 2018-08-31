/*
 * rfm12b.c
 *
 *  Created on: 30.08.2018
 *      Author: tomek
 */
#include "stm32f10x.h"
#include "RFM12B_reg.h"
#include <stdint.h>
#include <stdbool.h>



void RFM12B_GPIO_Init(void) {

	SPI_InitTypeDef  SPI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Enable SPI1 and GPIOA clocks */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);


	//SPI - SCK, MISO, MOSI
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);


	//CS hdwr
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);


	/* SPI1 configuration */
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_16b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_SSOutputCmd(SPI1, ENABLE);
	SPI_Init(SPI1, &SPI_InitStructure);

	/* Enable SPI1  */
	SPI_Cmd(SPI1, ENABLE);


	GPIO_InitTypeDef PORT;

	// Enable the nRF24L01 GPIO peripherals
	RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;

	// Configure CSN pin
	PORT.GPIO_Mode = GPIO_Mode_Out_PP;
	PORT.GPIO_Speed = GPIO_Speed_2MHz;
	PORT.GPIO_Pin = GPIO_Pin_1;
	GPIO_Init(GPIOB, &PORT);
	GPIO_SetBits(GPIOB, GPIO_Pin_1);
}



uint16_t SpiTransfer(uint16_t data) {

	 // Wait until TX buffer is empty
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
	// Send byte to SPI (TXE cleared)
	SPI_I2S_SendData(SPI1, data);
	// Wait while receive buffer is empty
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);

	// Return received byte
	return SPI_I2S_ReceiveData(SPI1);
}

#define RFM12B_NSEL_L()              GPIO_ResetBits(GPIOB, GPIO_Pin_1);
#define RFM12B_NSEL_H()               GPIO_SetBits(GPIOB, GPIO_Pin_1);


uint16_t Rfm_xmit(uint16_t data){
	RFM12B_NSEL_L();
	uint16_t spiRxData = SpiTransfer(data);
	RFM12B_NSEL_H()
	return spiRxData;
}

bool Rfm12bTest (void){
	Rfm_xmit(SW_RESET);

	static uint16_t status;
	do{

		_delay_ms(250);
		status = Rfm_xmit(STATUS_READ);

	}while(!(status & M_POR));


	status=Rfm_xmit(STATUS_READ);


	if(status&M_POR){
		return false;
	}else{
		return true;
	}
}



void Rfm_init(void){
	Rfm_xmit(SW_RESET);//resetuję programowo układ RFM12B
	_delay_ms(250);

	//inicjalizacja RFM12B
	//ustawienie pasma 868MHz, konfiguracja FIFO
	Rfm_xmit(CONFIGURATION|EN_DATA_REG|EN_FIFO|BAND_868|CAP_12_0);
	//włączenie oscylatora
	Rfm_xmit(POWER|EN_OSC|DIS_CLKO);
	//ustawienie pasma (musi być takie samo w nadajniku i odbiorniku)
	//Dla naszego układu częstotliwość musi zawierać się w przedziale 860480000-879515000Hz i musi być podawana ze skokami 5000Hz
	Rfm_xmit(FREQUENCY|RF12_FREQUENCY_CALC_868(868000000UL));
	//ustawienie prędkości transmisji danych (musi być takia sama w nadajniku i odbiorniku)
	Rfm_xmit(BAUD|BAUD_4800);
	//ustawienie pasma 134kHz i parametrów odbiornika
	Rfm_xmit(RECEIVER|P20_VDI|BW134|LNA_0|RSSI_103);
	//ustawienie cyfrowego filtra danych i opcji odzyskiwania zegara
	Rfm_xmit(DATA_FILTER|AUTO_CR|DIGITAL_F|DQD_4);
	//reset bufora FIFO, konfiguracja synchronizacji za pomocą 2 bajtów, ustawienie generowania przerwania FFIT po odebraniu 8 bitów
	Rfm_xmit(FIFO_RST|FIFO_IT_8|FIFO_SYNC|HS_RST_DIS);


	//konfiguracja kontrolera częstotliwości
	Rfm_xmit(AFC|KEEP_RX|REST_OFF|EN_FOFFSET|EN_AFC);
	//konfiguracja nadajnika i jego mocy (na ustawienie maksymalne)
	Rfm_xmit(TRANSMITER|FSK_PHASE_0|FSK_DF_90K|OUT_P_0);
	//konfiguracja pętli PLL
	Rfm_xmit(PLL|PLL_DH_DIS|SEL_CLK_2_5|MAX_BAUD_256);

	//wyłączenie timera wakeup
	Rfm_xmit(WAKEUP_TIM|WUT_X_2_0|0);
	//wyłączenie oszczędzania energii
	Rfm_xmit(LOW_DC|LOW_DC_DIS);
	//ustawienie monitora napięcia na 2,2V
	Rfm_xmit(BOD_CLK|CLK_5_00|BOD_2_2);
}



uint8_t Rfm_ready_wait(void){

		uint16_t data;
		while(1) {
			data = Rfm_xmit(0x0000);
			if ( (data&0x8000) ) {
				data = Rfm_xmit(0xB000);
				return (data&0x00FF);
			}
		}


//	uint8_t i=0;
//
//	RFM12B_NSEL_L();
//	_delay_ms(1);//czekam nieco aby układ mógł zareagować
//
//	while(!(IsGpioHigh(GPIOA, GPIO_Pin_6))){//następnie co 1ms sprawdzam, czy układ jest wolny, tzn, czy wystawił na linii MISO 1.
//		_delay_ms(1);
//		if((++i)==200){
//			return 1;//po ok. 200ms przerywam oczekiwanie i zwracam 1 - czyli sygnalizuję błąd
//		}
//	}
//
//	RFM12B_NSEL_H();
//
//	return 0;//jeśli pętla zostanie przerwana z powodu wystąpienia stanu wysokiego na linii MISO to zwracam 0 - nie ma błędu
}


void Rfm12bRx (void){
	RFM12B_GPIO_Init();//inicjalizacja magistrali SPI

	Rfm_init();//inicjalizujemy układ RFM12B

	Rfm_xmit(SYNC_PATTERN|0xD4);
	//ustawiamy programowalny bajt synchronizacji na wartość 0xD4
	//wykorzystamy tę funkcjonalność do adresowania wielu układów

	//włączamy odbiornik
	Rfm_xmit(POWER|EN_RX|EN_BASEBAND|EN_OSC|DIS_CLKO);
	_delay_ms(5);

	while(1){
		//para komend powodująca w efekcie działania reset synchronizacji odbiornika
		Rfm_xmit(FIFO_RST|FIFO_IT_8|HS_RST_DIS);
		Rfm_xmit(FIFO_RST|FIFO_IT_8|EN_AFT_SYNC|HS_RST_DIS);

		uint8_t timeout=Rfm_ready_wait();//oczekujemy na odebranie przez układ danych
		if(timeout){//i w zależności od tego czy układ odpowiedział
			//albo wyświetlamy informacje o braku odebranych danych
			//"NO INCOMING DATA\r\n"
		}else{
			//albo odbieramy dane z bufora
			uint8_t data=Rfm_xmit(FIFO_READ);
		}
	}
}



void Rfm12bSend (void){

	RFM12B_GPIO_Init();//inicjalizacja magistrali SPI

	Rfm_init();//wstępna konfiguracja układu RFM12B


	uint8_t data;

	while(1){
		//sprawdzam przyciski i ustawiam odpowiednie bity w bajcie do nadania
		data = 'x';

		//włączamy nadajnik
		Rfm_xmit(POWER|EN_TRANSMISSION|EN_SYNTH|EN_OSC|DIS_CLKO);
		//nadajemy 3 bajty synchronizacji (0xAA)
		//dwa pierwsze są już domyślnie umieszczone w buforze nadawczym po komendzie włączającej nadajnik
		Rfm_ready_wait();
		Rfm_xmit(TX_WRITE|0xAA);
		//następnie kolejny bajt synchrnoizacji (0x2D)
		Rfm_ready_wait();
		Rfm_xmit(TX_WRITE|0x2D);
		Rfm_ready_wait();
		//oraz definiowalny bajt synchronizacji (czyli nasz adres)
		Rfm_xmit(TX_WRITE|0xD4);
		Rfm_ready_wait();
		Rfm_xmit(TX_WRITE|data);
		Rfm_ready_wait();
		//oraz pusty bajt - konieczny by poprawnie zakończyć trnasmisję
		Rfm_xmit(TX_WRITE|0xAA);
	//	Rfm_ready_wait();
		//następnie czekamy na zakończenie transmisji
		uint16_t status;
		do
		{
			status=Rfm_xmit(STATUS_READ);
		} while (!(status&M_TX_READY));
		//i wyłączamy nadajnik
		Rfm_xmit(POWER|DIS_CLKO);
		_delay_ms(1000);
	}


}

