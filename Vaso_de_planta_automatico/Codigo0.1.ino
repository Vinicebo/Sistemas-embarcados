const int PINO_SOLO = A0; // Potenciômetro simulando sensor de solo
const int TRIG = 7; // Trigger do HC-SR04
const int ECHO = 6; // Echo do HC-SR04
const int PINO_LM35 = A1; // Sensor LM35
const int LED_VERDE = 9; // LED que simula bomba
const int LED_VERMELHO = 10; // Alerta de reservatório vazio

const int LIMITE_SOLO = 600; // Umidade do solo simulada (via potenciômetro)
const int LIMITE_DISTANCIA = 10; // Limite do nível de água (em cm)

long medirDistancia() {
digitalWrite(TRIG, LOW);
delayMicroseconds(2);
digitalWrite(TRIG, HIGH);
delayMicroseconds(10);
digitalWrite(TRIG, LOW);

long duracao = pulseIn(ECHO, HIGH);
long distancia = duracao * 0.034 / 2; // conversão para cm
return distancia;
}

float lerTemperaturaLM35() {
int leitura = analogRead(PINO_LM35);
float tensao = leitura * (5.0 / 1023.0); // Converte leitura para volts
float temperatura = tensao * 100.0; // 10 mV/°C → 100x
return temperatura;
}

void setup() {
Serial.begin(9600);

pinMode(TRIG, OUTPUT);
pinMode(ECHO, INPUT);

pinMode(LED_VERDE, OUTPUT);
pinMode(LED_VERMELHO, OUTPUT);
}

void loop() {
int umidadeSolo = analogRead(PINO_SOLO);
long nivelAgua = medirDistancia();
float temperatura = lerTemperaturaLM35();

Serial.print("Umidade do solo (simulada): ");
Serial.println(umidadeSolo);

Serial.print("Nível da água (cm): ");
Serial.println(nivelAgua);

Serial.print("Temperatura ambiente: ");
Serial.print(temperatura);
Serial.println(" °C");
if (nivelAgua > LIMITE_DISTANCIA) {
// Reservatório vazio
digitalWrite(LED_VERDE, LOW);
digitalWrite(LED_VERMELHO, HIGH);
Serial.println("⚠ Reservatório vazio!");
} else {
digitalWrite(LED_VERMELHO, LOW);
if (umidadeSolo > LIMITE_SOLO) {
digitalWrite(LED_VERDE, HIGH);
Serial.println("🌿 Solo seco → irrigando...");
} else {
digitalWrite(LED_VERDE, LOW);
Serial.println("✅ Solo úmido → sem irrigação.");
}
}

Serial.println("-----------------------------");
delay(2000);
}
