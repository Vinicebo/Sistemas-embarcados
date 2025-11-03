#include "HX711.h"

// --- Configuração dos Pinos ---
const int PINO_DOUT = 2;
const int PINO_SCK  = 3; // Corrigi o caractere inválido que havia no seu código original
HX711 balanca;

// --- Configuração da Calibração (Substitua pelos seus valores) ---
// Estes valores são cruciais para a precisão!
const float slope = 0.00451; 
const float offset = 300.25;

// --- Parâmetros de Filtragem ---
const int MEDIAN_COUNT = 5;    // Número ímpar para o filtro de mediana
const int AVG_READS = 10;      // Leituras para a biblioteca fazer a média (read_average)
const float EMA_ALPHA = 0.85; // 0..1 (0.5 é um bom equilíbrio. Maior = mais rápido, Menor = mais suave)

// --- Limiar para o Reset Inteligente (em gramas) ---
// Se o peso filtrado cair abaixo deste valor, ele força o zero imediatamente.
const float LIMIAR_ZERO_GRAMAS = 5.0; // 5 gramas (ajuste se necessário)

// --- Variáveis Globais de Filtragem ---
float emaPeso = 0.0;
bool emaInicializada = false;
long medianBuf[MEDIAN_COUNT];

//=================================================================
// SETUP
//=================================================================
void setup() {
  Serial.begin(9600);
  while (!Serial) ; // Espera a porta serial conectar
  balanca.begin(PINO_DOUT, PINO_SCK);

  delay(500);
  balanca.tare(); // Zera a balança na inicialização
  delay(2000); // Espera a balança estabilizar após a tara
}

//=================================================================
// FUNÇÃO DE LEITURA (Filtro de Mediana)
//=================================================================
long getMedianReading() {
  // 1. Coleta MEDIAN_COUNT leituras (cada leitura já é uma média de AVG_READS)
  for (int i = 0; i < MEDIAN_COUNT; ++i) {
    medianBuf[i] = balanca.read_average(AVG_READS);
    delay(10); // Pequena pausa entre as leituras
  }
  
  // 2. Ordena o buffer (insertion sort)
  for (int i = 1; i < MEDIAN_COUNT; ++i) {
    long key = medianBuf[i];
    int j = i - 1;
    while (j >= 0 && medianBuf[j] > key) {
      medianBuf[j+1] = medianBuf[j];
      j = j - 1;
    }
    medianBuf[j+1] = key;
  }
  
  // 3. Retorna o valor do meio (mediana)
  return medianBuf[MEDIAN_COUNT/2]; 
}

//=================================================================
// LOOP PRINCIPAL
//=================================================================
void loop() {
  if (!balanca.is_ready()) {
    delay(500);
    return;
  }

  // 1. Pega a leitura bruta filtrada (Mediana de Médias)
  long leituraBruta = getMedianReading();
  
  // 2. Converte para gramas
  float pesoAtual_gramas = slope * float(leituraBruta) + offset;

  // 3. Aplica o filtro EMA (Média Móvel Exponencial) para suavizar
  if (!emaInicializada) {
    emaPeso = pesoAtual_gramas;
    emaInicializada = true;
  } else {
    emaPeso = EMA_ALPHA * pesoAtual_gramas + (1.0 - EMA_ALPHA) * emaPeso;
  }

  // --- 4. IMPLEMENTAÇÃO DO RESET INTELIGENTE ---
  
  float peso_final_em_gramas;

  // Se o peso filtrado (emaPeso) cair abaixo do nosso limiar...
  if (emaPeso < LIMIAR_ZERO_GRAMAS) {
      // ...força o peso para zero imediatamente.
      peso_final_em_gramas = 0.0;
      
      // E reseta o filtro EMA para que ele não fique "preso" em valores baixos.
      // Na próxima vez que um peso for adicionado, ele inicializará instantaneamente.
      emaPeso = 0.0;
      emaInicializada = false;
  } else {
      // Se estiver acima do limiar, apenas usa o valor filtrado
      peso_final_em_gramas = emaPeso;
  }
  
  // 5. Converte o peso final de gramas para quilogramas
  float peso_em_kg = peso_final_em_gramas / 1000.0;

  // 6. Envia APENAS o número do peso em kg pela serial para o Python
  Serial.println(peso_em_kg, 4); // 4 casas decimais (ex: 0.1525)

  // Taxa de atualização (10 leituras por segundo)
  delay(100); 
}
