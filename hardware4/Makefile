BOARDNAME=arduino:samd:mkrwifi1010
     PORT=/dev/ttyACM0
#    PORT=/dev/ttyS4

#	sleep 1
#	arduino-cli monitor -p  $(PORT) --discovery-timeout 30s
upload: compile
	arduino-cli upload -v -p $(PORT) --fqbn $(BOARDNAME) .

compile:
	arduino-cli compile -e --fqbn $(BOARDNAME) .
