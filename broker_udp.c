// broker_udp.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8081 // Usamos un puerto diferente para no chocar con el de TCP
#define MAX_SUBS 100
#define BUFFER_SIZE 1024

// La estructura de suscripción ahora guarda la dirección completa del suscriptor
typedef struct {
    char topic[50];
    struct sockaddr_in subscriber_addr;
} Subscription;

int main() {
    int sockfd;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in servaddr, cliaddr;
    socklen_t len;
    int n;

    Subscription subscriptions[MAX_SUBS];
    int sub_count = 0;

    // Crear el socket UDP
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Fallo al crear socket");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    // Configurar la dirección del servidor
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    // Vincular el socket a la dirección y puerto
    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("Fallo en bind");
        exit(EXIT_FAILURE);
    }

    printf("Broker UDP escuchando en el puerto %d\n", PORT);

    // Bucle principal para recibir y procesar datagramas
    while (1) {
        len = sizeof(cliaddr);
        // recvfrom es una llamada bloqueante que espera un datagrama
        n = recvfrom(sockfd, (char *)buffer, BUFFER_SIZE, MSG_WAITALL, (struct sockaddr *) &cliaddr, &len);
        buffer[n] = '\0';

        char temp_buffer[BUFFER_SIZE];
        strcpy(temp_buffer, buffer);
        char *command = strtok(temp_buffer, ":");

        if (command != NULL && strcmp(command, "SUBSCRIBE") == 0) {
            char *topic = strtok(NULL, "");
            if (topic != NULL && sub_count < MAX_SUBS) {
                strcpy(subscriptions[sub_count].topic, topic);
                subscriptions[sub_count].subscriber_addr = cliaddr; // Guardamos la dirección del cliente
                sub_count++;
                printf("Cliente %s:%d suscrito al tema: %s\n", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port), topic);
            }
        } else if (command != NULL && strcmp(command, "PUBLISH") == 0) {
            char *topic = strtok(NULL, ":");
            char *message = strtok(NULL, "");
            if (topic != NULL && message != NULL) {
                printf("Publicando en el tema '%s': %s\n", topic, message);
                // Reenviar a los suscriptores
                for (int j = 0; j < sub_count; j++) {
                    if (strcmp(subscriptions[j].topic, topic) == 0) {
                        // Usamos sendto para enviar el mensaje a la dirección guardada del suscriptor
                        sendto(sockfd, message, strlen(message), 0, (const struct sockaddr *) &subscriptions[j].subscriber_addr, sizeof(subscriptions[j].subscriber_addr));
                    }
                }
            }
        }
    }

    close(sockfd);
    return 0;
}