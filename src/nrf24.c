/*
* ----------------------------------------------------------------------------
 * “THE COFFEEWARE LICENSE” (Revision 1):
 * <ihsan@kehribar.me> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a coffee in return.
 * -----------------------------------------------------------------------------
 * This library is based on this library:
 *   https://github.com/aaronds/arduino-nrf24l01
 * Which is based on this library:
 *   http://www.tinkerer.eu/AVRLib/nRF24L01
 * -----------------------------------------------------------------------------
 */
#include "nrf24.h"
#include <string.h>
uint8_t payload_len;


void _delay_ms(int n) {

	int i, j;
	j= n*1000;
	while(j--) {
		i=2;
		while(i--);
	}
}



uint16_t calculateCRC16(const uint8_t * data_p, uint16_t length){
    uint8_t x;
    uint16_t crc = 0xFFFF;

    while (length--){
        x = crc >> 8 ^ *data_p++;
        x ^= x>>4;
        crc = (crc << 8) ^ ((uint16_t)(x << 12)) ^ ((uint16_t)(x <<5)) ^ ((uint16_t)x);
    }
    return crc;
}


void l3_send_packet (uint8_t addr, uint8_t * data, uint8_t len){
	uint8_t pos=0;
	uint8_t frame[FRAME_SIZE];
	uint8_t frm_nb = 0;
	uint8_t total_frames_nb;
	uint8_t remaining_len = len;
	uint8_t size_to_send;

	total_frames_nb = (len + (L2_DATA_SIZE - 1)) / L2_DATA_SIZE;

	while (frm_nb < total_frames_nb){
		memset(&frame[0], 0, FRAME_SIZE);

		frame[H_FRM] = frm_nb;
		frame[H_TOTAL] = total_frames_nb;
		frame[H_LENH] = len >> 8;
		frame[H_LENL] = (uint8_t) len & 0xFF;
		uint16_t calcCRC = calculateCRC16(data, len);
		frame[H_CRCH] = calcCRC >> 8;
		frame[H_CRCL] = calcCRC & 0xFF;

		size_to_send = (remaining_len > L2_DATA_SIZE)? L2_DATA_SIZE : remaining_len;
		memcpy(&frame[0 + L2_HEAD_SIZE], &data[pos], size_to_send);
		nrf24_send(frame);

		remaining_len -= size_to_send;
		pos += L2_DATA_SIZE;
		frm_nb++;
	}
}




uint16_t l3_receive_packet(uint8_t *data, uint8_t * packet_buff, uint16_t buff_len){

	uint8_t localBuff[240];

	if (data[H_FRM] == 0){
		memset (localBuff, 0, 240);
		memset (packet_buff, 0, buff_len);
	}

	if (data[H_FRM] < data[H_TOTAL]){
		uint8_t frm_nb = data[H_FRM];
		memcpy(&localBuff[L2_DATA_SIZE * frm_nb], &data[0+ L2_HEAD_SIZE], L2_DATA_SIZE);
		if ((data[H_FRM]+1) == data[H_TOTAL]){
			uint16_t pcktLen = (data[H_LENH] << 8) + data[H_LENL];
			uint16_t rxPcktCRC =  (data[H_CRCH]<<8)  + data[H_CRCL];
			uint16_t calcCRC = calculateCRC16(localBuff, pcktLen);
			if (rxPcktCRC == calcCRC){
				memcpy(packet_buff, localBuff, 240);
				memset (localBuff, 0, 240);
				return pcktLen;
			}
		}
	}
	return 0;
}




/* init the hardware pins */
void nrf24_init() {
	nrf24_setupPins();
	nrf24_ce_digitalWrite(LOW);
	nrf24_csn_digitalWrite(HIGH);
}


