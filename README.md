# Sistemas-embarcados
Repositório para os projetos feito em aula

#Customizações e Esforços de Desenvolvimento

O desenvolvimento deste projeto exigiu diversas customizações no software base utilizado, especialmente para integrar o módulo de pesagem (HX711 + célula de carga) com o modelo de inteligência artificial responsável pela classificação das frutas e vegetais. A seguir estão descritos os principais esforços de adaptação e desenvolvimento realizados:

## 🔧 1. Integração entre Microcontrolador e Host (Arduino/ESP32 ↔ Host Computer)

- Implementação de uma interface de comunicação serial bidirecional entre o microcontrolador e o computador hospedeiro.

- Criação de um protocolo simples para transmissão dos valores de peso em tempo real, sincronizando a leitura da balança com o momento de captura da imagem.

- Adaptação do código padrão da biblioteca HX711 para incluir filtros de estabilidade (média móvel e rejeição de ruído) a fim de suavizar oscilações no sinal analógico.

## ⚖️ 2. Customização da Calibração da Célula de Carga

- Implementação do método de calibração por dois pontos, com cálculo dinâmico de slope e offset para conversão direta dos valores brutos (Raw_Reading) em gramas.

- Armazenamento dos coeficientes de calibração diretamente no código-fonte, permitindo replicação dos resultados sem reconfiguração manual.

- Adição de rotinas de correção de polaridade e verificação automática de leituras invertidas.

## 🧠 3. Adaptação do Modelo de Visão Computacional

- Treinamento e integração de um modelo YOLO customizado para detecção e classificação de frutas e vegetais.

- Ajuste dos parâmetros de confiança e sobreposição (confidence e IoU thresholds) para minimizar falsos positivos em ambiente real.

- Implementação de um script de inferência em Python que recebe a imagem capturada, executa a detecção e retorna a classe identificada ao microcontrolador.

📷 4. Automação do Fluxo de Captura de Imagens

- Desenvolvimento de uma rotina que dispara automaticamente a captura da imagem sempre que o microcontrolador detecta uma nova variação de peso significativa.

- Sincronização entre o evento de pesagem e a captura visual, garantindo correspondência entre os dados de peso e a imagem do produto.

## 💡 5. Otimizações e Ajustes Experimentais

- Implementação de filtros digitais simples para estabilização da leitura da balança (média móvel e exponencial).

- Parametrização do sistema para operar de forma estável entre 500 e 800 lux, conforme identificado nos testes de iluminação.

- Ajuste do threshold ótimo de confiança (≈ 0.8) no modelo YOLO, com base na curva F1 × Threshold obtida experimentalmente.

- Criação de scripts auxiliares para análise estatística (tempo médio de pesagem, desvio padrão e erro percentual).
