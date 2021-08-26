#include "main.h"
#include "spi.h"
#include "gpio.h"
#include "usart.h"

#include "Nrf24.h"

#define NRF24_CSN_HIGH		HAL_GPIO_WritePin(GPIOA, CSN_Pin, GPIO_PIN_SET)
#define NRF24_CSN_LOW		HAL_GPIO_WritePin(GPIOA, CSN_Pin, GPIO_PIN_RESET)

#define NRF24_CE_HIGH		HAL_GPIO_WritePin(GPIOA, CE_Pin, GPIO_PIN_SET)
#define NRF24_CE_LOW		HAL_GPIO_WritePin(GPIOA, CE_Pin, GPIO_PIN_RESET)

Nrf24::Nrf24(SPI_HandleTypeDef *hspi, PowerLevel level, DataRate rate, uint8_t channel, PipeNumber pipe)
    :hspi(hspi)
{
	NRF24_CE_LOW;
	NRF24_CSN_HIGH;

	delay_ms(5); // Wait for radio power up

	setPowerLevel(level); // Radio power
	setDataRate(rate); // Data Rate
	enableCrc(); // Enable CRC
	setCrcLength(Crc_1byte); // CRC Length 1 byte
	setRetries(0x0F, 0x0F); // 3750us, 15 times
	writeRegister(NRF24_DYNPD, 0); // Disable dynamic payloads for all pipes
	setRFChannel(channel); // Set RF channel for transmission
	setPayloadSizeForPipe(pipe, NRF24_PAYLOAD_SIZE); // Set 1 byte payload for pipe 0
	enablePipe(pipe); // Enable pipe 0
	enableAutoAckForPipe(pipe); // Enable auto ACK for pipe 0
	setAddressSize(Size_3bytes); // Set address size

	delay_ms(20);
	disableRxDataReadyIrq();
	disableTxDataSentIrq();
	disableMaxRetransmitIrq();

	delay_ms(20);
	clearInterrupts();
}

void Nrf24::delay_ms(uint8_t Time)
{
	HAL_Delay(Time);
}

void Nrf24::sendSpi(uint8_t *Data, uint8_t Length)
{
	if(
			HAL_SPI_Transmit(hspi, Data, Length, 1000)
		!= HAL_OK) {
		HAL_UART_Transmit(&huart2, (uint8_t*)"ERR_SEND ", 9, 1000);
		HAL_GPIO_WritePin(ERR_GPIO_Port, ERR_Pin, GPIO_PIN_SET);
	} else HAL_UART_Transmit(&huart2, (uint8_t*)"OK_SEND ", 8, 1000);
}

void Nrf24::readSpi(uint8_t *Data, uint8_t Length)
{
	if(
			HAL_SPI_Receive(hspi, Data, Length, 1000)
	    != HAL_OK) {
		HAL_UART_Transmit(&huart2, (uint8_t*)"ERR_READ ", 9, 1000);
		HAL_GPIO_WritePin(ERR_GPIO_Port, ERR_Pin, GPIO_PIN_SET);
	} else HAL_UART_Transmit(&huart2, (uint8_t*)"OK_READ ", 8, 1000);
}

uint8_t Nrf24::readRegister(uint8_t reg)
{
	uint8_t result;

	reg = NRF24_CMD_R_REGISTER | reg;

	NRF24_CSN_LOW;
	sendSpi(&reg, 1);
	readSpi(&result, 1);
	NRF24_CSN_HIGH;

	return result;
}

void Nrf24::readRegisters(uint8_t reg, uint8_t* ret, uint8_t len)
{
	reg = NRF24_CMD_R_REGISTER | reg;

	NRF24_CSN_LOW;

	sendSpi(&reg, 1);
	readSpi(ret, len);

	NRF24_CSN_HIGH;
}

void Nrf24::writeRegister(uint8_t reg, uint8_t val)
{
	uint8_t tmp[2];

	tmp[0] = NRF24_CMD_W_REGISTER | reg;
	tmp[1] = val;

	NRF24_CSN_LOW;

	sendSpi(tmp, 2);

	NRF24_CSN_HIGH;
}

void Nrf24::writeRegisters(uint8_t reg, uint8_t* val, uint8_t len)
{
	reg = NRF24_CMD_W_REGISTER | reg;

	NRF24_CSN_LOW;

	sendSpi(&reg, 1);
	sendSpi(val, len);

	NRF24_CSN_HIGH;
}

uint8_t Nrf24::readConfig(void)
{
	return readRegister(NRF24_CONFIG);
}

void Nrf24::writeConfig(uint8_t conf)
{
	writeRegister(NRF24_CONFIG, conf);
}

uint8_t Nrf24::readStatus(void)
{
	return readRegister(NRF24_STATUS);
}

