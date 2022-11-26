BOARDNAME=arduino:samd:mkrwifi1010
     PORT=/dev/ttyACM0
#    PORT=/dev/ttyS4

#	sleep 1
#	arduino-cli monitor -p  $(PORT) --discovery-timeout 30s

compile:
	arduino-cli compile --upload -p  $(PORT) --fqbn $(BOARDNAME) .

upload: compile
	arduino-cli upload -v -p $(PORT) --fqbn $(BOARDNAME) .

ju:
	arduino-cli upload -v -p $(PORT) --fqbn $(BOARDNAME) \
--additional-urls upload.php .


preproc:
	arduino-cli compile --preprocess --fqbn $(BOARDNAME) .

jc:
	arduino-cli compile --fqbn $(BOARDNAME) .
