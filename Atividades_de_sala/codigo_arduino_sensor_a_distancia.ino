int PinTrigger = 2;
int PinEcho = 3;
float TempoEcho = 0;
const float velocidadeSom_mps = 340;
const float velocidadeSom_mpus = 0.000340;

void setup(){
  pinMode(PinTrigger, OUTPUT);
  digitalWrite(PinTrigger, LOW);
  pinMode(PinEcho, INPUT);
  Serial.begin(9600);
  delay(100);
}

void loop(){
  DisparaPulsoUltrassonico();
  TempoEcho = pulseIn(PinEcho, HIGH);
  Serial.print("Distancia em metros: ");
  Serial.println(CalculaDistancia(TempoEcho));
  Serial.print("Distancia em centimentros: ");
  Serial.println(CalculaDistancia(TempoEcho) * 100);
  delay(2000);
}

void DisparaPulsoUltrassonico(){
  digitalWrite(PinTrigger, HIGH);
  delayMicroseconds(10);
  digitalWrite(PinTrigger, LOW);
}

float CalculaDistancia(float tempo_us){
  return ((tempo_us * velocidadeSom_mpus) / 2 );
}
