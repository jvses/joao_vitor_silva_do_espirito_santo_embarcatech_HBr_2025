#ifndef MQTT_COMM
#define MQTT_COMM

#include "lwip/apps/mqtt.h"
#include "lwipopts.h"
#include <string.h>
#include <stdint.h>



// static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status);
void mqtt_setup(const char *client_id, const char *broker_ip, const char *user, const char *pass);
// static void mqtt_pub_request_cb(void *arg, err_t result);
void mqtt_comm_publish(const char *topic, const uint8_t *data, size_t len);

void on_message(char* msg);
void xor_encrypt(const uint8_t *input, uint8_t *output, size_t len, uint8_t key);
void mqtt_subscribe_setup();

#endif