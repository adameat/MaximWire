#pragma once

#include <Arduino.h>

namespace MaximWire {

// HAL platform-specific implementations
#include "HAL/NRF52840.h"

class Bus;
class Discovery;
class Device;

class Address {
friend class Discovery;
public:
    static constexpr int ADDRESS_SIZE = 8;

    // public interface
    Address() = default;

    Address(const String& address) {
        uint8_t pos = 0;
        for (char c : address) {
            uint8_t v = 0;
            if (c >= '0' && c <= '9') {
                v = c - '0';
            } else if (c >= 'A' && c <= 'F') {
                v = c - 'A' + 10;
            }
            if (pos % 2 == 0) {
                _ID[pos / 2] = v << 4;
            } else {
                _ID[pos / 2] |= v;
            }
            ++pos;
            if (pos > ADDRESS_SIZE * 2) {
                break;
            }
        }
    }

    Address(const uint8_t (&address)[ADDRESS_SIZE]) {
        for (int i = 0; i < ADDRESS_SIZE; ++i) {
            _ID[i] = address[i];
        }
    }

    String ToString() const {
        String result;
        result.reserve(ADDRESS_SIZE * 2);
        for (uint8_t b : _ID) {
            result += Util::Hex(b);
        }
        return result;
    }

    bool IsValid() const {
        return _ID[0] != 0 && Util::CRC8(_ID, ADDRESS_SIZE - 1) == _ID[ADDRESS_SIZE - 1];
    }

    uint8_t GetModelCode() const {
        return _ID[0];
    }

    void Reset() {
        for (uint8_t& n : _ID) {
            n = 0;
        }
    }

protected:
    // implementation specific
    void SetBit(uint8_t n, uint8_t v) {
        uint8_t idxBit = n & 7;
        uint8_t idxByte = n >> 3;
        _ID[idxByte] = (_ID[idxByte] & ~(1 << idxBit)) | (v << idxBit);
    }

    uint8_t GetBit(uint8_t n) {
        uint8_t idxBit = n & 7;
        uint8_t idxByte = n >> 3;
        return (_ID[idxByte] >> idxBit) & 1;
    }

    uint8_t _ID[ADDRESS_SIZE];
};

class Discovery {
friend class Bus;
public:
    bool FindNextDevice(Address& address);
    bool HaveMore() const;
    bool HaveParasitePowered() const;

private:
    Discovery(Bus* bus)
        : _Bus(bus)
    {}

    Bus* _Bus;
    uint8_t _LastDiscrepancy = 0;
    bool _Done = false;
};

class Bus : public HAL {
friend class Discovery;
friend class Device;
public:
    // public interface
    Bus(uint8_t pin)
        : HAL(pin)
    {
    }

    Discovery Discover() {
        return Discovery(this);
    }

    // implementation specific
    bool ResetAndDetect() {
        return HAL::ResetPulse();
    }

    void WriteByte(uint8_t byte) {
        for (uint8_t mask = 1; mask; mask <<= 1) {
            if (byte & mask) {
                HAL::WriteSlot1();
            } else {
                HAL::WriteSlot0();
            }
        }
#ifdef MAXIMWIRE_SERIAL_DIAGNOSTICS
        Serial.print("<- ");
        for (uint8_t mask = 0x80; mask; mask >>= 1) {
            if (byte & mask) {
                Serial.print(1);
            } else {
                Serial.print(0);
            }
        }
        Serial.print(" ");
        Serial.println(Hex(byte));
#endif
    }

    uint8_t ReadByte() {
        uint8_t byte = 0;
        for (uint8_t b = 1; b; b <<= 1) {
            if (HAL::ReadSlot()) {
                byte |= b;
            }
        }
#ifdef MAXIMWIRE_SERIAL_DIAGNOSTICS
        Serial.print("-> ");
        for (uint8_t mask = 0x80; mask; mask >>= 1) {
            if (byte & mask) {
                Serial.print(1);
            } else {
                Serial.print(0);
            }
        }
        Serial.print(" ");
        Serial.println(Hex(byte));
#endif
        return byte;
    }

    enum ECommands : uint8_t {
        MatchROM = 0x55,
        ReadPowerSupply = 0xB4,
        SkipROM = 0xCC,
        SearchROM = 0xF0,
    };
};

bool inline Discovery::FindNextDevice(Address& address)  {
    address.Reset();

    if (_Done) {
        return false;
    }
    if (!_Bus->ResetAndDetect()) {
        _LastDiscrepancy = 0;
        _Done = true;
        return false;
    }

    uint8_t discrepancyMarker = 0;

    _Bus->WriteByte(Bus::ECommands::SearchROM);

    for (uint8_t romBitIndex = 1; romBitIndex <= 64; ++romBitIndex) {
        uint8_t bitA = _Bus->ReadSlot();
        uint8_t bitB = _Bus->ReadSlot();

        if (bitA == 1 && bitB == 1) {
            _Done = true;
            return false;
        }
        if (bitA == 0 && bitB == 0) {
            if (romBitIndex == _LastDiscrepancy) {
                address.SetBit(romBitIndex - 1, 1);
            } else {
                if (romBitIndex > _LastDiscrepancy) {
                    address.SetBit(romBitIndex - 1, 0);
                    discrepancyMarker = romBitIndex;
                } else if (address.GetBit(romBitIndex - 1) == 0) {
                    discrepancyMarker = romBitIndex;
                }
            }
        } else {
            address.SetBit(romBitIndex - 1, bitA);
        }
        if (address.GetBit(romBitIndex - 1) == 1) {
            _Bus->WriteSlot1();
        } else {
            _Bus->WriteSlot0();
        }
    }

    _LastDiscrepancy = discrepancyMarker;
    if (_LastDiscrepancy == 0) {
        _Done = true;
    }
    return true;
}

bool inline Discovery::HaveMore() const {
    return !_Done;
}

bool inline Discovery::HaveParasitePowered() const {
    if (_Bus->ResetAndDetect()) {
        _Bus->WriteByte(Bus::ECommands::SkipROM);
        _Bus->WriteByte(Bus::ECommands::ReadPowerSupply);
        return !_Bus->ReadSlot();
    }
    return false;
}

class Device : public Address {
public:
    Device() = default;

    Device(const Address& address)
        : Address(address)
    {}

    Device(const String& address)
	: Address(address)
    {}

    bool IsParasitePowered(Bus& bus) {
        if (MatchRom(bus)) {
            bus.WriteByte(Bus::ECommands::ReadPowerSupply);
            return !bus.ReadSlot();
        }
        return false;
    }

protected:
    // implementation specific
    bool MatchRom(Bus& bus) {
        if (bus.ResetAndDetect()) {
            bus.WriteByte(Bus::ECommands::MatchROM);
            for (uint8_t b : _ID) {
                bus.WriteByte(b);
            }
            return true;
        }
#ifdef MAXIMWIRE_SERIAL_DIAGNOSTICS
        Serial.println("device not found");
#endif
        return false;
    }
};

}

#include "DS18B20.h"
