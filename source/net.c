#include "net.h"
#include <nds.h>
#include <dswifi9.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <errno.h>
#include "log.h"

char http_ip[64] = "192.168.1.132";
char http_port_str[16] = "3000";
char wifi_ssid[33] = "Auto";

static bool wifi_inicializado = false;

void netDisconnect(void) {
    if (!wifi_inicializado) {
        printf("[NET] Wi-Fi no inicializado, no es necesario desconectar.\n");
        return;
    }
    printf("[NET] Desconectando Wi-Fi...\n");
    Wifi_DisconnectAP();
    // Esperar un momento a que se desconecte y estabilice el hardware
    for (int i = 0; i < 30; i++) {
        swiWaitForVBlank();
    }
}

void netLoadConfig(void) {
    FILE* f = fopen("sd:/ovenotes_config.txt", "r");
    if (f == NULL) {
        f = fopen("fat:/ovenotes_config.txt", "r");
    }
    if (f == NULL) {
        f = fopen("ovenotes_config.txt", "r");
    }
    
    if (f != NULL) {
        char line[128];
        // Read IP
        if (fgets(line, sizeof(line), f)) {
            line[strcspn(line, "\r\n")] = 0;
            if (line[0] != '\0') {
                strncpy(http_ip, line, sizeof(http_ip) - 1);
                http_ip[sizeof(http_ip) - 1] = '\0';
            }
        }
        // Read Port
        if (fgets(line, sizeof(line), f)) {
            line[strcspn(line, "\r\n")] = 0;
            if (line[0] != '\0') {
                strncpy(http_port_str, line, sizeof(http_port_str) - 1);
                http_port_str[sizeof(http_port_str) - 1] = '\0';
            }
        }
        // Read SSID
        if (fgets(line, sizeof(line), f)) {
            line[strcspn(line, "\r\n")] = 0;
            if (line[0] != '\0') {
                strncpy(wifi_ssid, line, sizeof(wifi_ssid) - 1);
                wifi_ssid[sizeof(wifi_ssid) - 1] = '\0';
            }
        }
        fclose(f);
        printf("[NET] Config cargada: IP=%s, Port=%s, SSID=%s\n", http_ip, http_port_str, wifi_ssid);
    } else {
        printf("[NET] No se encontro archivo de config. Usando valores por defecto.\n");
    }
}

void netSaveConfig(void) {
    FILE* f = fopen("sd:/ovenotes_config.txt", "w");
    if (f == NULL) {
        f = fopen("fat:/ovenotes_config.txt", "w");
    }
    if (f == NULL) {
        f = fopen("ovenotes_config.txt", "w");
    }
    
    if (f != NULL) {
        fprintf(f, "%s\n", http_ip);
        fprintf(f, "%s\n", http_port_str);
        fprintf(f, "%s\n", wifi_ssid);
        fclose(f);
        printf("[NET] Config guardada en SD.\n");
    } else {
        printf("[NET] Error al guardar config en SD.\n");
    }
}

