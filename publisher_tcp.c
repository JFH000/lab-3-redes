// publisher_tcp.c
// Publisher TCP: envía eventos al broker.
// Uso:
//   ./publisher_tcp <host> <puerto> <topic>
// Luego escribe por stdin líneas con el mensaje (sin el topic) y enter.
// Cada línea se publica como: PUBLISH <topic>|<mensaje>\n

#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static void die(const char *m){ perror(m); exit(1); }

int main(int argc, char **argv){
    if (argc!=4){
        fprintf(stderr,"Uso: %s <host> <puerto> <topic>\n", argv[0]);
        return 1;
    }
    const char *host = argv[1];
    int port = atoi(argv[2]);
    const char *topic = argv[3];

   
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd<0) die("socket");

    struct hostent *he = gethostbyname(host);
    if (!he){ fprintf(stderr,"gethostbyname fallo\n"); return 1; }

    struct sockaddr_in addr;
    memset(&addr,0,sizeof(addr));
    addr.sin_family=AF_INET;
    addr.sin_port=htons(port);
    memcpy(&addr.sin_addr, he->h_addr, he->h_length);

    if (connect(fd,(struct sockaddr*)&addr,sizeof(addr))<0) die("connect");

    fprintf(stderr,"Publisher conectado. Topic='%s'. Escribe mensajes y ENTER (Ctrl+D para salir).\n", topic);

   
    char line[4096];
    while (fgets(line, sizeof(line), stdin)){
   
        size_t L=strlen(line);
        if (L>0 && (line[L-1]=='\n' || line[L-1]=='\r')) line[--L]='\0';

        char out[8192];
        snprintf(out, sizeof(out), "PUBLISH %s|%s\n", topic, line);
        if (send(fd, out, strlen(out), 0) < 0){
            perror("send");
            break;
        }
;
    }

    close(fd);
    return 0;
}