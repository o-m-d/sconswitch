# Serial Controlled Switch

## Device description
Device have 32 high voltage open collector outputs, controlled by commands
via RS-232 port. Outputs can be in active or passive state and can drive
relays, solenoids, LEDs, opto triacs etc.
Active state - output transistor open, passive state - output transistor
closed.

Device can be used in power control, lighting or any other suitable
applications.

Device connects to controlled system via 40-wire ribbon cable and to
controlling host via DE9 connector.

Device IS NOT realtime and may not used in time-sensitive applications,
e.g. CNC.

Device built on microcontroller ATmega8515-16 in TQFP44 package and
drivers ULN2803A in SOIC-18 package.

In firmware part used [KonstantinChizhov/Mcucpp library] (https://github.com/KonstantinChizhov/Mcucpp)
and [Peter Fleury UART library] (http://homepage.hispeed.ch/peterfleury/avr-software.html).
Firmware compiled by avr-gcc-4.8, pcb designed in KiCAD-4.0.1.

## Supported commands
<pre>
a N|a   - Active output number N (or all)
p N|a   - Passive output number N (or all)
t N D   - Toggle output N for D (1-255) seconds, press s for stop during 
          countdown
n NAME  - Output name, set to NAME or unset if empty given
s       - Show output status
h or ?  - Show this help
</pre>

Output status saved automatically to EEPROM with Active or Passive commands
and restored when power-up.
Commands are not case sensitive. Command line are not editable.

Outputs have numbers from 0 to 31. Toggle means temporary inversion of
output state.

[Demo video](https://youtu.be/6nN7ayRqDR0)

## Electrical characteristics
<pre>
Supply voltage: 9-15V
Supply current: 25mA at 10V
Max voltage on closed output: 50V
Max current via one open output: 500mA
Max total current via all open outputs: 4A (125mA per output and 2x2A per
two ground wires in ribbon cable)
</pre>

Characteristics depend on applied components.

## RS-232 mode
9600 8N1


## Design Recommendations
* Use short as possible ribbon cable.
* Use only 40-wire ribbon cable, not 80-wire UDMA cable.
* ULN2803 have built-in surge protection diodes for inductive load, but
better use discrete diodes, placed as close as possible to coil.
