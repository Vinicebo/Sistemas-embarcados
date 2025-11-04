/*
 * PROTÓTIPO DE IRRIGAÇÃO AUTOMÁTICA (LÓGICA INTELIGENTE)
 * Placa: Arduino Mega
 * Sensores: SENSOR CAPACITIVO (Umidade), HC-SR04 (Nível), 
 * NTC (Microclima em A1), NTC (Ambiente em A2)
 * Atuador: Módulo Relé + Bomba 5V
 */

// --- Bibliotecas ---
#include <math.h> // Necessário para a função log() do termistor

// --- Pinos dos Sensores ---
#define PINO_UMIDADE_SOLO   A0  // Sensor Capacitivo
#define PINO_NTC_MICRO      A1  // NTC para Microclima (planta)
#define PINO_NTC_AMBIENTE   A2  // NTC para Temp. Ambiente
#define PINO_TRIGER         7   // Sensor Ultrassônico
#define PINO_ECHO           6   // Sensor Ultrassônico

// --- Pinos dos Atuadores e LEDs ---
#define PINO_RELE_BOMBA     24  // Módulo Relé (Pino 'IN')
#define LED_IRRIGANDO       30  // LED Verde
#define LED_AGUA_BAIXA      32  // LED Vermelho
#define LED_SISTEMA_OK      31  // LED Amarelo

// --- LÓGICA DO RELÉ ---
#define BOMBA_LIGADA    LOW
#define BOMBA_DESLIGADA HIGH

// --- Constantes de Calibração (SENSOR CAPACITIVO) ---
#define VALOR_SENSOR_SECO 360     // Calibrado por você (Ar/Seco = Valor Alto)
#define VALOR_SENSOR_MOLHADO 243  // Calibrado por você (Água/Molhado = Valor Baixo)
// Limite base de 50% (Valor ADC = 301.5)
#define LIMITE_SOLO_SECO ( (VALOR_SENSOR_SECO + VALOR_SENSOR_MOLHADO) / 2 ) 

// --- ADICIONADO: Constantes da Lógica Inteligente (Ajuste conforme necessário) ---
#define TEMP_AMBIENTE_QUENTE 30.0  // Acima de 30°C é considerado "quente"
#define TEMP_AMBIENTE_FRIA   18.0  // Abaixo de 18°C é considerado "frio"
#define DELTA_T_ESTRESSE     2.0   // Planta 2.0°C mais quente que o ar = Estresse Hídrico
#define AJUSTE_LIMITE_UMIDADE 30   // O "quanto" ajustar o limite ADC (ex: 301.5 +/- 30)

// --- Outras Constantes ---
#define LIMITE_DISTANCIA_AGUA_BAIXA 25.0 
#define TEMPO_IRRIGACAO 5000 // 5 segundos

// --- Constantes dos Termistores NTC 10k ---
#define TERMISTOR_NOMINAL 10000.0  
#define TEMP_NOMINAL      25.0     
#define COEFICIENTE_B     3950.0   
#define RESISTOR_DIVISOR  10000.0  

void setup() {
  Serial.begin(9600);
  // Mensagem de inicialização atualizada
  Serial.println("Iniciando Sistema de Irrigacao (LOGICA INTELIGENTE) - Prototipo Arduino Mega");

  // Configura pinos dos sensores
  pinMode(PINO_TRIGER, OUTPUT);
  pinMode(PINO_ECHO, INPUT);
  
  // Configura pinos de saída
  pinMode(PINO_RELE_BOMBA, OUTPUT);
  pinMode(LED_IRRIGANDO, OUTPUT);
  pinMode(LED_AGUA_BAIXA, OUTPUT);
  pinMode(LED_SISTEMA_OK, OUTPUT);

  // Garante que a bomba comece DESLIGADA
  digitalWrite(PINO_RELE_BOMBA, BOMBA_DESLIGADA);
  Serial.println("Sistema pronto.");
  Serial.print("Limite base de umidade (seco >): ");
  Serial.println(LIMITE_SOLO_SECO);
}

