/*
 * PROTÓTIPO DE IRRIGAÇÃO AUTOMÁTICA (COM 2 Termistores NTC)
 * Placa: Arduino Mega
 * Sensores: SENSOR CAPACITIVO (Umidade), HC-SR04 (Nível), 
 * NTC (Microclima em A1), NTC (Ambiente em A2)
 * Atuador: Módulo Relé + Bomba 5V
 */

// --- Bibliotecas ---
#include <math.h> // Necessário para a função log() do termistor

// --- Pinos dos Sensores ---
#define PINO_UMIDADE_SOLO   A0  // Sensor Capacitivo
#define PINO_NTC_MICRO      A1  // MODIFICADO: NTC para Microclima
#define PINO_NTC_AMBIENTE   A2  // MODIFICADO: NTC para Temp. Ambiente
#define PINO_TRIGER         7   // Sensor Ultrassônico
#define PINO_ECHO           6   // Sensor Ultrassônico

// --- Pinos dos Atuadores e LEDs ---
#define PINO_RELE_BOMBA     24  // Módulo Relé (Pino 'IN')
#define LED_IRRIGANDO       9   // LED Verde
#define LED_AGUA_BAIXA      10  // LED Vermelho
#define LED_SISTEMA_OK      8   // LED Azul

// --- LÓGICA DO RELÉ ---
#define BOMBA_LIGADA    LOW
#define BOMBA_DESLIGADA HIGH

// --- Constantes de Calibração (SENSOR CAPACITIVO) ---
#define VALOR_SENSOR_SECO 360     // Calibrado por você
#define VALOR_SENSOR_MOLHADO 243  // Calibrado por você
#define LIMITE_SOLO_SECO ( (VALOR_SENSOR_SECO + VALOR_SENSOR_MOLHADO) / 2 )

// --- Outras Constantes ---
#define LIMITE_DISTANCIA_AGUA_BAIXA 25.0 
#define TEMPO_IRRIGACAO 5000 // 5 segundos

// --- Constantes dos Termistores NTC 10k ---
#define TERMISTOR_NOMINAL 10000.0  // Resistência a 25°C
#define TEMP_NOMINAL      25.0     // Temp. nominal em Celsius
#define COEFICIENTE_B     3950.0   // Coeficiente Beta (B)
#define RESISTOR_DIVISOR  10000.0  // O resistor de 10k que está em série com cada NTC

void setup() {
  Serial.begin(9600);
  Serial.println("Iniciando Sistema de Irrigacao (CALIBRADO, 2 NTCs) - Prototipo Arduino Mega");

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
  Serial.print("Limite de umidade (seco >): ");
  Serial.println(LIMITE_SOLO_SECO);
}

void loop() {
  // 1. Ler todos os sensores
  int valorUmidade = lerUmidadeSolo();
  float distAgua = lerNivelAgua();
  // MODIFICADO: Chama a função genérica para ambos os pinos
  float tempMicroclima = lerTempNTC(PINO_NTC_MICRO);     // NTC em A1
  float tempAmbiente = lerTempNTC(PINO_NTC_AMBIENTE);   // NTC em A2

  // Converte a leitura da umidade para porcentagem (0-100)
  int umidadePercent = map(valorUmidade, VALOR_SENSOR_SECO, VALOR_SENSOR_MOLHADO, 0, 100);
  umidadePercent = constrain(umidadePercent, 0, 100); 

  // 2. Imprimir status no Monitor Serial (MODIFICADO)
  Serial.print("Umidade: " + String(umidadePercent) + "%");
  Serial.print(" (Raw: " + String(valorUmidade) + ")");
  Serial.print(" | Nivel Agua (Dist): " + String(distAgua) + " cm");
  Serial.print(" | T. Microclima: " + String(tempMicroclima) + " C"); // NTC A1
  Serial.print(" | T. Ambiente: " + String(tempAmbiente) + " C");   // NTC A2
  Serial.println();

  // 3. Lógica Principal de Decisão (Sem alteração)

  // --- VERIFICAÇÃO DE SEGURANÇA: Nível da Água ---
  if (distAgua > LIMITE_DISTANCIA_AGUA_BAIXA) {
    // CONDIÇÃO: ÁGUA BAIXA
    Serial.println("ALERTA: Tanque de agua baixo! Irrigacao impossivel.");
    digitalWrite(PINO_RELE_BOMBA, BOMBA_DESLIGADA); 
    digitalWrite(LED_AGUA_BAIXA, HIGH); 
    digitalWrite(LED_SISTEMA_OK, LOW);
    digitalWrite(LED_IRRIGANDO, LOW);
    
  } else {
    // CONDIÇÃO: Nível da água está OK
    digitalWrite(LED_AGUA_BAIXA, LOW); 
    digitalWrite(LED_SISTEMA_OK, HIGH); 

    // --- VERIFICAÇÃO DE IRRIGAÇÃO ---
    // Lógica invertida: "seco" é um valor MAIOR que o limite.
    if (valorUmidade > LIMITE_SOLO_SECO) {
      // CONDIÇÃO: Solo está seco. PRECISA IRRIGAR.
      Serial.println("NOTIFICACAO: Solo seco. Ativando bomba por 5 segundos...");
      
      digitalWrite(PINO_RELE_BOMBA, BOMBA_LIGADA); 
      digitalWrite(LED_IRRIGANDO, HIGH);          
      
      delay(TEMPO_IRRIGACAO); 
      
      Serial.println("NOTIFICACAO: Irrigacao finalizada.");
      digitalWrite(PINO_RELE_BOMBA, BOMBA_DESLIGADA);
      digitalWrite(LED_IRRIGANDO, LOW);
      
    } else {
      // CONDIÇÃO: Solo está úmido.
      digitalWrite(PINO_RELE_BOMBA, BOMBA_DESLIGADA); 
      digitalWrite(LED_IRRIGANDO, LOW);
    }
  }

  // Espera 2 segundos antes de fazer a próxima verificação completa
  delay(2000); 
}

// --- Funções Auxiliares ---

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

// REMOVIDO: A função lerTempAmbiente() não é mais necessária