// Set transceiver to it's initial state
// note: RX/TX pipe addresses remains untouched
void nRF24_restore_defaults(void) {
	// Write to registers their initial values
	nrf24_configRegister(nRF24_REG_CONFIG, 0x08);
	nrf24_configRegister(nRF24_REG_EN_AA, 0x3F);
	nrf24_configRegister(nRF24_REG_EN_RXADDR, 0x03);
	nrf24_configRegister(nRF24_REG_SETUP_AW, 0x03);
	nrf24_configRegister(nRF24_REG_SETUP_RETR, 0x03);
	nrf24_configRegister(nRF24_REG_RF_CH, 0x02);
	nrf24_configRegister(nRF24_REG_RF_SETUP, 0x0E);
	nrf24_configRegister(nRF24_REG_STATUS, 0x00);
	nrf24_configRegister(nRF24_REG_RX_PW_P0, 0x00);
	nrf24_configRegister(nRF24_REG_RX_PW_P1, 0x00);
	nrf24_configRegister(nRF24_REG_RX_PW_P2, 0x00);
	nrf24_configRegister(nRF24_REG_RX_PW_P3, 0x00);
	nrf24_configRegister(nRF24_REG_RX_PW_P4, 0x00);
	nrf24_configRegister(nRF24_REG_RX_PW_P5, 0x00);
	nrf24_configRegister(nRF24_REG_DYNPD, 0x00);
	nrf24_configRegister(nRF24_REG_FEATURE, 0x00);

	// Clear the FIFO's
	spi_transfer(FLUSH_RX);
    spi_transfer(FLUSH_TX);

	// Clear any pending interrupt flags
	// Clear RX_DR, TX_DS and MAX_RT bits of the STATUS register
    uint8_t reg;

	nrf24_readRegister(nRF24_REG_STATUS, &reg, 1);
	reg |= nRF24_MASK_STATUS_IRQ;
	//nRF24_WriteReg(nRF24_REG_STATUS, reg);
	nrf24_configRegister(nRF24_REG_STATUS, reg);


}



/* configure the module */
void nrf24_config(uint8_t channel, uint8_t pay_length) {
	/* Use static payload length ... */
	payload_len = pay_length;

	// Set RF channel
	nrf24_configRegister(RF_CH, channel);

	// Set length of incoming payload
	nrf24_configRegister(RX_PW_P0, 0x00); // Auto-ACK pipe ...
	nrf24_configRegister(RX_PW_P1, payload_len);
	nrf24_configRegister(RX_PW_P2, payload_len);
	nrf24_configRegister(RX_PW_P3, payload_len);
	nrf24_configRegister(RX_PW_P4, payload_len);
	nrf24_configRegister(RX_PW_P5, payload_len);

	// 1 Mbps, TX gain: 0dbm
	nrf24_configRegister(RF_SETUP, (1 << RF_DR_LOW) | ((0x03) << RF_PWR));
	// CRC enable, 1 byte CRC length
	nrf24_configRegister(CONFIG, nrf24_CONFIG);

	// Auto Acknowledgment
	nrf24_configRegister(EN_AA,
			(1 << ENAA_P0) | (1 << ENAA_P1) | (0 << ENAA_P2) | (0 << ENAA_P3)
					| (0 << ENAA_P4) | (0 << ENAA_P5));

	// Enable RX addresses
	nrf24_configRegister(EN_RXADDR,
			(1 << ERX_P0) | (1 << ERX_P1) | (0 << ERX_P2) | (0 << ERX_P3)
					| (0 << ERX_P4) | (0 << ERX_P5));

	// Auto retransmit delay: 1000 us and Up to 15 retransmit trials
	nrf24_configRegister(SETUP_RETR, (0x04 << ARD) | (0x0F << ARC));

	// Dynamic length configurations: No dynamic length
	nrf24_configRegister(DYNPD,
			(0 << DPL_P0) | (0 << DPL_P1) | (0 << DPL_P2) | (0 << DPL_P3)
					| (0 << DPL_P4) | (0 << DPL_P5));

	// Start listening
	nrf24_powerUpRx();
}

