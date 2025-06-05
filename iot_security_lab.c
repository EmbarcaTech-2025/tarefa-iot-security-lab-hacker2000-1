// Bibliotecas necessárias
#include <stdio.h>                // Para a função sprintf()
#include <string.h>               // Para funções de string como strlen()
#include "pico/stdlib.h"          // Biblioteca padrão do Pico (GPIO, tempo, etc.)
#include "pico/cyw43_arch.h"      // Driver WiFi para Pico W
#include "pico/util/datetime.h"   // Para obter o tempo (timestamp)

#include "include/wifi_conn.h"    // Funções personalizadas de conexão WiFi
#include "include/mqtt_comm.h"    // Funções personalizadas para MQTT
#include "include/xor_cipher.h"   // Funções de cifra XOR

// --- Configurações da Aplicação ---
#define WIFI_SSID       "Eu" 
#define WIFI_PASSWORD   "12345678"  
#define BROKER_IP "192.168.38.29"       
#define MQTT_USER       "aluno"
#define MQTT_PASS       "senha123"
#define MQTT_CLIENT_ID  "bitdog-publisher"
#define MQTT_TOPIC      "escola/sala1/temperatura"

// Chave para a cifra XOR (deve ser a mesma no publisher e no subscriber)
#define XOR_KEY 42

int main() {
    // Inicializa todas as interfaces de I/O padrão (USB serial, etc.)
    stdio_init_all();
    
    // Conecta à rede WiFi
    connect_to_wifi(WIFI_SSID, WIFI_PASSWORD);

    // Configura o cliente MQTT usando a nova função.
    // O Publisher não precisa de um callback especial na conexão, então passamos NULL.
    mqtt_setup_and_get_client(MQTT_CLIENT_ID, BROKER_IP, MQTT_USER, MQTT_PASS, NULL);

    // Buffer para construir a mensagem JSON
    char payload[128];
    // Buffer para a mensagem criptografada
    uint8_t encrypted_payload[128];

    // Loop principal do programa
    while (true) {
        // 1. Obter o timestamp atual (segundos desde a inicialização do Pico)
        uint64_t current_timestamp = to_us_since_boot(get_absolute_time()) / 1000000;

        // 2. Criar o payload no formato JSON com valor e timestamp
        sprintf(payload, "{\"valor\":26.5,\"ts\":%llu}", current_timestamp);
        
        printf("Publicando (texto claro): %s\n", payload);

        // 3. (Etapa 5) Criptografar a mensagem JSON usando a cifra XOR
        size_t payload_len = strlen(payload);
        xor_encrypt((uint8_t *)payload, encrypted_payload, payload_len, XOR_KEY);

        // 4. (Etapa 6) Publicar a mensagem CRIPTOGRAFADA
        mqtt_comm_publish(MQTT_TOPIC, encrypted_payload, payload_len);
        
        // Aguarda 5 segundos antes da próxima publicação
        sleep_ms(5000);
    }
    return 0;
}