// subscriber_tcp.c
// Subscriber TCP: se suscribe a uno o varios topics y muestra eventos.
// Uso:
//   ./subscriber_tcp <host> <puerto> <topic1> [<topic2> ...]
// Recibe del broker líneas "<topic>|<mensaje>"

#include <arpa/inet.h>
#include <netdb.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUFSZ 8192

static void die(const char *m){ perror(m); exit(1); }

int main(int argc, char **argv){
    if (argc < 4){
        fprintf(stderr,"Uso: %s <host> <puerto> <topic1> [<topic2> ...]\n", argv[0]);
        return 1;
    }
    const char *host = argv[1];
    int port = atoi(argv[2]);

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

    
    for(int i=3;i<argc;i++){
        char msg[512];
        snprintf(msg, sizeof(msg), "SUBSCRIBE %s\n", argv[i]);
        if (send(fd, msg, strlen(msg), 0) < 0) die("send");
    }
    fprintf(stderr,"Suscrito. Esperando eventos...\n");

    
    char inbuf[BUFSZ];
    size_t inlen = 0;

    while (1){
        fd_set rfds; FD_ZERO(&rfds); FD_SET(fd,&rfds);
        int rc = select(fd+1, &rfds, NULL, NULL, NULL);
        if (rc<0){ perror("select"); break; }
        if (FD_ISSET(fd,&rfds)){
            char buf[BUFSZ];
            ssize_t n = recv(fd, buf, sizeof(buf), 0);
            if (n<=0){ fprintf(stderr,"Conexión cerrada.\n"); break; }
            if (inlen + (size_t)n > sizeof(inbuf)) inlen = 0;
            memcpy(inbuf+inlen, buf, n); inlen += (size_t)n;

            size_t start=0;
            for(size_t i=0;i<inlen;i++){
                if (inbuf[i]=='\n'){
                    size_t L = i - start + 1;
                    char line[BUFSZ];
                    if (L >= sizeof(line)) L = sizeof(line)-1;
                    memcpy(line, inbuf+start, L);
                    line[L]='\0';
                    
                    while (L>0 && (line[L-1]=='\n' || line[L-1]=='\r')) line[--L]='\0';
                    if (L>0){
                        printf("%s\n", line); fflush(stdout);
                    }
                    start = i+1;
                }
            }
            if (start>0 && start<inlen){
                memmove(inbuf, inbuf+start, inlen-start);
                inlen -= start;
            } else if (start==inlen){
                inlen=0;
            }
        }
    }
    close(fd);
    return 0;
}