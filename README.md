# 📡 WiFi Scanner com ESP32

Este projeto realiza varreduras periódicas de redes Wi-Fi com um ESP32, calcula o **SNR (Signal-to-Noise Ratio)** de cada rede detectada e salva os resultados em um arquivo **wifi_log.txt** dentro da partição **SPIFFS**.

O objetivo é analisar a qualidade de redes Wi-Fi próximas, registrando métricas úteis como RSSI, canal, segurança, ruído estimado e SNR.

---

## ✨ Funcionalidades

* Realiza **scan ativo** de redes Wi-Fi.
* Obtém:

  * SSID
  * RSSI
  * Canal
  * Tipo de criptografia
  * Ruído estimado (noise floor)
  * SNR calculado
* Salva tudo em:

  ```
  /spiffs/wifi_log.txt
  ```
* Log em formato CSV:

  ```
  timestamp_ms,ssid,rssi,canal,auth,noise,snr
  ```
* Imprime os dados no monitor serial.
* Atualiza automaticamente a cada 5 segundos.

---

## 📁 Estrutura do arquivo de log (exemplo)

```
1732805123456,MinhaRede,-60,6,WPA2,-95,35
1732805128456,RedeVizinho,-72,1,WPA3,-95,23
```

Onde:

* **timestamp_ms** — tempo desde o boot
* **rssi** — intensidade do sinal
* **noise** — ruído estimado
* **snr** — qualidade real do link

---

## 📂 Local onde o arquivo é salvo

O arquivo é gravado dentro da flash, no caminho:

```
/spiffs/wifi_log.txt
```

Para ler o conteúdo, você pode:

* Imprimir pelo Serial (adicionando função de leitura)
* Usar ferramentas do ESP-IDF para exportar SPIFFS
* Criar um endpoint web (opcional)

## ▶️ Como executar

1. Instale o **ESP-IDF** (v5.x recomendado).
2. Configure o alvo:

   ```
   idf.py set-target esp32
   ```
3. Compile:

   ```
   idf.py build
   ```
4. Grave no ESP32:

   ```
   idf.py flash monitor
   ```

Ao iniciar, o programa irá:

* Montar o SPIFFS
* Inicializar Wi-Fi em modo STA
* Realizar scans periódicos
* Calcular SNR
* Registrar no arquivo


## 🧮 Como o SNR é calculado

O ESP32 não fornece ruído nativo, então usamos um noise floor estimado:

```
noise = -95 dBm
SNR = RSSI - noise
```

## 📊 Interpretação do SNR

| SNR (dB) | Qualidade |
| -------- | --------- |
| ≥ 40 dB  | Excelente |
| 25–40 dB | Muito bom |
| 15–25 dB | OK        |
| 10–15 dB | Ruim      |
| < 10 dB  | Péssimo   |


