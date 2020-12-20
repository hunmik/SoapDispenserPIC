# SoapDispenserPIC
Firmware for GOJO Soap Dispenser with PIC controller.

We have several soap dispensers, and some of them failed to detect RFID. A new custom firmware is presented which does not use the RFID circuit, so no RFID tag is necessary any more for operation.
Original firmware probably checked the motor current, but this part of the circuitry is foggy and current measurement is not really important therefore not implemented

## Circuit
I have reverse engineered the interesting part of the circuit. Schematics is in KiCad.
A proximity sensor based on an IR LED and photodiode detects any object (preferably a hand) in front of the unit, and sends a 1sec signal to the PIC. This circuit is based on a COB, so no further details are known.
The PIC controls the motor via a H-bridge. It is not clear why a full H-bridge is used, as the motor runs only in one direction. When hand is detected, motor starts, and makes one full rotation. Position is detected by a microswitch on the wheel. When motor is not controlled, it is shorted and stops instantly.
There is another COB on the PCB controlling the RFID.

## Changing firmware
Warning: Original firmware is code protected, the CP bit is set. Therefore it is not possible to save the firmware; once you have erased, it cannot be restored. (If you want to keep the original firmware the only way is to replace the original PIC with a stock one. If you should return to the original firmware, you can solder in the original PIC again.)
CP bit can be reset only with 5V programming voltage. It is not mentioned in the PIC documentation. In the soap dispenser, the PIC is supplied from 3.6V, so to erase the CP bit, 5V must be supplied externally. As there is no information if the other circuits (the COBs) are 5V tolerant, cut the PCB track of VCC near the PIC, solder a thin wire and feed 5V directly to the PIC. Set the PICKIT programmer for external supply, and feed the same 5V voltage to the PIC. Attention: make sure that 5V power is not fed into the PCB through ISCP cable. See the schematic for details.
