all: delaykey.crt delayjoy.crt

delaykey.8k: delaytest_key.asm
	acme --cpu 6510 delaytest_key.asm

delaykey.crt: delaykey.8k
	cartconv -t normal -i delaykey.8k -o delaykey.crt

delayjoy.8k: delaytest_joy.asm
	acme --cpu 6510 delaytest_joy.asm

delayjoy.crt: delayjoy.8k
	cartconv -t normal -i delayjoy.8k -o delayjoy.crt

clean:
	rm -f delaykey.8k delayjoy.8k delaykey.crt delayjoy.crt
