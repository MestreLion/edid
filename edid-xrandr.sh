#!/bin/bash

LC_ALL=C xrandr --prop | tr -d '\r' | awk '
	edid && /[^ \ta-f0-9]/ {edid = 0}
	!/^[ \t]/ {print; edid = 0}
	edid {sub(/[ \t]+/, ""); hex = hex ($0 ""); print}
	/EDID.*:/ {edid = 1; hex = ""}
'
