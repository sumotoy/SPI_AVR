#include "SPI_AVR.h"

SPI_AVR::SPI_AVR()
{

}

SPI_AVR::SPI_AVR(const uint8_t csPin,const uint8_t dcPin)
{
	postInstance(csPin,dcPin);
}

void SPI_AVR::postInstance(const uint8_t csPin,const uint8_t dcPin)
{
	_cs = csPin;
	_dc = dcPin;
}

bool SPI_AVR::begin(SPISettings settings,bool avoidInit)
{
	_spiSettings = settings;
	_initError = 0xFF;
	pinMode(_cs, OUTPUT);
	if (_dc != 255) {
		pinMode(_dc, OUTPUT);
		dcport = portOutputRegister(digitalPinToPort(_dc));
		dcpinmask = digitalPinToBitMask(_dc);
	}
	if (!avoidInit) SPI.begin();
	csport = portOutputRegister(digitalPinToPort(_cs));
	cspinmask = digitalPinToBitMask(_cs);
	disableCS();
	_changeMode(true);
	return 0xFF;
}

void SPI_AVR::enableCS(void)
{
	*csport &= ~cspinmask;//lo
}
		
void SPI_AVR::disableCS(void)
{
	*csport |= cspinmask;//hi
}

void SPI_AVR::setSpiSettings(SPISettings settings)
{
	_spiSettings = settings;
}

uint8_t SPI_AVR::getSPIbus(void)
{
	return 0;//ESP has currently only one SPI bus
}

void SPI_AVR::enableDataStream(void)
{
	_changeMode(true);
}
		
void SPI_AVR::enableCommandStream(void)
{
	_changeMode(false);
}

void SPI_AVR::_changeMode(bool dataMode)
{
	if (_dc == 255) return;
	if (dataMode){
		*dcport |= dcpinmask;//hi
	} else {
		*dcport &= ~dcpinmask;//lo
	}
}

void SPI_AVR::startTransaction(void)
{
	SPI.beginTransaction(_spiSettings);
	enableCS();
}

void SPI_AVR::endTransaction(void)
{
	SPI.endTransaction();
}

void SPI_AVR::writeByte_cont(uint8_t val,bool dataMode)
{
	_changeMode(dataMode);
	SPDR = val;
	//asm volatile("nop");
	while (!(SPSR & _BV(SPIF)));
}

void SPI_AVR::writeByte_last(uint8_t val,bool dataMode)
{
	writeByte_cont(val,dataMode);
	disableCS();
}

void SPI_AVR::writeWord_cont(uint16_t val,bool dataMode)
{
	_changeMode(dataMode);
	SPDR = highByte(val);
	while (!(SPSR & _BV(SPIF)));
	SPDR = lowByte(val);
	while (!(SPSR & _BV(SPIF)));
}

void SPI_AVR::writeWord_last(uint16_t val,bool dataMode)
{
	writeWord_cont(val,dataMode);
	disableCS();
}

uint8_t SPI_AVR::readByte_cont(bool dataMode)
{
	uint8_t r = 0;
	SPDR = 0x00;
	//asm volatile("nop");
	while(!(SPSR & _BV(SPIF)));
	r = SPDR;
	return r;
}

uint16_t SPI_AVR::readWord_cont(bool dataMode)
{
	union {
		uint16_t val= 0;
		struct {
            uint8_t lsb;
			uint8_t msb;
		};
	} out;
	out.msb = readByte_cont();
	out.lsb = readByte_cont();
	return out.val;
}

int SPI_AVR::getInterruptNumber(uint8_t pin)
{
	int intNum = digitalPinToInterrupt(pin);
	if (intNum != NOT_AN_INTERRUPT) {
		SPI.usingInterrupt(intNum);
		return intNum;
	}
	return 255;
}

void SPI_AVR::usingInterrupt(uint8_t n) 
{
	SPI.usingInterrupt(n); 
}