bool netInitWifi(void) {
    if (!wifi_inicializado) {
        printf("[NET] Inicializando hardware Wi-Fi...\n");
        // WIFI_ATTEMPT_DSI_MODE: usar Wi-Fi DSi (WPA2) si esta disponible,
        // si no, cae a modo DS clasico (WEP/Open)
        if (!Wifi_InitDefault(WIFI_ATTEMPT_DSI_MODE)) {
            printf("[NET] Error: fallo al inicializar hardware Wi-Fi\n");
            return false;
        }
        printf("[NET] Hardware inicializado OK\n");
        wifi_inicializado = true;
    }
    
    if (Wifi_AssocStatus() == ASSOCSTATUS_ASSOCIATED) {
        u32 ip = Wifi_GetIP();
        printf("[NET] Wi-Fi ya esta conectado! IP: %lu.%lu.%lu.%lu\n",
               (ip) & 0xFF, (ip >> 8) & 0xFF, 
               (ip >> 16) & 0xFF, (ip >> 24) & 0xFF);
        return true;
    }
    
    int intento = 1;
    const int max_intentos = 3;
    
    while (intento <= max_intentos) {
        printf("[NET] Conectando (Intento %d de %d)...\n", intento, max_intentos);
        
        if (wifi_ssid[0] == '\0' || strcmp(wifi_ssid, "Auto") == 0 || strcmp(wifi_ssid, "auto") == 0 || strcmp(wifi_ssid, "AUTO") == 0) {
            printf("[NET] Conectando automaticamente (AutoConnect)...\n");
            Wifi_AutoConnect();
        } else {
            printf("[NET] Conectando al perfil SSID: %s...\n", wifi_ssid);
            Wifi_AccessPoint ap;
            memset(&ap, 0, sizeof(Wifi_AccessPoint));
            int ssid_len = strlen(wifi_ssid);
            if (ssid_len > 32) ssid_len = 32;
            memcpy(ap.ssid, wifi_ssid, ssid_len);
            ap.ssid[ssid_len] = '\0';
            ap.ssid_len = ssid_len;
            Wifi_ConnectWfcAP(&ap);
        }
        
        // Esperar hasta 30 segundos en cada intento
        int timeout_frames = 60 * 30;
        bool conectado = false;
        
        while (timeout_frames > 0) {
            int status = Wifi_AssocStatus();
            
            if (status == ASSOCSTATUS_ASSOCIATED) {
                u32 ip = Wifi_GetIP();
                printf("[NET] Conectado! IP: %lu.%lu.%lu.%lu\n",
                       (ip) & 0xFF, (ip >> 8) & 0xFF, 
                       (ip >> 16) & 0xFF, (ip >> 24) & 0xFF);
                conectado = true;
                break;
            }
            if (status == ASSOCSTATUS_CANNOTCONNECT) {
                printf("[NET] Error de conexion/asociacion (status=%d)\n", status);
                break;
            }
            
            swiWaitForVBlank();
            timeout_frames--;
            
            if (timeout_frames % 60 == 0) {
                printf("[NET] Esperando... (status=%d, %ds)\n", status, timeout_frames / 60);
            }
        }
        
        if (conectado) {
            return true;
        }
        
        printf("[NET] Intento %d fallido. Reintentando...\n", intento);
        Wifi_DisconnectAP();
        
        // Esperar 2 segundos antes de reintentar para dar tiempo al chip a estabilizarse
        for (int i = 0; i < 120; i++) {
            swiWaitForVBlank();
        }
        intento++;
    }
    
    printf("[NET] Error: no se pudo conectar tras %d intentos\n", max_intentos);
    return false;
}

#ifndef closesocket
#define closesocket close
#endif

// Envia datos con timeout usando select y sockets no-bloqueantes
static int send_all_timeout(int sock, const char* data, int len, int timeout_sec) {
    int total_sent = 0;
    
    // Configurar socket en modo no-bloqueante
    u32 on = 1;
    if (ioctl(sock, FIONBIO, &on) < 0) {
        printf("[NET] Error al configurar no-bloqueante en send\n");
        return -1;
    }
    
    while (total_sent < len) {
        fd_set write_fds;
        FD_ZERO(&write_fds);
        FD_SET(sock, &write_fds);
        
        struct timeval tv;
        tv.tv_sec = timeout_sec;
        tv.tv_usec = 0;
        
        int sel = select(sock + 1, NULL, &write_fds, NULL, &tv);
        if (sel < 0) {
            printf("[NET] Error en select de escritura\n");
            return -1;
        } else if (sel == 0) {
            printf("[NET] Timeout en select de escritura (%ds)\n", timeout_sec);
            return -1;
        }
        
        int to_send = len - total_sent;
        if (to_send > 1024) to_send = 1024;
        
        int r = send(sock, data + total_sent, to_send, 0);
        if (r < 0) {
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                swiWaitForVBlank();
                continue;
            }
            printf("[NET] Error de transmision send: %d\n", errno);
            return -1;
        }
        if (r == 0) {
            printf("[NET] Socket cerrado por el host remoto\n");
            return -1;
        }
        total_sent += r;
    }
    
    // Restaurar socket a modo bloqueante
    u32 off = 0;
    ioctl(sock, FIONBIO, &off);
    return total_sent;
}

