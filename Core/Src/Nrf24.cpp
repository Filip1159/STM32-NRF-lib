#include "main.h"
#include "spi.h"
#include "gpio.h"
#include "usart.h"

#include "Nrf24.h"

Nrf24::Nrf24(SPI_HandleTypeDef *hspi, GPIO_TypeDef* CSN_Port, uint16_t CSN, GPIO_TypeDef* CE_Port, uint16_t CE, PowerLevel level, DataRate rate, uint8_t channel, PipeNumber pipe)
    :hspi(hspi), CSN_Port(CSN_Port), CSN(CSN), CE_Port(CE_Port), CE(CE)
{
	HAL_GPIO_WritePin(CE_Port, CE, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(CSN_Port, CSN, GPIO_PIN_SET);

	HAL_Delay(5); // Wait for radio power up
	setPowerLevel(level);
	setDataRate(rate);
	enableCrc();
	setCrcLength(Crc_1byte);
	setRetries(0x0F, 0x0F); // 3750us, 15 times
	writeRegister(dynamicPayload, 0); // Disable dynamic payloads for all pipes
	setRFChannel(channel);
	setPayloadSizeForPipe(pipe, NRF24_PAYLOAD_SIZE);
	enablePipe(pipe);
	enableAutoAckForPipe(pipe);
	setAddressSize(Size_3bytes);

	HAL_Delay(20);
	disableRxDataReadyIrq();
	disableTxDataSentIrq();
	disableMaxRetransmitIrq();

	HAL_Delay(20);
	clearInterrupts();
}

uint8_t Nrf24::readRegister(Register reg)
{
	uint8_t result;

	HAL_GPIO_WritePin(CSN_Port, CSN, GPIO_PIN_RESET);
	HAL_SPI_Transmit(hspi, (uint8_t*) &reg, 1, 1000);
	HAL_SPI_Receive(hspi, &result, 1, 1000);
	HAL_GPIO_WritePin(CSN_Port, CSN, GPIO_PIN_SET);

	return result;
}

void Nrf24::readRegisters(Register reg, uint8_t* ret, uint8_t len)
{
	HAL_GPIO_WritePin(CSN_Port, CSN, GPIO_PIN_RESET);
	HAL_SPI_Transmit(hspi, (uint8_t*) &reg, 1, 1000);
	HAL_SPI_Receive(hspi, ret, len, 1000);
	HAL_GPIO_WritePin(CSN_Port, CSN, GPIO_PIN_SET);
}

void Nrf24::writeRegister(Register reg, uint8_t val)
{
	uint8_t tmp[2];

	tmp[0] = writeReg | reg;
	tmp[1] = val;

	HAL_GPIO_WritePin(CSN_Port, CSN, GPIO_PIN_RESET);
	HAL_SPI_Transmit(hspi, tmp, 2, 1000);
	HAL_GPIO_WritePin(CSN_Port, CSN, GPIO_PIN_SET);
}

void Nrf24::writeRegisters(Register reg, uint8_t* val, uint8_t len)
{
	reg = (Register) (reg | writeReg);

	HAL_GPIO_WritePin(CSN_Port, CSN, GPIO_PIN_RESET);
	HAL_SPI_Transmit(hspi, (uint8_t*) &reg, 1, 1000);
	HAL_SPI_Transmit(hspi, val, len, 1000);
	HAL_GPIO_WritePin(CSN_Port, CSN, GPIO_PIN_SET);
}

void Nrf24::rxMode()
{
	uint8_t cfg = readRegister(config);
	// Restore pipe 0 adress after comeback from TX mode
	setRxAddressForPipe(Pipe0, addr_p0_backup);
	// PWR_UP bit set
	cfg |= 1 << powerUp;
	// PRIM_RX bit set
	cfg |= 1 << primRx;
	writeRegister(config, cfg);
	// Reset status
	writeRegister(status, 1 << dataReceived | 1 << dataSent | 1 << maxRetransmits);

	flushRx();
	flushTx();

	HAL_GPIO_WritePin(CE_Port, CE, GPIO_PIN_SET);
	HAL_Delay(1);
}

void Nrf24::txMode(void)
{
	HAL_GPIO_WritePin(CE_Port, CE, GPIO_PIN_RESET);
	uint8_t cfg = readRegister(config);
	// PWR_UP bit set
	cfg |= 1 << powerUp;
	// PRIM_RX bit low
	cfg &= ~(1 << primRx);
	writeRegister(config, cfg);
	// Reset status
	writeRegister(status, 1 << dataReceived | 1 << dataSent | 1 << maxRetransmits);

	flushRx();
	flushTx();

	HAL_Delay(1);
}

void Nrf24::setPowerLevel(PowerLevel level)
{
	uint8_t rf_setup = readRegister(rfSetup);
	rf_setup &= 0xF8; // Clear PWR bits
	rf_setup |= level << 1;
	writeRegister(rfSetup, rf_setup);
}

void Nrf24::setDataRate(DataRate rate)
{
	uint8_t rf_setup = readRegister(rfSetup);
	rf_setup &= 0xD7; // Clear DR bits (1MBPS)
	if(rate == DataRate_250kbps)
		rf_setup |= 1 << rfDataRateLow;
	else if(rate == DataRate_2mbps)
		rf_setup |= 1 << rfDataRateHigh;
	writeRegister(rfSetup, rf_setup);
}

void Nrf24::enableCrc()
{
	uint8_t cfg = readRegister(config);
	cfg |= 1 << enableCrcBit;
	writeRegister(config, cfg);
}

void Nrf24::disableCrc()
{
	uint8_t cfg = readRegister(config);
	cfg &= ~(1 << enableCrcBit);
	writeRegister(config, cfg);
}

void Nrf24::setCrcLength(CrcLength length)
{
	uint8_t cfg = readRegister(config);
	if(length == Crc_2bytes)
		cfg |= 1 << crco;
	else
		cfg &= ~(1 << crco);
	writeRegister(config, cfg);
}

void Nrf24::setRetries(uint8_t timeout, uint8_t repeats)
{
	// timeout * 250us, repeats
	writeRegister(setupRetries, ((timeout & 0x0F) << ard) | ((repeats & 0x0F) << arc));
}

void Nrf24::setRFChannel(uint8_t channel)
{
	writeRegister(rfChannel, channel & 0x7F);
}

void Nrf24::setPayloadSizeForPipe(PipeNumber pipe, uint8_t size)
{
	writeRegister((Register) (rxPayloadWidthPipe0 + pipe) , size & 0x3F);
}

void Nrf24::enablePipe(PipeNumber pipe)
{
	uint8_t enable_pipe = readRegister(enableRxAddr);
	enable_pipe |= 1 << pipe;
	writeRegister(enableRxAddr, enable_pipe);
}

void Nrf24::disablePipe(PipeNumber pipe)
{
	uint8_t enable_pipe = readRegister(enableRxAddr);
	enable_pipe &= ~(1 << pipe);
	writeRegister(enableRxAddr, enable_pipe);
}

void Nrf24::enableAutoAckForPipe(PipeNumber pipe)
{
	uint8_t enaa = readRegister(enableAutoAck);
	enaa |= 1 << pipe;
	writeRegister(enableAutoAck, enaa);
}

void Nrf24::disableAutoAckForPipe(PipeNumber pipe)
{
	uint8_t enaa = readRegister(enableAutoAck);
	enaa &= ~(1 << pipe);
	writeRegister(enableAutoAck, enaa);
}

void Nrf24::setAddressSize(AddrSize size)
{
	writeRegister(setupAddrWidth, size - 2);
}

void Nrf24::setRxAddressForPipe(PipeNumber pipe, uint8_t* address)
{
	// pipe 0 and pipe 1 are fully 40-bits storaged
	// pipe 2-5 is storaged only with last byte. Rest are as same as pipe 1
	// pipe 0 and 1 are LSByte first so they are needed to reverse address
	if(pipe == Pipe0 || pipe == Pipe1)
	{
		uint8_t i;
		uint8_t address_rev[NRF24_ADDR_SIZE];
		for(i=0; i<NRF24_ADDR_SIZE; i++)
			address_rev[NRF24_ADDR_SIZE - 1 - i] = address[i];
		writeRegisters((Register) (rxAddrPipe0 + pipe), address_rev, NRF24_ADDR_SIZE);
	}
	else
		writeRegister((Register) (rxAddrPipe0 + pipe), address[NRF24_ADDR_SIZE-1]);
}

void Nrf24::setTxAddress(uint8_t* address)
{
	// TX address is storaged similar to RX pipe 0 - LSByte first
	uint8_t i;
	uint8_t address_rev[NRF24_ADDR_SIZE];

	readRegisters(rxAddrPipe0, address_rev, NRF24_ADDR_SIZE); // Backup P0 address
	for(i = 0; i<NRF24_ADDR_SIZE; i++)
		addr_p0_backup[NRF24_ADDR_SIZE - 1 - i] = address_rev[i]; //Reverse P0 address

	for(i=0; i<NRF24_ADDR_SIZE; i++)
		address_rev[NRF24_ADDR_SIZE - 1 - i] = address[i];
	//make pipe 0 address backup;

	writeRegisters(rxAddrPipe0, address_rev, NRF24_ADDR_SIZE); // Pipe 0 must be same for auto ACk
	writeRegisters(txAddr, address_rev, NRF24_ADDR_SIZE);
}

void Nrf24::clearInterrupts()
{
	uint8_t st = readRegister(status);
	st |= 7 << 4; // Clear bits 4, 5, 6.
	writeRegister(status, st);
}

void Nrf24::enableRxDataReadyIrq()
{
	uint8_t cfg = readRegister(config);
	cfg &= ~(1 << dataReceived);
	writeRegister(config, cfg);
}

void Nrf24::disableRxDataReadyIrq()
{
	uint8_t cfg = readRegister(config);
	cfg |= 1 << dataReceived;
	writeRegister(config, cfg);
}

void Nrf24::enableTxDataSentIrq()
{
	uint8_t cfg = readRegister(config);
	cfg &= ~(1 << dataSent);
	writeRegister(config, cfg);
}

void Nrf24::disableTxDataSentIrq()
{
	uint8_t cfg = readRegister(config);
	cfg |= 1 << dataSent;
	writeRegister(config, cfg);
}

void Nrf24::enableMaxRetransmitIrq()
{
	uint8_t cfg = readRegister(config);
	cfg &= ~(1 << maxRetransmits);
	writeRegister(config, cfg);
}

void Nrf24::disableMaxRetransmitIrq()
{
	uint8_t cfg = readRegister(config);
	cfg |= 1 << maxRetransmits;
	writeRegister(config, cfg);
}

void Nrf24::writeTxPayload(uint8_t * data, uint8_t size)
{
	writeRegisters((Register) writePayload, data, NRF24_PAYLOAD_SIZE);
}

uint8_t Nrf24::waitTx()
{
	uint8_t st;
	HAL_GPIO_WritePin(CE_Port, CE, GPIO_PIN_SET);
	HAL_Delay(1);
	HAL_GPIO_WritePin(CE_Port, CE, GPIO_PIN_RESET);
	do {
		HAL_Delay(1);
		st = readRegister(status);
	} while(!(st & 1 << maxRetransmits || st & 1 << dataSent));
	if (st & 1 << dataSent) return 0;
	else return 1;
}

void Nrf24::readRxPaylaod(uint8_t *data, uint8_t *size)
{
	readRegisters((Register) readPayload, data, NRF24_PAYLOAD_SIZE);
	writeRegister(status, 1 << dataReceived);
	if (readRegister(status) & 1 << dataSent)
		writeRegister(status, 1 << dataSent);
}

uint8_t Nrf24::rxAvailable()
{
	uint8_t st = readRegister(status);

	// RX FIFO Interrupt
	if (st & 1 << 6) {
		st |= 1 << 6; // Interrupt flag clear
		writeRegister(status, st);
		return 1;
	}
	return 0;
}

void Nrf24::flushRx()
{
	Command cmd = flushRxCmd;

	HAL_GPIO_WritePin(CSN_Port, CSN, GPIO_PIN_RESET);
	HAL_SPI_Transmit(hspi, (uint8_t*) &cmd, 1, 1000);
	HAL_GPIO_WritePin(CSN_Port, CSN, GPIO_PIN_SET);
}

void Nrf24::flushTx()
{
	Command cmd = flushTxCmd;

	HAL_GPIO_WritePin(CSN_Port, CSN, GPIO_PIN_RESET);
	HAL_SPI_Transmit(hspi, (uint8_t*) &cmd, 1, 1000);
	HAL_GPIO_WritePin(CSN_Port, CSN, GPIO_PIN_SET);
}
