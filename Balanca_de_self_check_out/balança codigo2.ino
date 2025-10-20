#include "HX711.h"

const int PINO_DOUT = 2;
const int PINO_SCK  = 3;
HX711 balanca;

// Substitua pelos seus valores calibrados
const float slope = 0.00451;
const float offset = 300.25;

// Parâmetros de filtragem
const int MEDIAN_COUNT = 5;   // número ímpar para mediana
const float EMA_ALPHA = 0.5; // 0..1 (maior = responde mais rápido, menor = mais suave)
const int AVG_READS = 10;     // usado em read_average()

float emaPeso = 0.0;
bool emaInicializada = false;

long medianBuf[MEDIAN_COUNT];

void setup() {
  Serial.begin(9600);
  while (!Serial) ;
  balanca.begin(PINO_DOUT, PINO_SCK);

  Serial.println("Balança inicializando...");
  delay(500);
  balanca.tare(); // tara inicial
  Serial.println("Tare realizada. Aguardando estabilizacao...");
  // opcional: esperar alguns segundos para estabilizar
  delay(2000);
}

long getMedianReading() {
  // Coleta MEDIAN_COUNT leituras (cada leitura já é média de AVG_READS)
  for (int i = 0; i < MEDIAN_COUNT; ++i) {
    medianBuf[i] = balanca.read_average(AVG_READS);
    delay(20);
  }
  // ordena o buffer (simples insertion sort por ser pequeno)
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
    Serial.println("Balança não encontrada.");
    delay(500);
    return;
  }

  long leituraBruta = getMedianReading();      // leitura bruta estabilizada (mediana)
  float pesoAtual = slope * float(leituraBruta) + offset; // em gramas

  // Inicializa EMA no primeiro valor
  if (!emaInicializada) {
    emaPeso = pesoAtual;
    emaInicializada = true;
  } else {
    // aplica EMA: ema = alpha*novo + (1-alpha)*antigo
    emaPeso = EMA_ALPHA * pesoAtual + (1.0 - EMA_ALPHA) * emaPeso;
  }

  // Arredonda para 2 casas decimais (ajuste conforme preferir)
  float exibicao = round(emaPeso * 100.0) / 100.0;

  Serial.print("Peso bruto: ");
  Serial.print(pesoAtual, 2);
  Serial.print(" g\t EMA: ");
  Serial.print(exibicao, 2);
  Serial.println(" g");

  delay(300); // taxa de atualização — ajuste conforme necessidade
}
