#ifndef INC_NRF24_NRF24_DEFS_H_
#define INC_NRF24_NRF24_DEFS_H_

//
// Registers
//

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
	rxAddrPipe1 = 11,
	rxAddrPipe2 = 12,
	rxAddrPipe3 = 13,
	rxAddrPipe4 = 14,
	rxAddrPipe5 = 15,
	txAddr = 16,
	rxPayloadWidthPipe0 = 17,
	rxPayloadWidthPipe1 = 18,
	rxPayloadWidthPipe2 = 19,
	rxPayloadWidthPipe3 = 20,
	rxPayloadWidthPipe4 = 21,
	rxPayloadWidthPipe5 = 22,
	fifoStatus = 23,
	dynamicPayload = 0x1C,
	feature = 0x1D
};
#define CONFIG		0x00
#define EN_AA		0x01
#define EN_RXADDR	0x02
#define SETUP_AW	0x03
#define SETUP_RETR	0x04
#define RF_CH		0x05
#define RF_SETUP	0x06
#define STATUS		0x07
#define OBSERVE_TX	0x08
#define CD			0x09
#define RX_ADDR_P0	0x0A
#define RX_ADDR_P1	0x0B
#define RX_ADDR_P2	0x0C
#define RX_ADDR_P3	0x0D
#define RX_ADDR_P4	0x0E
#define RX_ADDR_P5	0x0F
#define TX_ADDR		0x10
#define RX_PW_P0	0x11
#define RX_PW_P1	0x12
#define RX_PW_P2	0x13
#define RX_PW_P3	0x14
#define RX_PW_P4	0x15
#define RX_PW_P5	0x16
#define FIFO_STATUS	0x17
#define DYNPD		0x1C
#define FEATURE		0x1D

//
// Commands
//
#define CMD_R_REGISTER			0x00
#define CMD_W_REGISTER			0x20
#define CMD_R_RX_PAYLOAD		0x61
#define CMD_W_TX_PAYLOAD		0xA0
#define CMD_FLUSH_TX			0xE1
#define CMD_FLUSH_RX			0xE2
#define CMD_REUSE_TX_PL			0xE3
#define CMD_ACTIVATE			0x50
#define CMD_R_RX_PL_WID			0x60
#define CMD_W_ACK_PAYLOAD		0xA8
#define CMD_W_TX_PAYLOAD_NOACK	0xB0
#define CMD_NOP					0xFF

//
// Bit Mnemonics
//
#define NRF24_MASK_RX_DR  6
#define NRF24_MASK_TX_DS  5
#define NRF24_MASK_MAX_RT 4
#define NRF24_EN_CRC      3
#define NRF24_CRCO        2
#define NRF24_PWR_UP      1
#define NRF24_PRIM_RX     0
#define NRF24_ENAA_P5     5
#define NRF24_ENAA_P4     4
#define NRF24_ENAA_P3     3
#define NRF24_ENAA_P2     2
#define NRF24_ENAA_P1     1
#define NRF24_ENAA_P0     0
#define NRF24_ERX_P5      5
#define NRF24_ERX_P4      4
#define NRF24_ERX_P3      3
#define NRF24_ERX_P2      2
#define NRF24_ERX_P1      1
#define NRF24_ERX_P0      0
#define NRF24_AW          0
#define NRF24_ARD         4
#define NRF24_ARC         0
#define NRF24_PLL_LOCK    4
#define NRF24_RF_DR_HIGH  3
#define NRF24_RF_DR_LOW	  5
#define NRF24_RF_PWR      1
#define NRF24_LNA_HCURR   0
#define NRF24_RX_DR       6
#define NRF24_TX_DS       5
#define NRF24_MAX_RT      4
#define NRF24_RX_P_NO     1
#define NRF24_TX_FULL     0
#define NRF24_PLOS_CNT    4
#define NRF24_ARC_CNT     0
#define NRF24_TX_REUSE    6
#define NRF24_FIFO_FULL   5
#define NRF24_TX_EMPTY    4
#define NRF24_RX_FULL     1
#define NRF24_RX_EMPTY    0
#define NRF24_RPD         0x09
#define NRF24_EN_DPL      2

#define NRF24_PAYLOAD_SIZE 1

#define NRF24_ADDR_SIZE 3

enum PipeNumber {Pipe0 = 0, Pipe1 = 1, Pipe2 = 2, Pipe3 = 3, Pipe4 = 4, Pipe5 = 5};

enum AddrSize {Size_3bytes = 3, Size_4bytes = 4, Size_5bytes = 5};

enum CrcLength {Crc_1byte = 0, Crc_2bytes = 1};

enum DataRate {DataRate_250kbps = 2, DataRate_1mbps = 0, DataRate_2mbps = 1};

enum PowerLevel {Power_m18 = 0, Power_m12 = 1, Power_m6 = 2, Power_0 = 3};

#endif /* INC_NRF24_NRF24_DEFS_H_ */
