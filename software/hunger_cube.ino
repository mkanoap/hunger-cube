/*
  assumes display is wired as follows:
  ardino pin,segment,display pin
    2,a,7
    3,b,6
    4,c,4
    5,d,2
    6,e,1
    7,f,9
    8,g,10
    9,dp,5
    gnd,gnd,3 & 8
*/
 #include "pitches.h"
/*
  set up alphabet array, a-z0-9.  Arduino pins must be sequential for this to work.
  alphabet is encoded as 1-36 with bit 1=segment a, bit 2=segment b, etc.
*/

int font[] = {
  0,   // norhing
  119, // A
  124,  // b
  57,  // C
  94,  // d
  121, // E
  113, // F
  111,  // G (poor, looks like 6)
  118,  // H
  48,  // I (left side)
  30,  // J
  117, // k (poor)
  56,  // L
  79,  // m (looks like a 3)
  55,  // n
  63,  // o
  115, // P
  103, // q (looks like 9)
  49,  // r
  109, // S (looks like 5)
  120, //  112, // T (sideways)
  62,  // U
  28,  // v 
  42,  // w (terrible)
  73,  // x (extra terrible)
  110, // Y
  91,  // z (on it's side)
  191,  // 0
  134,   // 1
  219, // 2
  207, // 3
  230,  // 4
  237, // 5
  253, // 6
  135,  //7
  255, // 8
  239  // 9
};

// i/o pin assignments
int lightpin1=0;  // light sensor on analog pin 0
int lightpin2=1;  // light sensor on analog pin 1
int temppin=2;    // temperature sensor on analog pin 2, MCP9700
int reedpin=17;    // reed sensor on digital pin 3
int buzzpin=10;   // buzzer pin
int tiltpin=11;   // tilt sensor pin
int enterpin=12;  // the button
int screwpin=13;  // secret screw button

// other global values
int inputvalue=0;  // holds the bits of the sensors
int buttonState;  // stores the current state of the button
int enterState=0; // status of enter button.  0= off or on for the first time, 1=already on
int lastButtonState = LOW;
long lastDebounceTime = 0; // last time the button was toggled
int tempCount=0;  // counter used to for temperature debounce
int tempCountMax=3;  // how many times the temperature must be the same
int tempState; // whether the temperature is out of range
int lastTempState = 0; // counter for temperature debounce
int letter = 0; // current letter being entered
float starting_temp; // temperature when device it turned on
int blinkTime = 2000; // time a decimal blink cycle should last
int dOnTime = 80; // time a decimal blink should last
int dOffTime = blinkTime; // time between blinks
int decState = 0; // whether the decimal point is on or not
int lastBlinkTime = 0; // counter used to tell how long the decimal blink has been on or off
int blinkCounter=0; // used to count how many times it has blinked
int guesses =0;  // how many times they have guessed wrong

// tuneable values
int light2threshold=40; // value at which to trigger light bit.  Bigger numbers require brighter light.
int tempThreshold=2; // how many degrees (celcius) change to trigger temperature bit.
long debounceDelay = 10; // how long to decide the button is really pushed.
char message[] = "the red ring of death"; // message and codeword must be all lower case
int maxwrong = 300; // number of times they can guess wrong without resetting
char codeword[] = "forceful"; // 6 15 18 3 5 6 21 12 - 4+2 8+4+2+1 16+2 1+2 4+1 4+2 16+4+1 8+4 - f=m+w o=l+m+w+t r=c+w c=t+w e=m+t f=m+w u=c+m+t l=l+m

//char codeword[] = "aaaaaaaa";


void setup () {
// initialize serial communication:
  Serial.begin(9600);
// loop throu pins 2-10 and set them to output
  for (int i=2; i <= 10; i++) {
    pinMode(i, OUTPUT);
  }
// set the pins as input with pullup resistors
  pinMode(reedpin, INPUT); digitalWrite(reedpin, HIGH);
  pinMode(tiltpin, INPUT); digitalWrite(tiltpin, HIGH);
  pinMode(enterpin, INPUT); digitalWrite(enterpin, HIGH);
  pinMode(screwpin, INPUT); digitalWrite(screwpin, HIGH);
// show that it's been turned on
  animate(30,3);

// calibrate the temperature sensor with starting temperature
  set_display(0);
  delay(10);
  starting_temp=read_temp();
  lastBlinkTime = millis(); //start the blink timer
//  play_tune();
//  show_font();
}