// =========================================================================
// --- VOID LOOP ATUALIZADO COM A LÓGICA INTELIGENTE ---
// =========================================================================
void loop() {
  // 1. Ler todos os sensores
  int valorUmidade = lerUmidadeSolo();
  float distAgua = lerNivelAgua();
  float tempMicroclima = lerTempNTC(PINO_NTC_MICRO);     // NTC em A1
  float tempAmbiente = lerTempNTC(PINO_NTC_AMBIENTE);   // NTC em A2

  // Converte a leitura da umidade para porcentagem (0-100)
  int umidadePercent = map(valorUmidade, VALOR_SENSOR_SECO, VALOR_SENSOR_MOLHADO, 0, 100);
  umidadePercent = constrain(umidadePercent, 0, 100); 

  // 2. Imprimir status no Monitor Serial
  Serial.print("Umidade: " + String(umidadePercent) + "%");
  Serial.print(" (Raw: " + String(valorUmidade) + ")");
  Serial.print(" | Nivel Agua (Dist): " + String(distAgua) + " cm");
  Serial.print(" | T. Microclima: " + String(tempMicroclima) + " C"); // NTC A1
  Serial.print(" | T. Ambiente: " + String(tempAmbiente) + " C");   // NTC A2
  Serial.println();

  // 3. Lógica Principal de Decisão
  
  // Flags para controle
  bool irrigarAgora = false;
  String motivoIrrigacao = "";

  // --- VERIFICAÇÃO DE SEGURANÇA: Nível da Água ---
  if (distAgua > LIMITE_DISTANCIA_AGUA_BAIXA) {
    // CONDIÇÃO: ÁGUA BAIXA - Para tudo
    Serial.println("ALERTA: Tanque de agua baixo! Irrigacao impossivel.");
    digitalWrite(LED_AGUA_BAIXA, HIGH); // Acende LED Vermelho
    digitalWrite(LED_SISTEMA_OK, LOW);  // Apaga LED Amarelo
    irrigarAgora = false; // Garante que não irrigue
    
  } else {
    // CONDIÇÃO: Nível da água está OK
    digitalWrite(LED_AGUA_BAIXA, LOW); // Apaga LED Vermelho
    digitalWrite(LED_SISTEMA_OK, HIGH); // Acende LED Amarelo (Sistema OK)

    // --- LÓGICA DE IRRIGAÇÃO INTELIGENTE ---

    // Calcula os gatilhos
    float deltaT = tempMicroclima - tempAmbiente;
    bool estresseHidrico = (deltaT > DELTA_T_ESTRESSE);
    bool climaQuente = (tempAmbiente > TEMP_AMBIENTE_QUENTE);
    bool climaFrio = (tempAmbiente < TEMP_AMBIENTE_FRIA);
    
    // Define o limite dinâmico com base na leitura ADC (onde > é mais seco)
    float limiteDinamico = LIMITE_SOLO_SECO; // Base = 301.5

    // GATILHO 1: EMERGÊNCIA (Estresse Hídrico - Planta mais quente que o ar)
    if (estresseHidrico) {
      irrigarAgora = true;
      motivoIrrigacao = "NOTIFICACAO (EMERGENCIA): Estresse Hidrico detectado (Planta > Ar)!";
    
    // GATILHO 2: CLIMA QUENTE (Irriga mais cedo/com mais umidade)
    } else if (climaQuente && (valorUmidade > (limiteDinamico - AJUSTE_LIMITE_UMIDADE))) {
      // Irriga se umidade for > (301.5 - 30) = 271.5 
      irrigarAgora = true;
      motivoIrrigacao = "NOTIFICACAO (PREVENCAO): Ambiente quente, irrigando mais cedo.";

    // GATILHO 3: CLIMA FRIO (Irriga mais tarde/com menos umidade)
    } else if (climaFrio && (valorUmidade > (limiteDinamico + AJUSTE_LIMITE_UMIDADE))) {
      // Irriga se umidade for > (301.5 + 30) = 331.5
      irrigarAgora = true;
      motivoIrrigacao = "NOTIFICACAO (ECONOMIA): Clima frio, irrigando (solo muito seco).";
    
    // GATILHO 4: NORMAL (Clima ameno, usa limite base)
    } else if (!climaQuente && !climaFrio && (valorUmidade > limiteDinamico)) {
      // Irriga se umidade for > 301.5
      irrigarAgora = true;
      motivoIrrigacao = "NOTIFICACAO (NORMAL): Solo seco. Ativando bomba...";
    
    } else {
      // CONDIÇÃO: Solo úmido o suficiente para as condições atuais
      irrigarAgora = false;
    }
  } // Fim da checagem de água

  // 4. Executa a Ação (Irrigar ou Não)
  
  if (irrigarAgora) {
    // CONDIÇÃO: PRECISA IRRIGAR
    Serial.println(motivoIrrigacao);
      
    digitalWrite(PINO_RELE_BOMBA, BOMBA_LIGADA); 
    digitalWrite(LED_IRRIGANDO, HIGH);          // Acende LED Verde
    digitalWrite(LED_SISTEMA_OK, LOW);           // Apaga LED Amarelo (mostra "ocupado")
      
    delay(TEMPO_IRRIGACAO); 
      
    Serial.println("NOTIFICACAO: Irrigacao finalizada.");
    digitalWrite(PINO_RELE_BOMBA, BOMBA_DESLIGADA);
    digitalWrite(LED_IRRIGANDO, LOW);           // Apaga LED Verde
    // O LED Amarelo (Sistema OK) acenderá novamente no início do próximo loop se a água estiver OK
      
  } else {
    // CONDIÇÃO: Não irrigar (ou por solo úmido, ou por água baixa)
    digitalWrite(PINO_RELE_BOMBA, BOMBA_DESLIGADA); 
    digitalWrite(LED_IRRIGANDO, LOW); // Garante que LED verde esteja apagado
    // O estado dos LEDs (Vermelho e Amarelo) já foi definido no bloco de checagem de água
  }

  // Espera 2 segundos antes de fazer a próxima verificação completa
  delay(2000); 
}
// =========================================================================
// --- FIM DO VOID LOOP ATUALIZADO ---
// =========================================================================