void Nrf24::writeStatus(uint8_t st)
{
	writeRegister(NRF24_STATUS, st);
}

void Nrf24::rxMode(void)
{
	uint8_t config = readConfig();
	// Restore pipe 0 adress after comeback from TX mode
	setRxAddressForPipe(Pipe0, addr_p0_backup);
	// PWR_UP bit set
	config |= (1<<NRF24_PWR_UP);
	// PRIM_RX bit set
	config |= (1<<NRF24_PRIM_RX);
	writeConfig(config);
	// Reset status
	writeStatus((1<<NRF24_RX_DR)|(1<<NRF24_TX_DS)|(1<<NRF24_MAX_RT));

	flushRx();
	flushTx();

	NRF24_CE_HIGH;
	delay_ms(1);
}

void Nrf24::txMode(void)
{
	NRF24_CE_LOW;

	uint8_t config = readConfig();
	// PWR_UP bit set
	config |= (1<<NRF24_PWR_UP);
	// PRIM_RX bit low
	config &= ~(1<<NRF24_PRIM_RX);
	writeConfig(config);
	// Reset status
	writeStatus((1<<NRF24_RX_DR)|(1<<NRF24_TX_DS)|(1<<NRF24_MAX_RT));

	flushRx();
	flushTx();

	delay_ms(1);
}

void Nrf24::setPowerLevel(PowerLevel level)
{
	uint8_t rf_setup = readRegister(NRF24_RF_SETUP);
	rf_setup &= 0xF8; // Clear PWR bits
	rf_setup |= (level<<1);
	writeRegister(NRF24_RF_SETUP, rf_setup);
}

void Nrf24::setDataRate(DataRate rate)
{
	uint8_t rf_setup = readRegister(NRF24_RF_SETUP);
	rf_setup &= 0xD7; // Clear DR bits (1MBPS)
	if(rate == DataRate_250kbps)
		rf_setup |= (1<<NRF24_RF_DR_LOW);
	else if(rate == DataRate_2mbps)
		rf_setup |= (1<<NRF24_RF_DR_HIGH);
	writeRegister(NRF24_RF_SETUP, rf_setup);
}

void Nrf24::enableCrc()
{
	uint8_t config = readConfig();
	config |= (1<<NRF24_EN_CRC);
	writeConfig(config);
}

void Nrf24::disableCrc()
{
	uint8_t config = readConfig();
	config &= ~(1<<NRF24_EN_CRC);
	writeConfig(config);
}

void Nrf24::setCrcLength(CrcLength length)
{
	uint8_t config = readConfig();
	if(length == Crc_2bytes)
		config |= (1<<NRF24_CRCO);
	else
		config &= ~(1<<NRF24_CRCO);
	writeConfig(config);
}

void Nrf24::setRetries(uint8_t timeout, uint8_t repeats)
{
	// timeout * 250us, repeats
	writeRegister(NRF24_SETUP_RETR, (((timeout & 0x0F)<<NRF24_ARD) | ((repeats & 0x0F)<<NRF24_ARC)));
}

void Nrf24::setRFChannel(uint8_t channel)
{
	writeRegister(NRF24_RF_CH, (channel & 0x7F));
}

void Nrf24::setPayloadSizeForPipe(PipeNumber pipe, uint8_t size)
{
	writeRegister(NRF24_RX_PW_P0 + pipe , (size & 0x3F));
}

void Nrf24::enablePipe(PipeNumber pipe)
{
	uint8_t enable_pipe = readRegister(NRF24_EN_RXADDR);
	enable_pipe |= (1<<pipe);
	writeRegister(NRF24_EN_RXADDR, enable_pipe);
}

void Nrf24::disablePipe(PipeNumber pipe)
{
	uint8_t enable_pipe = readRegister(NRF24_EN_RXADDR);
	enable_pipe &= ~(1<<pipe);
	writeRegister(NRF24_EN_RXADDR, enable_pipe);
}

void Nrf24::enableAutoAckForPipe(PipeNumber pipe)
{
	uint8_t enaa = readRegister(NRF24_EN_AA);
	enaa |= (1<<pipe);
	writeRegister(NRF24_EN_AA, enaa);
}

void Nrf24::disableAutoAckForPipe(PipeNumber pipe)
{
	uint8_t enaa = readRegister(NRF24_EN_AA);
	enaa &= ~(1<<pipe);
	writeRegister(NRF24_EN_AA, enaa);
}

void Nrf24::setAddressSize(AddrSize size)
{
	writeRegister(NRF24_SETUP_AW, ((size-2) & 0x03));
}

