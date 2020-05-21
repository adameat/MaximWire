#pragma once

namespace MaximWire {

class DS18B20 : public Device {
public:
    // public interface
    DS18B20() = default;

    DS18B20(const Address& address, uint8_t modelCode = EmodelCode::CodeDS18B20)
        : Device(address), precisionDenominator(getPrecisionDenominatorFromModelCode(modelCode))
    {
    }

    DS18B20(const String& address, uint8_t modelCode = EmodelCode::CodeDS18B20)
	    : Device(address), precisionDenominator(getPrecisionDenominatorFromModelCode(modelCode))
    {
    }

    DS18B20(const uint8_t (&address)[Address::ADDRESS_SIZE], uint8_t modelCode = EmodelCode::CodeDS18B20)
        : Device(address), precisionDenominator(getPrecisionDenominatorFromModelCode(modelCode))
    {
    }

    void Update(Bus& bus) { // it requires 93.75ms..750ms for background conversion (depending on resolution)
        if (Device::MatchRom(bus)) {
            bus.WriteByte(ECommands::ConvertT);
        }
    }

    template <typename T>
    T GetTemperature(Bus& bus);
    
    enum EmodelCode : uint8_t {
        CodeDS18B20 = 0x28,
        CodeDS18S20 = 0x10,
    };

    // implementation specific
    enum ECommands : uint8_t {
        ConvertT = 0x44,
        ReadScratchpad = 0XBE,
    };

    enum EResolution : uint8_t {
        Bits9 = 0,
        Bits10 = 1,
        Bits11 = 2,
        Bits12 = 3,
    };

    struct ConfigurationRegister { 
        uint8_t Reserver:5;
        uint8_t Resolution:2;
    };

    static_assert(sizeof(ConfigurationRegister) == 1);

#pragma pack(push, 1)
    struct ScratchPad {
        int16_t Temperature;
        uint8_t Th;
        uint8_t Tl;
        ConfigurationRegister Configuration;
        uint8_t Reserved[3];
        uint8_t CRC8;
    };
#pragma pack(pop)

    static_assert(sizeof(ScratchPad) == 9);

    union ReadData {
        ScratchPad StructuredData;
        uint8_t RawData[9];
    };

    static constexpr int16_t UNKNOWN_TEMPERATURE = 0x550;
private:
    uint8_t precisionDenominator;
    uint8_t getPrecisionDenominatorFromModelCode(uint8_t modelCode);
};

template <>
int16_t DS18B20::GetTemperature<int16_t>(Bus& bus) {
    ReadData data;
    data.StructuredData.Temperature = UNKNOWN_TEMPERATURE;
    for (int retries = 3; retries > 0; --retries) {
        if (Device::MatchRom(bus)) {
            bus.WriteByte(ECommands::ReadScratchpad);
            for (uint8_t& b : data.RawData) {
                b = bus.ReadByte();
            }
            if (data.StructuredData.CRC8 == Bus::CRC8(data.RawData, 8)) {
                switch (data.StructuredData.Configuration.Resolution) {
                    case EResolution::Bits11:
                        data.StructuredData.Temperature &= 0xFFFE;
                        break;
                    case EResolution::Bits10:
                        data.StructuredData.Temperature &= 0xFFFC;
                        break;
                    case EResolution::Bits9:
                        data.StructuredData.Temperature &= 0xFFF8;
                        break;
                }
                break;
            } else {
                data.StructuredData.Temperature = UNKNOWN_TEMPERATURE;
                continue;
            }
        }
    }
    return data.StructuredData.Temperature;
}

template <>
float DS18B20::GetTemperature<float>(Bus& bus) {
    int16_t temperature = GetTemperature<int16_t>(bus);
    if (temperature == UNKNOWN_TEMPERATURE) {
        return NAN;
    } else {
        return (float)temperature / precisionDenominator;
    }
}

template <>
int DS18B20::GetTemperature<int>(Bus& bus) {
    int16_t temperature = GetTemperature<int16_t>(bus);
    if (temperature == UNKNOWN_TEMPERATURE) {
        return 0;
    } else {
        return temperature / precisionDenominator;
    }
}

uint8_t DS18B20::getPrecisionDenominatorFromModelCode(uint8_t modelCode){
    uint8_t denominator = 16; // Default: DS18B20
    switch(modelCode) {
        case EmodelCode::CodeDS18B20:
            break;
        case EmodelCode::CodeDS18S20:
            denominator = 2;
            break;
    }
    return denominator;
}
}
