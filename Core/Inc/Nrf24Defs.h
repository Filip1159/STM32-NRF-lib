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
	dataReady = 6,
	dataSent = 5,
	maxRetransmits = 4,
	enCrc = 3,
	crco = 2,
	powerUp = 1,
	primRx = 0,
	enableAutoAckPipe0 = 0,
	enableRxPipe0 = 0,
	aw = 0,
	pllLock = 4,
	dataRateHigh = 3,
	dataRateLow = 5,
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

enum PipeNumber {
	pipe0 = 0,
	pipe1 = 1,
	pipe2 = 2,
	pipe3 = 3,
	pipe4 = 4,
	pipe5 = 5
};

enum AddrSize {
	size3bytes = 3,
	size4bytes = 4,
	size5bytes = 5
};

enum CrcLength {
	crc1byte = 0,
	crc2bytes = 1
};

enum DataRate {
	dataRate250kbps = 2,
	dataRate1mbps = 0,
	dataRate2mbps = 1
};

enum PowerLevel {
	power_m18 = 0,
	power_m12 = 1,
	power_m6 = 2,
	power_0 = 3
};
