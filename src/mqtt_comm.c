#include "lwip/apps/mqtt.h"
#include "include/mqtt_comm.h"
#include "lwipopts.h"
#include <stdio.h>

// Variáveis globais estáticas para o módulo
static mqtt_client_t *mqtt_client_instance;
static on_connect_cb_t on_connect_callback;

// Callback interno de conexão MQTT
static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status) {
    LWIP_UNUSED_ARG(arg);
    if (status == MQTT_CONNECT_ACCEPTED) {
        printf("Conectado ao broker MQTT com sucesso!\n");
        if (on_connect_callback) {
            on_connect_callback(client); // Chama o callback de sucesso (para fazer o subscribe)
        }
    } else {
        printf("Falha ao conectar ao broker, código: %d\n", status);
    }
}

// Função para configurar e iniciar a conexão MQTT
mqtt_client_t* mqtt_setup_and_get_client(const char *client_id, const char *broker_ip, const char *user, const char *pass, on_connect_cb_t cb) {
    ip_addr_t broker_addr;
    
    if (!ip4addr_aton(broker_ip, &broker_addr)) {
        printf("Erro ao converter o IP do broker.\n");
        return NULL;
    }

    mqtt_client_instance = mqtt_client_new();
    if (mqtt_client_instance == NULL) {
        printf("Falha ao criar o cliente MQTT.\n");
        return NULL;
    }
    
    on_connect_callback = cb; // Armazena o callback para usar na conexão

    struct mqtt_connect_client_info_t ci = {
        .client_id = client_id,
        .client_user = user,
        .client_pass = pass,
        .keep_alive = 60 // Keep alive em segundos
    };

    err_t err = mqtt_client_connect(mqtt_client_instance, &broker_addr, 1883, mqtt_connection_cb, NULL, &ci);

    if (err != ERR_OK) {
        printf("mqtt_client_connect retornou erro: %d\n", err);
        return NULL;
    }
    
    return mqtt_client_instance;
}

// Callback de confirmação de publicação
static void mqtt_pub_request_cb(void *arg, err_t result) {
    if (result == ERR_OK) {
        // printf("Publicação MQTT enviada com sucesso!\n"); // Comentado para não poluir o log
    } else {
        printf("Erro ao publicar via MQTT: %d\n", result);
    }
}

// Função para publicar dados em um tópico MQTT
void mqtt_comm_publish(const char *topic, const uint8_t *data, size_t len) {
    if (mqtt_client_instance == NULL || !mqtt_client_is_connected(mqtt_client_instance)) {
        printf("Não é possível publicar: cliente MQTT não está conectado.\n");
        return;
    }

    err_t status = mqtt_publish(
        mqtt_client_instance,
        topic,
        data,
        len,
        0, // QoS 0
        0, // Não reter
        mqtt_pub_request_cb,
        NULL
    );

    if (status != ERR_OK) {
        printf("mqtt_publish falhou com o erro: %d\n", status);
    }
}