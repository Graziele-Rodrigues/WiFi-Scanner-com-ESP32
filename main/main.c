#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include "esp_timer.h"

static const char *TAG = "WIFI_SCAN_SNR";

static const char* get_auth_mode_name(wifi_auth_mode_t auth) {
    switch (auth) {
        case WIFI_AUTH_OPEN: return "OPEN";
        case WIFI_AUTH_WEP: return "WEP";
        case WIFI_AUTH_WPA_PSK: return "WPA";
        case WIFI_AUTH_WPA2_PSK: return "WPA2";
        case WIFI_AUTH_WPA_WPA2_PSK: return "WPA/WPA2";
        case WIFI_AUTH_WPA3_PSK: return "WPA3";
        case WIFI_AUTH_WPA2_WPA3_PSK: return "WPA2/WPA3";
        case WIFI_AUTH_ENTERPRISE: return "WPA2-ENT";
        default: return "UNKNOWN";
    }
}

void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());

    /* Montar SPIFFS */
    esp_vfs_spiffs_conf_t spiffs_conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true
    };

    esp_err_t spiffs_ret = esp_vfs_spiffs_register(&spiffs_conf);
    if (spiffs_ret != ESP_OK) {
        ESP_LOGE(TAG, "Falha ao montar SPIFFS: %s", esp_err_to_name(spiffs_ret));
    } else {
        size_t total = 0, used = 0;
        esp_spiffs_info(NULL, &total, &used);
        ESP_LOGI(TAG, "SPIFFS montado. Total: %d bytes, Usado: %d bytes", total, used);
    }

    /* Inicializar WiFi */
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    vTaskDelay(pdMS_TO_TICKS(500));

    wifi_scan_config_t scan_config = {
        .ssid = NULL,
        .bssid = NULL,
        .channel = 0,
        .show_hidden = true,
        .scan_type = WIFI_SCAN_TYPE_ACTIVE,
        .scan_time = {
            .active = {
                .min = 100,
                .max = 300
            }
        }
    };

    while (1) {
        ESP_LOGI(TAG, "Iniciando varredura WiFi...");
        ESP_ERROR_CHECK(esp_wifi_scan_start(&scan_config, true));

        uint16_t ap_num = 0;
        ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_num));

        wifi_ap_record_t *ap_list = malloc(ap_num * sizeof(wifi_ap_record_t));
        ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&ap_num, ap_list));

        ESP_LOGI(TAG, "Total de redes encontradas: %u\n", ap_num);

        if (ap_num > 0) {
            FILE *f = fopen("/spiffs/wifi_log.txt", "a");
            if (f == NULL) {
                ESP_LOGW(TAG, "Falha ao abrir arquivo de log");
            }

            for (int i = 0; i < ap_num; i++) {

                wifi_ap_record_t *ap = &ap_list[i];

                // Noise floor estimado (não disponível nativamente)
                int8_t noise = -95;   // típico entre -90 e -98 dBm
                int snr = ap->rssi - noise;

                ESP_LOGI(TAG,
                    "\nRede %d\n"
                    "  SSID: %s\n"
                    "  RSSI: %d dBm\n"
                    "  Canal: %d\n"
                    "  Criptografia: %s\n"
                    "  Noise (estimado): %d dBm\n"
                    "  SNR: %d dB\n",
                    i + 1,
                    (char *)ap->ssid,
                    ap->rssi,
                    ap->primary,
                    get_auth_mode_name(ap->authmode),
                    noise,
                    snr
                );

                if (f != NULL) {
                    int64_t timestamp_ms = esp_timer_get_time() / 1000;
                    fprintf(
                        f,
                        "%lld,%s,%d,%d,%s,%d,%d\n",
                        (long long)timestamp_ms,
                        (char *)ap->ssid,
                        ap->rssi,
                        ap->primary,
                        get_auth_mode_name(ap->authmode),
                        noise,
                        snr
                    );
                    fflush(f);
                }
            }

            if (f != NULL) fclose(f);
        }

        free(ap_list);

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
