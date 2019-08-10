all:
	javac Keyboard.java
	java Keyboard c64 > virtual_keyboard_c64.h
	java Keyboard vic20 > virtual_keyboard_vic20.h
	
clean:
	rm *.class virtual_keyboard_*.h
