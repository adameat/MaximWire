// multiple devices OR single parasite powered device requires external pull up of 1.7~2.2 kOm

#define MAXIMWIRE_EXTERNAL_PULLUP

#include <MaximWire.h>

#define PIN_BUS 9

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
            if (address.GetModelCode() == MaximWire::DS18B20::MODEL_CODE) {
                Serial.print(" (DS18B20)");
                MaximWire::DS18B20 device(address);
                if (device.IsParasitePowered(bus)) {
                    Serial.print(" (PARASITE POWER)");
                }
                float temp = device.GetTemperature<float>(bus);
                Serial.print(" temp=");
                Serial.print(temp);
                Serial.println();
                device.Update(bus);
            } else {
                Serial.println();
            }
        } else {
            Serial.println("NOTHING FOUND");
        }
    } while (discovery.HaveMore());
    delay(1000);
}
