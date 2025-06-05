#ifndef MQTT_COMM_H
#define MQTT_COMM_H

#include "lwip/apps/mqtt.h"


typedef void (*on_connect_cb_t)(mqtt_client_t *client);

// Funções do módulo
mqtt_client_t* mqtt_setup_and_get_client(const char *client_id, const char *broker_ip, const char *user, const char *pass, on_connect_cb_t on_connect_cb);
void mqtt_comm_publish(const char *topic, const uint8_t *data, size_t len);

#endif