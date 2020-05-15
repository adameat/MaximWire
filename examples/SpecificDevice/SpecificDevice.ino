#include <MaximWire.h>

#define PIN_BUS 9

MaximWire::Bus bus(PIN_BUS);
MaximWire::DS18B20 device("28FF6BC322170323");

void setup() {
    Serial.begin(9600);
}

void loop() {
    float temp = device.GetTemperature<float>(bus);
    if (!isnan(temp)) {
        Serial.println(temp);
    }
    device.Update(bus);
    delay(1000);
}
