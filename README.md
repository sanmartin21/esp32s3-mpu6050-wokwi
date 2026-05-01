# Atividade Avaliativa Pratica - Leitura de Sensor

## 1. Objetivo

Desenvolver uma aplicacao embarcada completa utilizando `ESP-IDF` no `VS Code`, com simulacao no `Wokwi`, para ler dados de um sensor e exibir os resultados no monitor serial.

Neste projeto, foi utilizado o sensor `MPU6050`, realizando a leitura de:

- aceleracao nos eixos `X`, `Y` e `Z`
- velocidade angular do giroscopio nos eixos `X`, `Y` e `Z`

## 2. Escolhas do Projeto

### Sensor escolhido: `MPU6050`

O `MPU6050` foi escolhido porque:

- esta disponivel no `Wokwi`
- atende diretamente ao requisito da atividade
- permite demonstrar leitura de dois conjuntos de dados no mesmo componente: acelerometro e giroscopio

### Placa escolhida: `ESP32-S3`

Foi utilizado o `ESP32-S3 DevKitC-1` porque:

- possui suporte completo no `ESP-IDF`
- esta disponivel no `Wokwi`
- oferece uma integracao direta com o fluxo de build e simulacao no `VS Code`

### Comunicacao escolhida: `I2C`

A comunicacao com o `MPU6050` foi implementada via `I2C`, porque:

- e a interface nativa do sensor
- utiliza poucos pinos
- e bem suportada pelo `ESP-IDF`

### Estrategia de implementacao

Em vez de depender de uma biblioteca externa especifica para o `MPU6050`, a leitura foi implementada diretamente em `C` usando a API de `I2C` do `ESP-IDF`. Essa decisao foi tomada para:

- deixar a inicializacao e a leitura explicitas
- demonstrar dominio da comunicacao com registradores
- manter o projeto simples, controlado e facil de explicar na entrega

## 3. Requisitos da Atividade e Como Foram Atendidos

| Requisito | Situacao |
| --- | --- |
| Configuracao do `ESP-IDF` | Atendido |
| Configuracao da simulacao no `Wokwi` | Atendido |
| Escolha de sensor disponivel no `Wokwi` | Atendido com `MPU6050` |
| Montagem correta do circuito | Atendido |
| Inicializacao correta do sensor | Atendido |
| Implementacao em `C` | Atendido |
| Leitura no monitor serial | Atendido |
| Codigo compilando sem erros | Atendido |

## 4. Montagem do Circuito no Wokwi

Arquivo principal do circuito:

- [diagram.json](./diagram.json)

Componentes utilizados:

- `ESP32-S3 DevKitC-1`
- `MPU6050`

Ligacoes realizadas:

| Pino do MPU6050 | Pino do ESP32-S3 |
| --- | --- |
| `VCC` | `3V3` |
| `GND` | `GND` |
| `SDA` | `GPIO 8` |
| `SCL` | `GPIO 9` |

## 5. Estrutura do Projeto

Os arquivos mais importantes do projeto sao:

- [CMakeLists.txt](./CMakeLists.txt): definicao do projeto `ESP-IDF`
- [wokwi.toml](./wokwi.toml): integracao da simulacao com os binarios gerados
- [diagram.json](./diagram.json): circuito da simulacao
- [main/mpu6050_reader_main.c](./main/mpu6050_reader_main.c): firmware principal da leitura do sensor
- [main/CMakeLists.txt](./main/CMakeLists.txt): configuracao do componente `main`

## 6. Funcionamento do Firmware

O fluxo principal da aplicacao e:

1. inicializar o barramento `I2C`
2. detectar o sensor no endereco `0x68`
3. ler o registrador `WHO_AM_I`
4. acordar o `MPU6050` pelo registrador `PWR_MGMT_1`
5. ler os registradores do acelerometro
6. ler os registradores do giroscopio
7. converter os dados brutos para `g` no acelerometro e `dps` no giroscopio
8. imprimir os valores no monitor serial a cada `1000 ms`

### Parametros definidos no codigo

- Endereco `I2C` do sensor: `0x68`
- `SDA`: `GPIO 8`
- `SCL`: `GPIO 9`
- Clock `I2C`: `400000 Hz`
- Intervalo entre leituras: `1000 ms`

## 7. Arquitetura de Leitura

As principais decisoes de estrutura no codigo foram:

- criar funcoes separadas para escrita e leitura de registradores
- encapsular a leitura de tres eixos em uma estrutura `axis3_t`
- validar o sensor antes de iniciar a leitura continua
- manter logs objetivos no serial para facilitar demonstracao e depuracao

Isso deixou o projeto mais organizado e mais facil de apresentar na atividade.

## 8. Como Executar

### Compilacao no VS Code

1. abrir o projeto no `VS Code`
2. garantir que o target esteja como `esp32s3`
3. executar `ESP-IDF: Build your project`

### Simulacao no Wokwi

1. abrir a paleta de comandos
2. executar `Wokwi: Start Simulator`
3. acompanhar a saida no monitor serial

## 9. Exemplo de Saida no Monitor Serial

Saida obtida durante a simulacao:

```text
I (32) main_task: Calling app_main()
Iniciando leitura do MPU6050...
I (132) mpu6050_reader: MPU6050 inicializado com sucesso. WHO_AM_I=0x68
I (132) mpu6050_reader: Accel[g] X=0.00 Y=0.00 Z=1.00 | Gyro[dps] X=0.00 Y=0.00 Z=0.00
I (1132) mpu6050_reader: Accel[g] X=0.00 Y=0.00 Z=1.00 | Gyro[dps] X=0.00 Y=0.00 Z=0.00
I (2132) mpu6050_reader: Accel[g] X=0.00 Y=0.00 Z=1.00 | Gyro[dps] X=0.00 Y=0.00 Z=0.00
```

Esses valores sao coerentes com o sensor em repouso na simulacao:

- aceleracao em `Z` proxima de `1 g`
- giroscopio proximo de `0 dps`

## 10. Repositorio

Repositorio da entrega:

- `https://github.com/sanmartin21/esp32s3-mpu6050-wokwi`

## 11. Conclusao

O projeto atende a proposta da atividade, pois realiza:

- configuracao do ambiente `ESP-IDF`
- simulacao no `Wokwi`
- leitura de sensor embarcado
- inicializacao correta do `MPU6050`
- exibicao continua dos dados no monitor serial

Alem disso, as escolhas de estrutura adotadas no codigo priorizaram clareza, controle direto do hardware e facilidade de demonstracao.