void Nrf24::setRxAddressForPipe(PipeNumber pipe, uint8_t* address)
{
	// pipe 0 and pipe 1 are fully 40-bits storaged
	// pipe 2-5 is storaged only with last byte. Rest are as same as pipe 1
	// pipe 0 and 1 are LSByte first so they are needed to reverse address
	if((pipe == Pipe0) || (pipe == Pipe1))
	{
		uint8_t i;
		uint8_t address_rev[NRF24_ADDR_SIZE];
		for(i = 0; i<NRF24_ADDR_SIZE; i++)
			address_rev[NRF24_ADDR_SIZE - 1 - i] = address[i];
		writeRegisters(NRF24_RX_ADDR_P0 + pipe, address_rev, NRF24_ADDR_SIZE);
	}
	else
		writeRegister(NRF24_RX_ADDR_P0 + pipe, address[NRF24_ADDR_SIZE-1]);
}

void Nrf24::setTxAddress(uint8_t* address)
{
	// TX address is storaged similar to RX pipe 0 - LSByte first
	uint8_t i;
	uint8_t address_rev[NRF24_ADDR_SIZE];

	readRegisters(NRF24_RX_ADDR_P0, address_rev, NRF24_ADDR_SIZE); // Backup P0 address
	for(i = 0; i<NRF24_ADDR_SIZE; i++)
		addr_p0_backup[NRF24_ADDR_SIZE - 1 - i] = address_rev[i]; //Reverse P0 address

	for(i = 0; i<NRF24_ADDR_SIZE; i++)
		address_rev[NRF24_ADDR_SIZE - 1 - i] = address[i];
	//make pipe 0 address backup;

	writeRegisters(NRF24_RX_ADDR_P0, address_rev, NRF24_ADDR_SIZE); // Pipe 0 must be same for auto ACk
	writeRegisters(NRF24_TX_ADDR, address_rev, NRF24_ADDR_SIZE);
}

void Nrf24::clearInterrupts()
{
	uint8_t status = readStatus();
	status |= (7<<4); // Clear bits 4, 5, 6.
	writeStatus(status);
}

void Nrf24::enableRxDataReadyIrq()
{
	uint8_t config = readConfig();
	config &= ~(1<<NRF24_RX_DR);
	writeConfig(config);
}

void Nrf24::disableRxDataReadyIrq()
{
	uint8_t config = readConfig();
	config |= (1<<NRF24_RX_DR);
	writeConfig(config);
}

void Nrf24::enableTxDataSentIrq()
{
	uint8_t config = readConfig();
	config &= ~(1<<NRF24_TX_DS);
	writeConfig(config);
}

void Nrf24::disableTxDataSentIrq()
{
	uint8_t config = readConfig();
	config |= (1<<NRF24_TX_DS);
	writeConfig(config);
}

void Nrf24::enableMaxRetransmitIrq()
{
	uint8_t config = readConfig();
	config &= ~(1<<NRF24_MAX_RT);
	writeConfig(config);
}

void Nrf24::disableMaxRetransmitIrq()
{
	uint8_t config = readConfig();
	config |= (1<<NRF24_MAX_RT);
	writeConfig(config);
}

void Nrf24::writeTxPayload(uint8_t * data, uint8_t size)
{
	writeRegisters(NRF24_CMD_W_TX_PAYLOAD, data, NRF24_PAYLOAD_SIZE);
}

uint8_t Nrf24::waitTx()
{
	uint8_t status;
	NRF24_CE_HIGH;
	delay_ms(1);
	NRF24_CE_LOW;
	do
	{
		delay_ms(1);
		status = readStatus();
	}while(!((status & (1<<NRF24_MAX_RT)) || (status & (1<<NRF24_TX_DS))));
	if(status&(1<<NRF24_TX_DS)) return 0; else return 1;
}

void Nrf24::readRxPaylaod(uint8_t *data, uint8_t *size)
{
	readRegisters(NRF24_CMD_R_RX_PAYLOAD, data, NRF24_PAYLOAD_SIZE);

	writeRegister(NRF24_STATUS, (1<NRF24_RX_DR));
	if(readStatus() & (1<<NRF24_TX_DS))
		writeRegister(NRF24_STATUS, (1<<NRF24_TX_DS));
}

uint8_t Nrf24::rxAvailable()
{
	uint8_t status = readStatus();

	// RX FIFO Interrupt
	if ((status & (1 << 6)))
	{
		status |= (1<<6); // Interrupt flag clear
		writeStatus(status);
		return 1;
	}
	return 0;
}

void Nrf24::flushRx()
{
	uint8_t command = NRF24_CMD_FLUSH_RX;

	NRF24_CSN_LOW;
	sendSpi(&command, 1);
	NRF24_CSN_HIGH;
}

void Nrf24::flushTx()
{
	uint8_t command = NRF24_CMD_FLUSH_TX;

	NRF24_CSN_LOW;
	sendSpi(&command, 1);
	NRF24_CSN_HIGH;
}
