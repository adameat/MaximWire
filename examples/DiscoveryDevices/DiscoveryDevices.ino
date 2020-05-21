// multiple devices OR single parasite powered device requires external pull up of 1.7~2.2 kOm

#define MAXIMWIRE_EXTERNAL_PULLUP

#include <MaximWire.h>

#define PIN_BUS 2

MaximWire::Bus bus(PIN_BUS);
MaximWire::DS18B20 device;

void setup() {
    Serial.begin(9600);
}

void loop() {
    MaximWire::Discovery discovery = bus.Discover();
    do {
        MaximWire::Address address;
        if (discovery.FindNextDevice(address)) {
            Serial.print("FOUND: ");
            Serial.print(address.ToString());
            if (address.IsValid()) {
                Serial.print(" (VALID)");
            } else {
                Serial.print(" (INVALID)");
            }
            uint8_t modelCode = address.GetModelCode();
            if (modelCode == MaximWire::DS18B20::EmodelCode::CodeDS18B20) {
                Serial.print(" (DS18B20)");
            }
            else if (modelCode == MaximWire::DS18B20::EmodelCode::CodeDS18S20) {
                Serial.print(" (DS18S20)");
            }
            else {
              Serial.println();
              break;
            }
            MaximWire::DS18B20 device(address, modelCode);
            if (device.IsParasitePowered(bus)) {
                Serial.print(" (PARASITE POWER)");
            }
            float temp = device.GetTemperature<float>(bus);
            Serial.print(" temp=");
            Serial.print(temp);
            Serial.println();
            device.Update(bus);
        }
    } while (discovery.HaveMore());
    delay(1000);
}
