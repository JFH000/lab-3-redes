// publisher_udp.c

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
    char topic[50], message[200], buffer[BUFFER_SIZE];
    struct sockaddr_in servaddr;

    // Crear el socket UDP
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Fallo al crear socket");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));

    // Configurar la dirección del broker de destino
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    printf("Publicador UDP listo. Escribe 'salir' para terminar.\n");

    while(1) {
        printf("Introduce el tema: ");
        fgets(topic, 50, stdin);
        topic[strcspn(topic, "\n")] = 0;

        if(strcmp(topic, "salir") == 0) break;

        printf("Introduce el mensaje: ");
        fgets(message, 200, stdin);
        message[strcspn(message, "\n")] = 0;

        sprintf(buffer, "PUBLISH:%s:%s", topic, message);

        // Enviar el mensaje al broker usando sendto
        sendto(sockfd, (const char *)buffer, strlen(buffer), 0, (const struct sockaddr *) &servaddr, sizeof(servaddr));
        printf("Mensaje enviado.\n\n");
    }

    close(sockfd);
    return 0;
}