void loop () {
  set_display(0); // turn off display so the voltage drop does not interfere with reading sensors
  int readvalue=read_sensors();
  int letterBits=font[inputvalue];  // figure out which bits go for the current letter to display
  letterBits = letterBits | (decState << 7); // turn on the 8th bit if the decimal point should be forced on
  set_display(letterBits); // turn display back on
  if (readvalue!=inputvalue) {
    inputvalue=readvalue;
    set_display(letterBits);
  }
  if (buttonState==0) { // if the button is NOT pushed
    enterState=0; // we are not entering a new value
  } else {
    if (enterState==0) { // the button has been pushed AND we have not yet entered the result
      make_guess();
      enterState=1; // set so we don't repeat while button is pushed down.
    } // end of "enter state=0"
  } // end of "button pushed"
  delay(75);
  update_blink();
  delay(75);
}

void show_font() {
    for (int i=1;i<37;i++) {
      set_display(font[i]);
      delay(1000);
    }
}

void update_blink() { // check to see if it's time to toggle the decimal point
  int blinkDelta=millis()-lastBlinkTime; // see how long it has been since the blinker turned on
  if (blinkDelta < dOnTime && blinkCounter <= letter ) { // if within the "on" period, turn on
    decState=1;
  } else {
    decState=0;
  }
  if (blinkDelta > dOffTime) { // if it's gone beyond the "off" period, start over.
    lastBlinkTime=millis();
    if (blinkCounter <= letter) {
      decState=1; // and turn on
    }
    blinkCounter++; // increment the blink counter
    if (blinkCounter > (letter+1)) {
      blinkCounter=0;
    }
  }
}

void make_guess() { // input current state
  int letternum= codeword[letter] - 96;
  if (inputvalue!=letternum) {
    tone(buzzpin,NOTE_B2,500);
    delay(500);
    guesses++; // increment the bad guess counter
    if (guesses>=maxwrong) {  // if they have guess wrong too many times, reset
      tone(buzzpin,NOTE_B1,500);
      delay(500);
      letter=0;
      guesses=0;
      dOffTime=blinkTime;
    }
  } else {
    tone(buzzpin,NOTE_B4,250);
    letter++; // increment the current letter
    dOffTime=blinkTime/(letter +1);  // 
    if (letter>= sizeof(codeword) - 1) {
      do_finish();
    }
  }
  Serial.println(letternum);
}

void do_finish() {  // run after all the correct value is entered
  delay(500);
  decState=0;
  set_display(0);
  play_tune();
  int displayValue=0;
  int mnum=0;
  while (true) { // do until turned off
    animate2(30,10);
    for (int i=0; i<sizeof(message); i++) {
      mnum=message[i];
      mnum=mnum-96;
      displayValue=font[mnum];
//      displayValue=font[message[i] - 96];
      if (mnum<1) {
        displayValue=0;
      }
      set_display(displayValue);
      delay(500);
      set_display(0);
      delay(200);
    }
    set_display(0);
    delay(500);
  }
}

float read_temp() {
//  delay(10);
  float temperature=analogRead(temppin); // mostly just to switch the ADC to this pin
  delay(1);
  temperature=analogRead(temppin);  // do it twice because the first value is probably wrong
  temperature=temperature*5/1024.0; // times 5 volts divided by 1024 steps
  temperature=temperature - 0.4;    // 400mv offset for this sensor
  temperature=temperature / 0.0195;   // 19.5mv per step
  return temperature;
}

