// Bibliotecas necessárias
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/apps/mqtt.h"

#include "include/wifi_conn.h"
#include "include/mqtt_comm.h" 
#include "include/xor_cipher.h"

// --- Configurações da Aplicação ---
#define WIFI_SSID       "Eu" 
#define WIFI_PASSWORD   "12345678"  
#define BROKER_IP       "192.168.38.29"       
#define MQTT_USER       "aluno"
#define MQTT_PASS       "senha123"
#define MQTT_CLIENT_ID  "bitdog-subscriber"
#define MQTT_TOPIC      "escola/sala1/temperatura"

// Chave para a cifra XOR
#define XOR_KEY 42

// Variável para armazenar o último timestamp recebido e válido
static uint32_t last_valid_timestamp = 0;

// Callbacks para o recebimento de mensagens MQTT

// Buffer para remontar a mensagem (pode vir em pedaços)
static char received_payload[128];
static int received_len = 0;


/**
 * @brief Callback chamado quando os dados de uma publicação são recebidos.
 * * @param arg Argumento personalizado (não usado).
 * @param data Ponteiro para o fragmento de dados.
 * @param len Comprimento do fragmento de dados.
 * @param flags Sinalizadores indicando se há mais fragmentos.
 */
static void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags) {
    LWIP_UNUSED_ARG(arg);
    
    // Copia o fragmento recebido para o buffer global
    if (received_len + len < sizeof(received_payload)) {
        memcpy(received_payload + received_len, data, len);
        received_len += len;
    }

    // Se for o último fragmento da mensagem 
    if (flags & MQTT_DATA_FLAG_LAST) {
        // Adiciona um terminador nulo para tratar como string
        received_payload[received_len] = '\0';
        
        printf("\n--- Nova Mensagem Recebida (Cifrada) ---\n");
        
        // 1. (Etapa 5) Decifrar a mensagem usando a mesma chave XOR
        char decrypted_payload[128];
        xor_encrypt((uint8_t *)received_payload, (uint8_t *)decrypted_payload, received_len, XOR_KEY);
        decrypted_payload[received_len] = '\0'; // Garantir terminação da string

        printf("Mensagem Decifrada: %s\n", decrypted_payload);

        // 2. (Etapa 6) Fazer o parse do JSON e validar o timestamp
        uint32_t new_timestamp;
        float value;
        
        if (sscanf(decrypted_payload, "{\"valor\":%f,\"ts\":%u}", &value, &new_timestamp) == 2) {
            // Verificação de replay
            if (new_timestamp > last_valid_timestamp) {
                last_valid_timestamp = new_timestamp;
                printf("SUCESSO: Nova leitura processada -> Valor: %.2f (Timestamp: %u)\n", value, new_timestamp);
                //
                // ---> AQUI VOCÊ PROCESSARIA O DADO (ex: acender um LED, mostrar em um display, etc.)
                //
            } else {
                printf("FALHA: Replay de ataque detectado! (ts recebido: %u <= ts último válido: %u)\n", new_timestamp, last_valid_timestamp);
            }
        } else {
            printf("FALHA: Erro ao fazer o parse da mensagem JSON decifrada.\n");
        }
        
        // Reseta o buffer para a próxima mensagem
        received_len = 0;
    }
}

/**
 * @brief Callback chamado quando uma publicação chega em um tópico inscrito.
 * * @param arg Argumento personalizado (não usado).
 * @param topic Tópico no qual a mensagem foi publicada.
 * @param tot_len Comprimento total do payload da mensagem.
 */
static void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len) {
    LWIP_UNUSED_ARG(arg);
    // Zera o buffer para a nova mensagem
    received_len = 0; 
    printf("Mensagem chegando no tópico '%s' com tamanho total de %d bytes.\n", topic, tot_len);
}

/**
 * @brief Callback chamado após uma tentativa de inscrição (subscribe).
 */
static void mqtt_sub_request_cb(void *arg, err_t err) {
    if (err == ERR_OK) {
        printf("Inscrito no tópico '%s' com sucesso!\n", MQTT_TOPIC);
    } else {
        printf("Falha ao se inscrever no tópico, erro: %d\n", err);
    }
}

/**
 * @brief Função chamada quando a conexão MQTT é estabelecida.
 * Aqui, nos inscrevemos no tópico desejado.
 */
static void on_mqtt_connect_subscribe(mqtt_client_t *client) {
    err_t err;
    // Qualidade de Serviço 0 (QoS 0)
    err = mqtt_subscribe(client, MQTT_TOPIC, 0, mqtt_sub_request_cb, NULL);
    if (err != ERR_OK) {
        printf("mqtt_subscribe retornou erro: %d\n", err);
    }
}

int main() {
    stdio_init_all();

    connect_to_wifi(WIFI_SSID, WIFI_PASSWORD);
    
    // Usa a mesma função de setup, mas com ID de cliente diferente
    mqtt_client_t* client = mqtt_setup_and_get_client(MQTT_CLIENT_ID, BROKER_IP, MQTT_USER, MQTT_PASS, on_mqtt_connect_subscribe);

    if (client == NULL) {
        printf("Falha ao inicializar o cliente MQTT. O programa será encerrado.\n");
        return 1;
    }

    // Configura os callbacks para receber mensagens
    mqtt_set_inpub_callback(client, mqtt_incoming_publish_cb, mqtt_incoming_data_cb, NULL);

    printf("Subscriber pronto. Aguardando mensagens...\n");

    // Loop infinito para manter a aplicação rodando e o lwIP processando eventos
    while (true) {
        tight_loop_contents();
    }

    return 0;
}