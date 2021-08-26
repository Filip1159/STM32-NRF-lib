#pragma once

enum Register {
	config = 0,
	enableAutoAck = 1,
	enableRxAddr = 2,
	setupAddrWidth = 3,
	setupRetries = 4,
	rfChannel = 5,
	rfSetup = 6,
	status = 7,
	observeTx = 8,
	cd = 9,
	rxAddrPipe0 = 10,
	txAddr = 16,
	rxPayloadWidthPipe0 = 17,
	fifoStatus = 23,
	dynamicPayload = 0x1C,
	feature = 0x1D
};

enum Command {
	readReg = 0x00,
	writeReg = 0x20,
	readPayload = 0x61,
	writePayload = 0xA0,
	flushTxCmd = 0xE1,
	flushRxCmd = 0xE2,
	reuseTxPayload = 0xE3,
	activate = 0x50,
	readRxPayloadWidth = 0x60,
	writeAckPayload = 0xA8,
	writeTxPayloadNoAck = 0xB0,
	nop = 0xFF
};

enum Bit {
	dataReceived = 6,
	dataSent = 5,
	maxRetransmits = 4,
	enableCrcBit = 3,
	crco = 2,
	powerUp = 1,
	primRx = 0,
	enableAutoAckPipe5 = 5,
	enableAutoAckPipe4 = 4,
	enableAutoAckPipe3 = 3,
	enableAutoAckPipe2 = 2,
	enableAutoAckPipe1 = 1,
	enableAutoAckPipe0 = 0,
	enableRxPipe5 = 5,
	enableRxPipe4 = 4,
	enableRxPipe3 = 3,
	enableRxPipe2 = 2,
	enableRxPipe1 = 1,
	enableRxPipe0 = 0,
	aw = 0,
	ard = 4,
	arc = 0,
	pllLock = 4,
	rfDataRateHigh = 3,
	rfDataRateLow = 5,
	rfPwr = 1,
	lnaHcurr = 0,
	rxPNo = 1,
	txFull = 0,
	plosCnt = 4,
	arcCnt = 0,
	txReuse = 6,
	fifoFull = 5,
	txEmpty = 4,
	rxFull = 1,
	rxEmpty = 0,
	rpd = 9,
	enDynamicPayload = 2
};

#define NRF24_PAYLOAD_SIZE 1

#define NRF24_ADDR_SIZE 3

enum PipeNumber {
	Pipe0 = 0,
	Pipe1 = 1,
	Pipe2 = 2,
	Pipe3 = 3,
	Pipe4 = 4,
	Pipe5 = 5
};

enum AddrSize {
	Size_3bytes = 3,
	Size_4bytes = 4,
	Size_5bytes = 5
};

enum CrcLength {
	Crc_1byte = 0,
	Crc_2bytes = 1
};

enum DataRate {
	DataRate_250kbps = 2,
	DataRate_1mbps = 0,
	DataRate_2mbps = 1
};

enum PowerLevel {
	Power_m18 = 0,
	Power_m12 = 1,
	Power_m6 = 2,
	Power_0 = 3
};
