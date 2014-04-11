/*

	CNC driving circuit
	Microcontroller: ATMEGA323P
	Programming platform: Arduino UNO
	Driver circuit: 4094 and 2*L298
	Display: 16*2 Character LCD
	
	Author: Heikki Juva - 2011
	
   
  LCD circuit:
 * LCD RS pin to digital pin 7
 * LCD Enable pin to digital pin 6
 * LCD D4 pin to digital pin 5
 * LCD D5 pin to digital pin 4
 * LCD D6 pin to digital pin 3
 * LCD D7 pin to digital pin 2
 * LCD R/W pin to ground
 * 10K resistor:
 * ends to +5V and ground
 * wiper to LCD VO pin (pin 3)
 
 Shift Register circuit:
 * Data to digital pin 12
 * Clock to digital pin 11
 * Latch to digital pin 10
 
 
 LCD Library:
 Library originally added 18 Apr 2008
 by David A. Mellis
 library modified 5 Jul 2009
 by Limor Fried (http://www.ladyada.net)
 example added 9 Jul 2009
 by Tom Igoe
 modified 22 Nov 2010
 by Tom Igoe
 
 ShiftRegister example:
 Alex Zivanovic
 March 2009
 http://senster.com/blog/wp-content/uploads/2009/02/expanding_outputs.pdf
 
 
 This code uses public domain libraries of SimpleTimer and LED.
 
 Author: Heikki Juva - 2011
 */

// include the library code:
#include <LiquidCrystal.h>
#include <SimpleTimer.h>
#include <LED.h>


LiquidCrystal lcd(7, 6, 5, 4, 3, 2); //LCD init
int CoordX = 0; //Coordinates
int CoordY = 0;
int latchPin = 10; //ShiftRegister init
int clockPin = 11;
int dataPin = 12;

//Drill control init
LED DrillUp = LED(13);
LED DrillDown = LED(9);

SimpleTimer timer; // Timer to move drill

//Motor steps, note different steps for motor A and motor B, so that different steps can be OR'ed.
byte STEPA1 = B10000000;
byte STEPA2 = B00100000;
byte STEPA3 = B01000000;
byte STEPA4 = B00010000;
byte STEPB1 = B00001000;
byte STEPB2 = B00000010;
byte STEPB3 = B00000100;
byte STEPB4 = B00000001;

int LastStepA = 1; // Variables to save last motor steps
int LastStepB = 1;

char dirA = 'N'; // Motor directions for next step
char dirB = 'N';

byte Output = B00000000; //Output that is sent to ShiftRegister

String incomingString = "";		// For incoming serial data
String outgoingString = "";     // For outgoing serial data

void setup() {
  // LCD code
  // set up the LCD's number of columns and rows: 
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("Coordinates");
  
  //Setup pin modes to ShiftRegister
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  digitalWrite(latchPin, 0);        //Make sure data isn't latched
  
  Serial.begin(9600);	// opens serial port, sets data rate to 9600 bps
  
  //Calculate first step and output it to ShiftRegister.
  Step(dirA, dirB);
  WriteData(Output);
  
  //Make sure the drill is lifted at startup.
  LiftDrill();
}

void loop() {
  
  //Start timer, this is used to lift and lower the drill.
  timer.run();
  
  // send data only when you receive data:
	if (Serial.available() > 0) {
		// read the incoming byte:
		incomingString = Serial.read();
                
				// If incoming is "q"
                if (incomingString == 101){
                  dirA = 'F';
                  dirB = 'F';
                }
				// If incoming is "e"
                else if (incomingString == 113){
                  dirA = 'F';
                  dirB = 'R';
                }
				// If incoming is "w"
                else if (incomingString == 119){
                  dirA = 'F';
                  dirB = 'N';
                }
				// If incoming is "z"
                else if (incomingString == 99){
                  dirA = 'R';
                  dirB = 'F';
                }
				// If incoming is "c"
                else if (incomingString == 122){
                  dirA = 'R';
                  dirB = 'R';
                }
				// If incoming is "x"
                else if (incomingString == 120){
                  dirA = 'R';
                  dirB = 'N';
                }
				// If incoming is "a"
                else if (incomingString == 100){
                  dirA = 'N';
                  dirB = 'F';
                }
				// If incoming is "d"
                else if (incomingString == 97){
                  dirA = 'N';
                  dirB = 'R';
                }
				// If incoming is "s"
                else if (incomingString == 115){
                  dirA = 'N';
                  dirB = 'N';
                }
				// If incoming is "r"
                else if (incomingString == 114){
                  ResetCoords();
                  dirA = 'N';
                  dirB = 'N';
                }
				// If incoming is "f"
                else if (incomingString == 102){
                  LiftDrill();
                  Serial.print("Drill is up");
                  Serial.print("\n");
                  dirA = 'N';
                  dirB = 'N';
                }
				// If incoming is "v"
                else if (incomingString == 118){
                  LowerDrill();
                  Serial.print("Drill is down");
                  Serial.print("\n");
                  dirA = 'N';
                  dirB = 'N';
                }

				//Calculate steps according to selected direction and last step
                Step(dirA, dirB);
				//Write data to ShiftRegister
                WriteData(Output);
                
				//Write coordinates to LCD
                outgoingString = "";
                outgoingString = outgoingString + "X: ";
                outgoingString = outgoingString + CoordX;
                outgoingString = outgoingString + "   Y: ";
                outgoingString = outgoingString + CoordY;
                //Write coordintes to Serial port
                Serial.print("Position");
                Serial.print(outgoingString);
                Serial.print("\n");
                Serial.print("\n");
	}

  // LCD Part of code
  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
  lcd.setCursor(0, 1);
  // print the number of seconds since reset:
  lcd.print("X: ");
  lcd.setCursor(3, 1);
  lcd.print(CoordX);
  lcd.setCursor(8, 1);
  lcd.print("Y: ");
  lcd.setCursor(11, 1);
  lcd.print(CoordY);
}

