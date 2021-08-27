#include "main.h"
#include "spi.h"
#include "gpio.h"
#include "usart.h"

#include "Nrf24.h"

Nrf24::Nrf24(
		SPI_HandleTypeDef *hspi,
		GPIO_TypeDef* CSN_Port,
		uint16_t CSN,
		GPIO_TypeDef* CE_Port,
		uint16_t CE,
		PowerLevel level,
		DataRate rate,
		uint8_t channel,
		PipeNumber pipe,
		bool dynamicPayloadEnabled,
		uint8_t payloadSize,
		AddrSize addrSize
	)
    :hspi(hspi), CSN_Port(CSN_Port), CSN(CSN), CE_Port(CE_Port), CE(CE), payloadSize(payloadSize), addrSize(addrSize)
{
	HAL_GPIO_WritePin(CE_Port, CE, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(CSN_Port, CSN, GPIO_PIN_SET);

	HAL_Delay(5); // Wait for radio power up
	setPowerLevel(level);
	setDataRate(rate);
	enableCrc();
	setCrcLength(crc1byte);
	setRetries(3750, 15);
	if (dynamicPayloadEnabled) {
		enableDynamicPayload();
	} else {
		disableDynamicPayload();
		setPayloadSizeForPipe(pipe, payloadSize);
	}
	setRFChannel(channel);
	enablePipe(pipe);
	enableAutoAckForPipe(pipe);
	setAddressSize(size3bytes);

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
	setRxAddressForPipe(pipe0, addr_p0_backup);
	cfg |= 1 << powerUp;
	cfg |= 1 << primRx;
	writeRegister(config, cfg);
	writeRegister(status, 1 << dataReady | 1 << dataSent | 1 << maxRetransmits);

	flushRx();
	flushTx();

	HAL_GPIO_WritePin(CE_Port, CE, GPIO_PIN_SET);
	HAL_Delay(1);
}

void Nrf24::txMode()
{
	HAL_GPIO_WritePin(CE_Port, CE, GPIO_PIN_RESET);
	uint8_t cfg = readRegister(config);
	cfg |= 1 << powerUp;
	cfg &= ~(1 << primRx);
	writeRegister(config, cfg);
	writeRegister(status, 1 << dataReady | 1 << dataSent | 1 << maxRetransmits);

	flushRx();
	flushTx();

	HAL_Delay(1);
}

void Nrf24::disableDynamicPayload() {
	writeRegister(dynamicPayload, 0);
}

void Nrf24::enableDynamicPayload() {
	uint8_t f = readRegister(feature);
	writeRegister(feature, f | 1 << enDynamicPayload);
	writeRegister(dynamicPayload, 0x3F);  // enable dynamic payload for all pipes
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
	if (rate == dataRate250kbps) {
		rf_setup |= 1 << dataRateLow;
	} else if (rate == dataRate2mbps) {
		rf_setup |= 1 << dataRateHigh;
	}
	writeRegister(rfSetup, rf_setup);
}

void Nrf24::enableCrc()
{
	uint8_t cfg = readRegister(config);
	cfg |= 1 << enCrc;
	writeRegister(config, cfg);
}

void Nrf24::disableCrc()
{
	uint8_t cfg = readRegister(config);
	cfg &= ~(1 << enCrc);
	writeRegister(config, cfg);
}

void Nrf24::setCrcLength(CrcLength length)
{
	uint8_t cfg = readRegister(config);
	if (length == crc2bytes) {
		cfg |= 1 << crco;
	} else {
		cfg &= ~(1 << crco);
	}
	writeRegister(config, cfg);
}

void Nrf24::setRetries(uint16_t timeout, uint8_t repeats)
{
	assert_param(timeout % 250 == 0);
	assert_param(timeout <= 3750);
	assert_param(repeats <= 15);
	writeRegister(setupRetries, (timeout / 250) << 4 | repeats);
}

void Nrf24::setRFChannel(uint8_t channel)
{
	assert_param(channel < 126);
	writeRegister(rfChannel, channel);
}

void Nrf24::setPayloadSizeForPipe(PipeNumber pipe, uint8_t size)
{
	assert_param(size > 0);
	assert_param(size <= 32);
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
	if (pipe == pipe0 || pipe == pipe1) {
		uint8_t address_rev[5];
		for (uint8_t i=0; i<addrSize; i++)
			address_rev[addrSize-1-i] = address[i];
		writeRegisters((Register) (rxAddrPipe0 + pipe), address_rev, addrSize);
	}
	else {
		writeRegister((Register) (rxAddrPipe0 + pipe), address[addrSize-1]);
	}
}

void Nrf24::setTxAddress(uint8_t* address)
{
	// TX address is storaged similar to RX pipe 0 - LSByte first
	uint8_t address_rev[5];

	readRegisters(rxAddrPipe0, address_rev, addrSize); // Backup P0 address
	for (uint8_t i=0; i<addrSize; i++)
		addr_p0_backup[addrSize-1-i] = address_rev[i]; //Reverse P0 address

	for (uint8_t i=0; i<addrSize; i++)
		address_rev[addrSize-1-i] = address[i];
	//make pipe 0 address backup;

	writeRegisters(rxAddrPipe0, address_rev, addrSize); // Pipe 0 must be same for auto ACk
	writeRegisters(txAddr, address_rev, addrSize);
}

void Nrf24::clearInterrupts()
{
	uint8_t st = readRegister(status);
	st |= 1 << dataReady | 1 << dataSent | 1 << maxRetransmits;
	writeRegister(status, st);
}

void Nrf24::enableRxDataReadyIrq()
{
	uint8_t cfg = readRegister(config);
	cfg &= ~(1 << dataReady);
	writeRegister(config, cfg);
}

void Nrf24::disableRxDataReadyIrq()
{
	uint8_t cfg = readRegister(config);
	cfg |= 1 << dataReady;
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
	writeRegisters((Register) writePayload, data, payloadSize);
}

bool Nrf24::waitTx()
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
	readRegisters((Register) readPayload, data, payloadSize);
	writeRegister(status, 1 << dataReady);
	if (readRegister(status) & 1 << dataSent) {
		writeRegister(status, 1 << dataSent);
	}
}

bool Nrf24::rxAvailable()
{
	uint8_t st = readRegister(status);

	if (st & 1 << dataReady) {
		st |= 1 << dataReady;
		writeRegister(status, st);
		return true;
	}
	return false;
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
