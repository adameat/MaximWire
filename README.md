
# MaximWire

This is very simple library to connect DS18B20 temperature sensors and Arduino NANO 33 BLE controllers.

Getting started
---------------

By default, if you use only one DS18B20 device, you don't need external pull-up resistor.
Just connect directly:

* +3.3V -> RED
* BUS   -> YELLOW
* GND   -> BLACK

Where BUS is any digital ping (like 9, for example).
And use example from examples/AnyDevice.

Multiple devices
----------------

When you need to work with multiple devices on the same bus, you need external pull-up resistor of 1.7~2.2kOm.

* +3.3V -> RED1 + RED2 + ...
* BUS   -> YELLOW1 + YELLOW2 + ... 
* GND   -> BLACK1 + BLACK2 + ...

And connect BUS and +3.3V lines with pull-up resistor.

Parasite power
--------------

DS18B20 are able to share bus wire for powering itself, so only two wires required. And you will need external pullup resistor too.

* BUS -> YELLOW
* GND -> RED + BLACK

And connect BUS and +3.3V lines with pull-up resistor.