// Function to use ShiftRegisters as output.
// Takes byte-value as input
void WriteData(unsigned int outputpattern){
  for (int i=0 ; i < 8; i++){
    shiftOut(dataPin, clockPin, LSBFIRST, (byte) outputpattern);     //Shift the data to register
    digitalWrite(latchPin, 1);                // flick the latch to put the data on the output pins
    delay(100);
    digitalWrite(latchPin, 0);
  }
}

// Function to calculate next steps
void Step(char dirA, char dirB){
  // Get byte-value for motor A
  Output = DirStepA(dirA);
  // Get byte-value for motor B and OR it with motor A value
  Output = Output | DirStepB(dirB);
}

// Function to tell what is next step for motor A
// input is direction that motor should turn, this can be Forward, Reverse, or No movement.
// coordinates are also calculated based on motor movement.
byte DirStepA(char dir){
  byte next = B00000000;
  if (dir == 'F'){
    CoordX++;
    if (LastStepA == 1){
      next = STEPA2;
      LastStepA = 2;
    }
    else if (LastStepA == 2){
      next = STEPA3;
      LastStepA = 3;
    }
    else if (LastStepA == 3){
      next = STEPA4;
      LastStepA = 4;
    }
    else if (LastStepA == 4){
      next = STEPA1;
      LastStepA = 1;
    }
  }
  else if (dir == 'R'){
    CoordX--;
    if (LastStepA == 1){
      next = STEPA2;
      LastStepA = 4;
    }
    else if (LastStepA == 2){
      next = STEPA3;
      LastStepA = 1;
    }
    else if (LastStepA == 3){
      next = STEPA4;
      LastStepA = 2;
    }
    else if (LastStepA == 4){
      next = STEPA1;
      LastStepA = 3;
    }
  }
  else{
    if (LastStepA == 1){
      next = STEPA1;
    }
    else if (LastStepA == 2){
      next = STEPA2;
    }
    else if (LastStepA == 3){
      next = STEPA3;
    }
    else if (LastStepA == 4){
      next = STEPA4;
    }
  }
  return next;
}

byte DirStepB(char dir){
  byte next = B00000000;
  if (dir == 'F'){
    CoordY++;
    if (LastStepB == 1){
      next = STEPB2;
      LastStepB = 2;
    }
    else if (LastStepB == 2){
      next = STEPB3;
      LastStepB = 3;
    }
    else if (LastStepB == 3){
      next = STEPB4;
      LastStepB = 4;
    }
    else if (LastStepB == 4){
      next = STEPB1;
      LastStepB = 1;
    }
  }
  else if (dir == 'R'){
    CoordY--;
    if (LastStepB == 1){
      next = STEPB4;
      LastStepB = 4;
    }
    else if (LastStepB == 2){
      next = STEPB1;
      LastStepB = 1;
    }
    else if (LastStepB == 3){
      next = STEPB2;
      LastStepB = 2;
    }
    else if (LastStepB == 4){
      next = STEPB3;
      LastStepB = 3;
    }
  }
  else{
    if (LastStepB == 1){
      next = STEPB1;
    }
    else if (LastStepB == 2){
      next = STEPB2;
    }
    else if (LastStepB == 3){
      next = STEPB3;
    }
    else if (LastStepB == 4){
      next = STEPB4;
    }
  }
  return next;
}

// Reset coordinate values to zero
void ResetCoords(){
  CoordX = 0;
  CoordY = 0;
}

// Lift drill, outputs 2sec pulse to lifting pin.
void LiftDrill(){
  DrillUp.on();
  timer.setTimeout(2000, StopDrillMove);
}

// Lower drill, outputs 2sec pulse to lowering pin
void LowerDrill(){
  DrillDown.on();
  timer.setTimeout(2000, StopDrillMove);
}

// Stop drill movement, sets lifting and lowering pins to LOW state.
void StopDrillMove(){
  DrillUp.off();
  DrillDown.off();
}
