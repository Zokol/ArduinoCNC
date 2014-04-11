import serial
import sys
from time import sleep


def move(dir):
	print dir
	ser.write(dir)      	# write a string
	ser.sendBreak(0.25)
	ser.flush()
	sleep(1)

def ResetCoords():
	dir = 'r'
	print dir
	ser.write(dir)      	# write a string
	ser.sendBreak(0.25)
	ser.flush()
	sleep(1)
	
def DrawRect(dim):
	print ""
	print "Drawing 10-size rectangle"

	out = ""
	k = 2;

	while(k > 1):
		print "First side:"
		dir = 'd'
		for i in range(0, dim[0]):
			move(dir)
		print "Second side:"
		dir = 'x'
		for i in range(0, dim[1]):
			move(dir)
		print "Third side:"
		dir = 'a'
		for i in range(0, dim[0]):
			move(dir)
		print "Fourth side:"
		dir = 'w'
		for i in range(0, dim[1]):
			move(dir)
			
		print "Finished, starting over."
		print "________________________"
		k = k - 1
		
def ManualControl():
	run = 1
	while run == 1:
		print ""
		print ""
		print "___________________________"
		print "Use Keypad or following keys to control motors"
		print "Direction:"
		print "q w e"
		print "a s d"
		print "z x c"
		print "Drill control:"
		print "  Up: f"
		print "Down: v"
		print ""
		print "Press m to exit to menu"
		print "___________________________"
		select = raw_input(": ")
		
		if select == "m":
			run = 0
		else:
			move(select)

def DrawText():
	print "This option is not ready yet"
	return 0

	
def ClosePort():
	ser.close()             # close port
	
def OpenPort(port):
	print ""
	print "Initializing Com-port to device."

	ser = serial.Serial(port, 9600, 8, serial.PARITY_NONE, 1, None, False, False, None, False, None)
	print "" + ser.portstr + " is open"
	
	return ser
	
def Menu():
	print "___________________________"
	print "Menu"
	print "1. Manual Control"
	print "2. Demonstration"
	print "3. Text carving"
	print "4. Quit"
	print ""
	select = raw_input("Select: ")

	if select == "1":
		ManualControl()
	if select == "2":
		DrawRect([5,5])
	if select == "3":
		DrawText()
	if select == "4":
		Quit()
		
def Quit():
	ClosePort()
	sys.exit()

print "Welcome to PyCNC 0.5"
print "Author: Heikki Juva @ 2011"

print ""
print "___________________________"
port = raw_input("Give CNC port name ")

ser = OpenPort(port)

print ""
while(1):
	Menu()