int read_sensors() {
  int currentval=0; // holds the bits for the current sensors
//  int light1=analogRead(lightpin1);
//  delay(10);
  int light2=analogRead(lightpin2);  // set ADC to this pin
  delay(1);
  light2=analogRead(lightpin2); // read light value again, because the first reading was probably wrong
  float current_temperature= read_temp();
  float temp_delta= current_temperature-starting_temp;
  temp_delta=abs(temp_delta);
  
  int magnet=!digitalRead(reedpin);   // magnets, how do they work?
  int tilt=!digitalRead(tiltpin);    // are we upside down?
  int screw=!digitalRead(screwpin);  // are the screws connected?
  int curTempBool= (temp_delta > tempThreshold); // see if the temperature is out of bounds
  
  if (curTempBool != lastTempState) {
    tempCount=0; // reset the timer every time the temp delta toggles
  } else {
    tempCount++;
  }
  if (tempCount > tempCountMax) {
    // it's been the same for tempCountMax times
    tempState=curTempBool; // go ahead and update the state
  }
  lastTempState=curTempBool;  // always update the lastButtonState to the current reading

  currentval=tempState;
  currentval=currentval<<1;
  currentval=currentval | (light2 > light2threshold);  // set the light detection bit (4);
  currentval=currentval<<1;  // shift it over for the next 
  currentval=currentval | magnet; // set the magnet bit (3)
  currentval=currentval<<1;  // shift it over for the next
  currentval=currentval | screw; // set the screw bit (2)
  currentval=currentval<<1;  // shift it over for the next
  currentval=currentval | tilt;  // set the tilt bit (1)
//  currentval=currentval<<1;  // shift it over for the next
  
  readbutton();
//  Serial.print(light1);
//  Serial.print("\t");

  Serial.print(light2);
  Serial.print("\t");
  Serial.print(tilt);
  Serial.print("\t");
  Serial.print(current_temperature);
  Serial.print("(");
  Serial.print(temp_delta);
  Serial.print(") ctb=");
  Serial.print(curTempBool);
  Serial.print(" ts=");
  Serial.print(tempState);
  Serial.print("\tlt=");
  Serial.print(lastTempState);
  Serial.print("\tbc=");
  Serial.print(blinkCounter);
  Serial.print("\t");
  Serial.print(letter);
  Serial.print("\t");
  Serial.print(blinkCounter <= letter);
  Serial.println();

  return(currentval);
}

void readbutton() {
  int buttval = !digitalRead(enterpin);  // we want to return 1 when button is pushed to LOW
  if (buttval != lastButtonState) {
    lastDebounceTime = millis(); // reset the timer every time the button toggles
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // it's been the same for 'debounceDelay' milliseconds
    if (buttval != buttonState) { // the button has changed!
      buttonState=buttval; // go ahead and update the state
    }
  }
  lastButtonState=buttval;  // always update the lastButtonState to the current reading
}

void set_display(int value) {
//  value = value | (decState << 7); // turn on the 8th bit if the decimal point should be forced on
//  Serial.println(value);
  // shift character's bits two to the left for pins 2-7, and leave leave pin 0 and 1 alone
  PORTD = (PORTD & B00000011) | (value << 2);
  // put the last two bits in pins 8 and 9
  PORTB = (PORTB & B11111100) | (value >> 6);
}

void animate(int delval, int times) {
  int frame[] = {1,2,64,16,8,4,64,32}; // make a figure 8
  for (int i=0; i <= times; i++) {
    for (int ii=0; ii <= 7; ii++) {
      set_display(frame[ii]);
      delay(delval);
    }
  }
}

void animate2(int delval, int times) {
  int frame[] = {1,2,4,8,16,32}; // make a figure 0
  for (int i=0; i <= times; i++) {
    for (int ii=0; ii <= 5; ii++) {
      set_display(frame[ii]);
      delay(delval);
    }
  }
}

void play_tune() {
  int melody[]= {
    NOTE_G4, NOTE_AS4, NOTE_A4, NOTE_D4};
  int noteDurations[] = {
    2, 2, 4, 2};
  for (int thisNote=0; thisNote < sizeof(melody); thisNote++) {
    int noteDuration = 1000/noteDurations[thisNote];
    tone(buzzpin, melody[thisNote],noteDuration);
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    noTone(buzzpin);
  }
}
