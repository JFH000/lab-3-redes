// subscriber_udp.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8081
#define BUFFER_SIZE 1024

int main() {
    int sockfd;
    char buffer[BUFFER_SIZE];
    char topic[50];
    struct sockaddr_in servaddr;
    socklen_t len;
    int n;

    // Crear socket UDP
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Fallo al crear socket");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));

    // Configurar la dirección del broker de destino
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    // Pedir tema y enviar suscripción
    printf("Introduce el tema al que te quieres suscribir: ");
    fgets(topic, 50, stdin);
    topic[strcspn(topic, "\n")] = 0;

    sprintf(buffer, "SUBSCRIBE:%s", topic);
    sendto(sockfd, (const char *)buffer, strlen(buffer), 0, (const struct sockaddr *) &servaddr, sizeof(servaddr));

    printf("Suscrito a '%s'. Esperando mensajes...\n", topic);

    // Bucle para recibir mensajes
    while(1) {
        n = recvfrom(sockfd, (char *)buffer, BUFFER_SIZE, MSG_WAITALL, NULL, NULL);
        buffer[n] = '\0';
        printf(">> Actualizacion: %s\n", buffer);
    }

    close(sockfd);
    return 0;
}