#include "HX711.h"

const int PINO_DOUT = 2;
const int PINO_SCK  = 3; // O pino SCK tinha um caractere inválido no seu original ( ), corrigido aqui.
HX711 balanca;

// Substitua pelos seus valores calibrados
const float slope = 0.00451;
const float offset = 300.25;

// Parâmetros de filtragem
const int MEDIAN_COUNT = 5;    // número ímpar para mediana
const float EMA_ALPHA = 0.5; // 0..1 (maior = responde mais rápido, menor = mais suave)
const int AVG_READS = 10;      // usado em read_average()

float emaPeso = 0.0;
bool emaInicializada = false;

long medianBuf[MEDIAN_COUNT];

void setup() {
  Serial.begin(9600);
  while (!Serial) ;
  balanca.begin(PINO_DOUT, PINO_SCK);

  // Serial.println("Balança inicializando..."); // Podemos remover os logs humanos
  delay(500);
  balanca.tare(); // tara inicial
  // Serial.println("Tare realizada. Aguardando estabilizacao...");
  delay(2000);
}

long getMedianReading() {
  for (int i = 0; i < MEDIAN_COUNT; ++i) {
    medianBuf[i] = balanca.read_average(AVG_READS);
    delay(20);
  }
  for (int i = 1; i < MEDIAN_COUNT; ++i) {
    long key = medianBuf[i];
    int j = i - 1;
    while (j >= 0 && medianBuf[j] > key) {
      medianBuf[j+1] = medianBuf[j];
      j = j - 1;
    }
    medianBuf[j+1] = key;
  }
  return medianBuf[MEDIAN_COUNT/2]; // retorna mediana
}

void loop() {
  if (!balanca.is_ready()) {
    // Serial.println("Balança não encontrada."); // Removemos
    delay(500);
    return;
  }

  long leituraBruta = getMedianReading();
  float pesoAtual = slope * float(leituraBruta) + offset; // em gramas

  if (!emaInicializada) {
    emaPeso = pesoAtual;
    emaInicializada = true;
  } else {
    emaPeso = EMA_ALPHA * pesoAtual + (1.0 - EMA_ALPHA) * emaPeso;
  }

  // Pega o valor estabilizado (em gramas)
  float exibicao_em_gramas = round(emaPeso * 100.0) / 100.0;

  // Garante que o peso não seja negativo (filtro de ruído)
  if (exibicao_em_gramas < 1.0) { // Ajuste o limiar (ex: 1 grama)
      exibicao_em_gramas = 0.0;
  }

  // Converte o peso final de gramas para quilogramas
  float peso_em_kg = exibicao_em_gramas / 1000.0;

  // Envia APENAS o número do peso em kg pela serial
  // O '4' é o número de casas decimais (ex: 0.1525 kg)
  Serial.println(peso_em_kg, 4);

  delay(300); // Taxa de atualização (envia ~3x por segundo)
}
