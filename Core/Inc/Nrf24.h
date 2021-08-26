#ifndef NRF24_H
#define NFR24_H
#include "Nrf24Defs.h"

class Nrf24 {
	private:		
		SPI_HandleTypeDef *hspi;
		uint8_t addr_p0_backup[Size_3bytes];

	public:
		Nrf24(SPI_HandleTypeDef *hspi, PowerLevel level, DataRate rate, uint8_t channel, PipeNumber pipe);

		uint8_t readRegister(Register reg);
		void readRegisters(Register reg, uint8_t* ret, uint8_t len);
		void writeRegister(Register reg, uint8_t val);
		void writeRegisters(Register reg, uint8_t* val, uint8_t len);
		

		void rxMode();
		void txMode();

		void setPowerLevel(PowerLevel level);
		void setDataRate(DataRate rate);
		void enableCrc();
		void disableCrc();
		void setCrcLength(CrcLength length);
		void setRetries(uint8_t timeout, uint8_t repeats);
		void setRFChannel(uint8_t channel);
		void setPayloadSizeForPipe(PipeNumber pipe, uint8_t size);
		void enablePipe(PipeNumber pipe);
		void disablePipe(PipeNumber pipe);
		void enableAutoAckForPipe(PipeNumber pipe);
		void disableAutoAckForPipe(PipeNumber pipe);
		void setRxAddressForPipe(PipeNumber pipe, uint8_t* address); // Remember to define RX address before TX
		void setTxAddress(uint8_t* address);
		void setAddressSize(AddrSize size);

		void clearInterrupts();
		void enableRxDataReadyIrq();
		void disableRxDataReadyIrq();
		void enableTxDataSentIrq();
		void disableTxDataSentIrq();
		void enableMaxRetransmitIrq();
		void disableMaxRetransmitIrq();

		uint8_t rxAvailable();
		void readRxPaylaod(uint8_t *data, uint8_t *size);
		void writeTxPayload(uint8_t * data, uint8_t size);
		uint8_t waitTx();

		void flushRx();
		void flushTx();
};

#endif
