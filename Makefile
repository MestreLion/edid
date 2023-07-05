PYTHON:=python3
CFLAGS+=-Wall -Wextra

all: build run

run: xlib i2c

build: venv/bin/python edid-i2c

venv/bin/python:
	$(PYTHON) -m venv venv
	PYTHONWARNINGS="ignore::DeprecationWarning" venv/bin/pip install --upgrade pip setuptools wheel
	venv/bin/pip install python-xlib

xlib: venv/bin/python
	venv/bin/python edid-xlib.py

i2c: edid-i2c
	sudo ./edid-i2c

udev:
	sudo cp *.rules /etc/udev/rules.d
	sudo udevadm control --reload
	# sudo udevadm trigger

clean:
	rm -rf venv edid-i2c

.PHONY: all run build xlib i2c clean
