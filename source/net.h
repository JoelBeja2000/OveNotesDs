#ifndef NET_H
#define NET_H

#include <stdbool.h>
#include <stddef.h>

#include <stdint.h>

extern char http_ip[64];
extern char http_port_str[16];
extern char wifi_ssid[33];

bool netInitWifi(void);
void netDisconnect(void);
bool enviarNotaHTTP(const char* ip, int puerto, const uint8_t* datosNota, size_t tamañoDatos);
void netLoadConfig(void);
void netSaveConfig(void);

#endif // NET_H