// --- Funções Auxiliares (O código original abaixo está perfeito) ---

int lerUmidadeSolo() {
  return analogRead(PINO_UMIDADE_SOLO);
}

float lerNivelAgua() { 
  digitalWrite(PINO_TRIGER, LOW); delayMicroseconds(2);
  digitalWrite(PINO_TRIGER, HIGH); delayMicroseconds(10);
  digitalWrite(PINO_TRIGER, LOW);
  long duracao = pulseIn(PINO_ECHO, HIGH, 30000); 
  float distancia = (duracao * 0.0343) / 2.0;
  if (distancia == 0 || distancia > 400) { return 400; }
  return distancia;
}

// --- MODIFICADO: Função genérica para ler qualquer Termistor NTC ---
// Recebe o pino analógico como argumento
float lerTempNTC(int pino) {
  // Lê o valor analógico (0-1023) do divisor de tensão no pino especificado
  int valorADC = analogRead(pino);
  
  // Evita divisão por zero se o sensor estiver desconectado
  if (valorADC < 1 || valorADC > 1022) return -100; // Retorna valor inválido

  // --- Fórmula do Divisor de Tensão ---
  // Nosso circuito: 5V -> R10k -> Pino Analog -> NTC -> GND
  // A fórmula para R_NTC é: R_NTC = R10k * (valorADC / (1023 - valorADC))
  float resistencia = RESISTOR_DIVISOR * ( (float)valorADC / (1023.0 - (float)valorADC) );
  
  // --- Fórmula Steinhart-Hart (Equação B) ---
  float steinhart;
  steinhart = resistencia / TERMISTOR_NOMINAL;     // (R/R0)
  steinhart = log(steinhart);                      // ln(R/R0)
  steinhart /= COEFICIENTE_B;                      // 1/B * ln(R/R0)
  steinhart += 1.0 / (TEMP_NOMINAL + 273.15);      // + 1/T0
  steinhart = 1.0 / steinhart;                     // Inverte
  steinhart -= 273.15;                             // Converte de Kelvin para Celsius
  
  return steinhart;
}

// REMOVIDO: A função lerTempAmbiente() (do LM35) não é mais necessária
