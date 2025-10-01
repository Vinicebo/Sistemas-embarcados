// Inclui a biblioteca para o módulo HX711
#include "HX711.h"

// --- PINAGEM ---
// Defina os pinos do Arduino conectados ao HX711
const int PINO_DOUT = 2; // Data Out do HX711
const int PINO_SCK  = 3; // Clock do HX711

// Cria uma instância do objeto da balança
HX711 balanca;

// --- CALIBRAÇÃO ---
// Este valor é o FATOR DE CALIBRAÇÃO. Ele é crucial e único para cada balança.
// Você precisará descobrir o valor correto para a sua montagem seguindo os passos abaixo.
// Comece com um valor inicial para poder fazer a calibração.
const float slope = 2.3810;
const float offset = 0.0; // Exemplo: -21200.0 (valor negativo é comum)

void setup() {
  // Inicia a comunicação serial para vermos os resultados no computador
  Serial.begin(9600);
  Serial.println("Inicializando a balança inteligente...");

  // Inicializa a balança com os pinos definidos
  balanca.begin(PINO_DOUT, PINO_SCK);


  // Tara a balança (zera o valor inicial)
  // É importante garantir que não haja nada sobre a balança neste momento.
  balanca.tare(); 

  Serial.println("Balança pronta para uso.");
  Serial.println("Coloque um objeto para medir o peso.");
}

void loop() {
  if (balanca.is_ready()) { // <-- Só tenta ler se a balança estiver pronta
    long leitura_bruta = balanca.read_average(10);
    float peso = slope * leitura_bruta + offset;

    Serial.print("Peso: ");
    Serial.print(peso, 2);
    Serial.println(" g");
  } else {
    Serial.println("Balança não encontrada."); // Avisa se houver um problema
  }
  delay(500);
}
