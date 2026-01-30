# Sistema de Ponto Eletrônico IoT (BitDogLab) 🕒🐶

Este projeto é um terminal de registro de ponto inteligente desenvolvido para o kit educacional **BitDogLab** (baseado no Raspberry Pi Pico W).

O sistema utiliza a conectividade Wi-Fi do Pico W para enviar registros de "Entrada" e "Saída" para um servidor remoto, usando os periféricos integrados da placa para interação com o usuário.

## 🚀 Funcionalidades
* **Botão A (GPIO 5):** Registra **Entrada**.
* **Botão B (GPIO 6):** Registra **Saída**.
* **LED RGB:**
  * 🟢 **Verde:** Registro enviado com sucesso.
  * 🔴 **Vermelho:** Erro de conexão ou servidor fora do ar.
  * 🔵 **Azul:** Conectando ao Wi-Fi.
* **Buzzer:** Feedback sonoro (Bip curto para sucesso, Bip longo para erro).
* **OLED (Opcional):** Exibe status da conexão e hora atual.

## 🛠️ Hardware (Kit BitDogLab)
Este projeto roda nativamente na placa **BitDogLab** sem necessidade de fios externos.

### Mapeamento de Pinos (Pinout Padrão)

| Componente na Placa | GPIO (Pico W) | Função no Código |
| :--- | :---: | :--- |
| **Botão A** | **GPIO 5** | Registrar Entrada |
| **Botão B** | **GPIO 6** | Registrar Saída |
| **LED RGB (Verde)** | **GPIO 11** | Status: Sucesso |
| **LED RGB (Azul)** | **GPIO 12** | Status: Wi-Fi/Processando |
| **LED RGB (Vermelho)** | **GPIO 13** | Status: Erro |
| **Buzzer A** | **GPIO 21** | Feedback Sonoro |
| **Display OLED** | **GPIO 14/15** | SDA/SCL (I2C) |

## ⚙️ Configuração do Firmware

No arquivo `main.c`, ajuste suas credenciais Wi-Fi e o endereço do seu servidor:

```c
// Credenciais da Rede
#define WIFI_SSID "NomeDaSuaRede"
#define WIFI_PASS "SenhaDaSuaRede"

// Endereço do Servidor (Backend)
#define API_URL "[http://192.168.0.100:5000/api/ponto](http://192.168.0.100:5000/api/ponto)"
