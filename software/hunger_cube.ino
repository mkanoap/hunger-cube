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
  61,  // G (poor)
  118,  // H
  48,  // I (left side)
  30,  // J
  117, // k (poor)
  56,  // L
  79,  // m (looks like a 3)
  84,  // n
  92,  // o
  115, // P
  103, // q (looks like 9)
  80,  // r
  109, // S (looks like 5)
  112, // T (sideways)
  62,  // U
  28,  // v 
  42,  // w (terrible)
  73,  // x (extra terrible)
  110, // Y
  82,  // z (on it's side)
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

void setup () {
// initialize serial communication:
  Serial.begin(9600);
// loop throu pins 2-10 and set them to output
  for (int i=2; i <= 10; i++) {
    pinMode(i, OUTPUT);
  }
}

void loop () {
  for (int i=0; i<=36; i++) {
    update_display(i);
    delay(1000);
  }
}

void update_display(int character) {
  // shift character's bits two to the left for pins 2-7, and leave leave pin 0 and 1 alone
  PORTD = (PORTD & B00000011) | (font[character] << 2);
  // put the last two bits in pins 8 and 9
  PORTB = (PORTB & B11111100) | (font[character] >> 6);
  Serial.print("character is ");
  Serial.print(character);
  Serial.print("-");
  Serial.print(font[character]);
  Serial.print(" Setting portD to ");
  Serial.print(PORTD);
  Serial.print(", PORTB to ");
  Serial.println(PORTB);
}