/* Set the RX address */
void nrf24_rx_address(uint8_t * adr) {
	nrf24_ce_digitalWrite(LOW);
	nrf24_writeRegister(RX_ADDR_P1, adr, nrf24_ADDR_LEN);
	nrf24_ce_digitalWrite(HIGH);
}

/* Returns the payload length */
uint8_t nrf24_payload_length() {
	return payload_len;
}

/* Set the TX address */
void nrf24_tx_address(uint8_t* adr) {
	/* RX_ADDR_P0 must be set to the sending addr for auto ack to work. */
	nrf24_writeRegister(RX_ADDR_P0, adr, nrf24_ADDR_LEN);
	nrf24_writeRegister(TX_ADDR, adr, nrf24_ADDR_LEN);
}

/* Checks if data is available for reading */
/* Returns 1 if data is ready ... */
uint8_t nrf24_dataReady() {
	// See note in getData() function - just checking RX_DR isn't good enough
	uint8_t status = nrf24_getStatus();

	// We can short circuit on RX_DR, but if it's not set, we still need
	// to check the FIFO for any pending packets
	if (status & (1 << RX_DR)) {
		return 1;
	}

	return !nrf24_rxFifoEmpty();;
}

/* Checks if receive FIFO is empty or not */
uint8_t nrf24_rxFifoEmpty() {
	uint8_t fifoStatus;

	nrf24_readRegister(FIFO_STATUS, &fifoStatus, 1);

	return (fifoStatus & (1 << RX_EMPTY));
}

/* Returns the length of data waiting in the RX fifo */
uint8_t nrf24_payloadLength() {
	uint8_t status;
	nrf24_csn_digitalWrite(LOW);
	spi_transfer(R_RX_PL_WID);
	status = spi_transfer(0x00);
	nrf24_csn_digitalWrite(HIGH);
	return status;
}

/* Reads payload bytes into data array */
void nrf24_getData(uint8_t* data) {
	/* Pull down chip select */
	nrf24_csn_digitalWrite(LOW);

	/* Send cmd to read rx payload */
	spi_transfer( R_RX_PAYLOAD);

	/* Read payload */
	nrf24_transferSync(data, data, payload_len);

	/* Pull up chip select */
	nrf24_csn_digitalWrite(HIGH);

	/* Reset status register */
	nrf24_configRegister(STATUS, (1 << RX_DR));
}

/* Returns the number of retransmissions occured for the last message */
uint8_t nrf24_retransmissionCount() {
	uint8_t rv;
	nrf24_readRegister(OBSERVE_TX, &rv, 1);
	rv = rv & 0x0F;
	return rv;
}

// Sends a data package to the default address. Be sure to send the correct
// amount of bytes as configured as payload on the receiver.
void nrf24_send(uint8_t* value) {
	/* Go to Standby-I first */
	nrf24_ce_digitalWrite(LOW);

	/* Set to transmitter mode , Power up if needed */
	nrf24_powerUpTx();
    _delay_ms(50);
	/* Do we really need to flush TX fifo each time ? */
#if 0
	/* Pull down chip select */
	nrf24_csn_digitalWrite(LOW);

	/* Write cmd to flush transmit FIFO */
	spi_transfer(FLUSH_TX);

	/* Pull up chip select */
	nrf24_csn_digitalWrite(HIGH);
#endif

	/* Pull down chip select */
	nrf24_csn_digitalWrite(LOW);

	/* Write cmd to write payload */
	spi_transfer(W_TX_PAYLOAD);

	/* Write payload */
	nrf24_transmitSync(value, payload_len);

	/* Pull up chip select */
	nrf24_csn_digitalWrite(HIGH);

	/* Start the transmission */
	nrf24_ce_digitalWrite(HIGH);
	while(nrf24_isSending());
}

