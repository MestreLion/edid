EDID and DDC/CI tools
=====================

Monitors usually implement DDC / E-DDC to communicate with Display Adapters, a collection of protocols for data and control besides the video data itself.

The most widely used DDC protocols are:
- DDC2B (1996), to retrieve the monitor's EDID data containing metadata such as product information, specifications and capabilities.
	- Superseded by E-DDC v1.0 (1999)
- DDC/CI with MCCS, to adjust parameters such as brightness and contrast.

https://en.wikipedia.org/wiki/Display_Data_Channel
https://en.wikipedia.org/wiki/Extended_Display_Identification_Data

--------------------------------------------------------------------------------
DDC2B
-----

- Based on I²C protocol, with a single Master (the Display Adapter) and the Monitor as a slave at address 50h
- Only 128/256 bytes for EDID
- Supported by VGA, DVI and HDMI, but not (directly) by DisplayPort


--------------------------------------------------------------------------------
E-DDC v1.2 (2007)
-----------------

- Also based on I²C, single Master and Slave at 50h
- Segment selector at 30h to allow 128 segments of 256 bytes = 32KiB
- Support for DisplayPort

Latest E-DDC v1.3 (2017) adds only corrections and clarifications from v1.2

Not sure why official docs keep saying I²C address is A0h/A1h when it actually is 50h

Other ways to obtain EDID:
- `xrandr --prop | grep -B1 -A16 EDID`
- `cat /sys/class/drm/card*-*/edid` (if corresponding `./enabled` contains `enabled`)
	- Not very meaningful names from `/sys/bus/i2c/devices/i2c-*/name`

Nice references:
- https://www.kernel.org/doc/Documentation/i2c/
- http://www.ddcutil.com/
- https://github.com/ddccontrol/ddccontrol
- http://www.polypux.org/projects/read-edid/
- https://git.linuxtv.org/edid-decode.git/tree/README

Xlib:
- https://cgit.freedesktop.org/xorg/proto/randrproto/tree/randrproto.txt
- https://gist.github.com/courtarro/3adec649c086eea1bb18919d6269d544

Sysfs:
- https://gitweb.mageia.org/software/monitor-edid/tree/monitor-edid




--------------------------------------------------------------------------------
DDC/CI
------

--------------------------------------------------------------------------------
MCCS
----