// Recibe datos con timeout usando select
static int recv_timeout(int sock, char* buf, int max_len, int timeout_sec) {
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(sock, &read_fds);
    
    struct timeval tv;
    tv.tv_sec = timeout_sec;
    tv.tv_usec = 0;
    
    int sel = select(sock + 1, &read_fds, NULL, NULL, &tv);
    if (sel < 0) {
        printf("[NET] Error en select de lectura\n");
        return -1;
    } else if (sel == 0) {
        printf("[NET] Timeout esperando respuesta de red (%ds)\n", timeout_sec);
        return -1;
    }
    
    // Configurar no-bloqueante temporalmente
    u32 on = 1;
    ioctl(sock, FIONBIO, &on);
    
    int r = recv(sock, buf, max_len, 0);
    
    u32 off = 0;
    ioctl(sock, FIONBIO, &off);
    return r;
}

bool enviarNotaHTTP(const char* ip, int puerto, const uint8_t* datosNota, size_t tamañoDatos) {
    printf("[NET] Iniciando enviarNotaHTTP a %s:%d (%u bytes)\n", ip, puerto, (unsigned int)tamañoDatos);
    if (Wifi_AssocStatus() != ASSOCSTATUS_ASSOCIATED) {
        printf("[NET] Error: Wifi no asociado (Status: %d)\n", Wifi_AssocStatus());
        return false;
    }

    printf("[NET] Creando socket TCP...\n");
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        printf("[NET] Error: no se pudo crear el socket\n");
        return false;
    }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(puerto);
    if (inet_aton(ip, &serv_addr.sin_addr) == 0) {
        printf("[NET] Error: IP invalida (%s)\n", ip);
        closesocket(sock);
        return false;
    }

    printf("[NET] Conectando a %s:%d...\n", ip, puerto);
    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("[NET] Error: conexion fallida\n");
        closesocket(sock);
        return false;
    }
    printf("[NET] Conectado! Preparando cabeceras...\n");

    // Construir la cabecera HTTP POST
    char header[512];
    int header_len = snprintf(header, sizeof(header),
                              "POST /api/nueva-nota HTTP/1.1\r\n"
                              "Host: %s:%d\r\n"
                              "Content-Type: application/octet-stream\r\n"
                              "Content-Length: %u\r\n"
                              "Connection: close\r\n\r\n",
                              ip, puerto, (unsigned int)tamañoDatos);

    if (header_len < 0 || header_len >= (int)sizeof(header)) {
        printf("[NET] Error: cabecera demasiado larga\n");
        closesocket(sock);
        return false;
    }

    printf("[NET] Enviando cabecera HTTP...\n");
    if (send_all_timeout(sock, header, header_len, 5) < 0) {
        printf("[NET] Error: fallo al enviar cabecera (timeout)\n");
        closesocket(sock);
        return false;
    }
    printf("[NET] Cabecera enviada. Enviando datos binarios...\n");

    // Enviar cuerpo (los datos binarios de la nota)
    if (send_all_timeout(sock, (const char*)datosNota, tamañoDatos, 10) < 0) {
        printf("[NET] Error: fallo al enviar el cuerpo (timeout)\n");
        closesocket(sock);
        return false;
    }
    printf("[NET] Cuerpo de nota enviado. Esperando respuesta...\n");

    // Leer respuesta para verificar el estado HTTP
    char response[256];
    int recv_len = recv_timeout(sock, response, sizeof(response) - 1, 5);
    closesocket(sock);

    if (recv_len > 0) {
        response[recv_len] = '\0';
        printf("[NET] Respuesta recibida de %d bytes. Comprobando status...\n", recv_len);
        if (strstr(response, "HTTP/1.1 200") != NULL || strstr(response, "HTTP/1.1 201") != NULL) {
            printf("[NET] Envio HTTP exitoso!\n");
            return true;
        }
        printf("[NET] Error: Status HTTP invalido: %.40s\n", response);
    } else {
        printf("[NET] Error: Sin respuesta HTTP o timeout (recv_len=%d)\n", recv_len);
    }

    return false;
}
