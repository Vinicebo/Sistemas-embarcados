#define LED 3
#define POT A0 

void setup(){
 pinMode(LED, OUTPUT); 
}

void loop(){
 int brilho = map(analogRead(POT), 0, 1023, 0, 255); 
 analogWrite(LED, brilho); 
}
