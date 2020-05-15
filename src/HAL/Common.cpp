#include <Arduino.h>

namespace MaximWire {

#include "Common.h"

HAL_Common::HAL_Common(uint8_t pin)
    : _Pin(pin)
{  
}

// Dow-CRC using polynomial X^8 + X^5 + X^4 + X^0
// Tiny 2x16 entry CRC table created by Arjen Lentz
// See http://lentz.com.au/blog/calculating-crc-with-a-tiny-32-entry-lookup-table
static const uint8_t PROGMEM dscrc2x16_table[] = {
	0x00, 0x5E, 0xBC, 0xE2, 0x61, 0x3F, 0xDD, 0x83,
	0xC2, 0x9C, 0x7E, 0x20, 0xA3, 0xFD, 0x1F, 0x41,
	0x00, 0x9D, 0x23, 0xBE, 0x46, 0xDB, 0x65, 0xF8,
	0x8C, 0x11, 0xAF, 0x32, 0xCA, 0x57, 0xE9, 0x74
};

uint8_t Util::CRC8(const uint8_t* address, uint8_t length) {
	uint8_t crc = 0;

	while (length--) {
		crc = *address++ ^ crc;  // just re-using crc as intermediate
		crc = pgm_read_byte(dscrc2x16_table + (crc & 0x0F)) ^ pgm_read_byte(dscrc2x16_table + 16 + ((crc >> 4) & 0x0F));
	}

	return crc;
}

char Util::HexC(uint8_t d) {
	if (d < 10) {
		return '0' + d;
	}
	return 'A' + (d - 10);
}

String Util::Hex(uint8_t b) {
	String r;
	r.reserve(2);
	r += HexC(b >> 4);
	r += HexC(b & 15);
	return r;
}

} // MaximWire