uint8_t nrf24_isSending() {
	uint8_t status;

	/* read the current status */
	status = nrf24_getStatus();

	/* if sending successful (TX_DS) or max retries exceded (MAX_RT). */
	if ((status & ((1 << TX_DS) | (1 << MAX_RT)))) {
		return 0; /* false */
	}

	return 1; /* true */

}

uint8_t nrf24_getStatus() {
	uint8_t rv;
	nrf24_csn_digitalWrite(LOW);
	rv = spi_transfer(NOP);
	nrf24_csn_digitalWrite(HIGH);
	return rv;
}

uint8_t nrf24_lastMessageStatus() {
	uint8_t rv;

	rv = nrf24_getStatus();

	/* Transmission went OK */
	if ((rv & ((1 << TX_DS)))) {
		return NRF24_TRANSMISSON_OK;
	}
	/* Maximum retransmission count is reached */
	/* Last message probably went missing ... */
	else if ((rv & ((1 << MAX_RT)))) {
		return NRF24_MESSAGE_LOST;
	}
	/* Probably still sending ... */
	else {
		return 0xFF;
	}
}

void nrf24_powerUpRx() {
	nrf24_csn_digitalWrite(LOW);
	spi_transfer(FLUSH_RX);
	nrf24_csn_digitalWrite(HIGH);

	nrf24_configRegister(STATUS, (1 << RX_DR) | (1 << TX_DS) | (1 << MAX_RT));

	nrf24_ce_digitalWrite(LOW);
	nrf24_configRegister(CONFIG,
			nrf24_CONFIG | ((1 << PWR_UP) | (1 << PRIM_RX)));
	nrf24_ce_digitalWrite(HIGH);
}

void nrf24_powerUpTx() {
	nrf24_configRegister(STATUS, (1 << RX_DR) | (1 << TX_DS) | (1 << MAX_RT));

	nrf24_configRegister(CONFIG,
			nrf24_CONFIG | ((1 << PWR_UP) | (0 << PRIM_RX)));
}

void nrf24_powerDown() {
	nrf24_ce_digitalWrite(LOW);
	nrf24_configRegister(CONFIG, nrf24_CONFIG);
}

/* software spi routine */
uint8_t spi_transfer(uint8_t tx) {
	uint8_t rx = 0;

	rx = nRF24_LL_RW(tx);

	return rx;
}

/* send and receive multiple bytes over SPI */
void nrf24_transferSync(uint8_t* dataout, uint8_t* datain, uint8_t len) {
	uint8_t i;

	for (i = 0; i < len; i++) {
		datain[i] = spi_transfer(dataout[i]);
	}

}

/* send multiple bytes over SPI */
void nrf24_transmitSync(uint8_t* dataout, uint8_t len) {
	uint8_t i;

	for (i = 0; i < len; i++) {
		spi_transfer(dataout[i]);
	}

}

/* Clocks only one byte into the given nrf24 register */
void nrf24_configRegister(uint8_t reg, uint8_t value) {
	nrf24_csn_digitalWrite(LOW);
	spi_transfer(W_REGISTER | (REGISTER_MASK & reg));
	spi_transfer(value);
	nrf24_csn_digitalWrite(HIGH);
}

/* Read single register from nrf24 */
void nrf24_readRegister(uint8_t reg, uint8_t* value, uint8_t len) {
	nrf24_csn_digitalWrite(LOW);
	spi_transfer(R_REGISTER | (REGISTER_MASK & reg));
	nrf24_transferSync(value, value, len);
	nrf24_csn_digitalWrite(HIGH);
}

/* Write to a single register of nrf24 */
void nrf24_writeRegister(uint8_t reg, uint8_t* value, uint8_t len) {
	nrf24_csn_digitalWrite(LOW);
	spi_transfer(W_REGISTER | (REGISTER_MASK & reg));
	nrf24_transmitSync(value, len);
	nrf24_csn_digitalWrite(HIGH);
}