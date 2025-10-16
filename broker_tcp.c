// broker_tcp.c
// Broker TCP para modelo publicación–suscripción.
// Acepta Publishers y Subscribers. Protocolo de texto por líneas:
//   SUBSCRIBE <topic>\n
//   PUBLISH <topic>|<mensaje>\n
// El broker reenvía "<topic>|<mensaje>\n" a todos los suscritos a <topic>.

#define _GNU_SOURCE
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

#define BACKLOG 32
#define MAX_CLIENTS FD_SETSIZE
#define BUFSZ 4096
#define MAX_TOPICS 256
#define MAX_TOPIC_LEN 128

typedef struct {
    int fd;
    char inbuf[BUFSZ];
    size_t inlen;

    char *subs[MAX_TOPICS];
    int subs_count;
} client_t;

static client_t clients[MAX_CLIENTS];
static int listen_fd = -1;

static void die(const char *msg) { perror(msg); exit(EXIT_FAILURE); }

static void set_reuse(int fd){
    int opt=1;
    if (setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt))<0) die("setsockopt");
}

static void add_client(int fd){
    for (int i=0;i<MAX_CLIENTS;i++){
        if (clients[i].fd==-1){
            clients[i].fd=fd;
            clients[i].inlen=0;
            clients[i].subs_count=0;
            return;
        }
    }
    fprintf(stderr,"Max clients reached\n");
    close(fd);
}

static void remove_client(int i){
    if (clients[i].fd!=-1){
        close(clients[i].fd);
        clients[i].fd=-1;
        for(int k=0;k<clients[i].subs_count;k++) free(clients[i].subs[k]);
        clients[i].subs_count=0;
    }
}

static bool is_subscribed(client_t *c, const char *topic){
    for(int i=0;i<c->subs_count;i++){
        if(strcmp(c->subs[i], topic)==0) return true;
    }
    return false;
}

static void subscribe_topic(client_t *c, const char *topic){
    if ((int)strlen(topic) == 0 || (int)strlen(topic) > MAX_TOPIC_LEN) return;
    if (c->subs_count >= MAX_TOPICS) return;
    if (is_subscribed(c, topic)) return;
    c->subs[c->subs_count]=strdup(topic);
    c->subs_count++;
    dprintf(c->fd, "OK SUB %s\n", topic);
}

static void broadcast_to_topic(const char *topic, const char *payload, int from_fd){
    char line[BUFSZ];

    snprintf(line, sizeof(line), "%s|%s\n", topic, payload);
    for (int i=0;i<MAX_CLIENTS;i++){
        if (clients[i].fd!=-1 && clients[i].fd!=from_fd){
            if (is_subscribed(&clients[i], topic)){
                ssize_t n=send(clients[i].fd, line, strlen(line), 0);
                if(n<0){  }
            }
        }
    }
}

static void handle_command(client_t *c, char *line){
  
    size_t L = strlen(line);
    while (L>0 && (line[L-1]=='\n' || line[L-1]=='\r')) line[--L]='\0';
    if (L==0) return;

    if (strncmp(line, "SUBSCRIBE ", 10)==0){
        const char *topic = line + 10;
        subscribe_topic(c, topic);
        return;
    }
    if (strncmp(line, "PUBLISH ", 8)==0){

        char *p = line + 8;
        char *sep = strchr(p, '|');
        if (!sep){ dprintf(c->fd, "ERR MALFORMED\n"); return; }
        *sep = '\0';
        const char *topic = p;
        const char *msg = sep+1;
        broadcast_to_topic(topic, msg, c->fd);
        dprintf(c->fd, "OK PUB %s\n", topic);
        return;
    }
    dprintf(c->fd, "ERR UNKNOWN\n");
}

static void process_input(int idx){
    client_t *c = &clients[idx];
    char buf[BUFSZ];
    ssize_t n = recv(c->fd, buf, sizeof(buf), 0);
    if (n<=0){ remove_client(idx); return; }

    if (c->inlen + (size_t)n > sizeof(c->inbuf)) {
    
        c->inlen = 0;
    }
    memcpy(c->inbuf + c->inlen, buf, n);
    c->inlen += (size_t)n;


    size_t start = 0;
    for (size_t i=0; i<c->inlen; i++){
        if (c->inbuf[i]=='\n'){
            size_t len = i - start + 1;
            char line[BUFSZ];
            if (len >= sizeof(line)) len = sizeof(line)-1;
            memcpy(line, c->inbuf + start, len);
            line[len]='\0';
            handle_command(c, line);
            start = i+1;
        }
    }

    if (start>0 && start<c->inlen){
        memmove(c->inbuf, c->inbuf+start, c->inlen - start);
        c->inlen -= start;
    } else if (start==c->inlen){
        c->inlen=0;
    }
}

static void on_sigint(int sig){
    (void)sig;
    if (listen_fd!=-1) close(listen_fd);
    for(int i=0;i<MAX_CLIENTS;i++) remove_client(i);
    exit(0);
}

int main(int argc, char **argv){
    if (argc!=2){
        fprintf(stderr, "Uso: %s <puerto>\n", argv[0]);
        return 1;
    }
    signal(SIGINT, on_sigint);
    for (int i=0;i<MAX_CLIENTS;i++) clients[i].fd=-1;

    int port = atoi(argv[1]);
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd<0) die("socket");
    set_reuse(listen_fd);

    struct sockaddr_in addr;
    memset(&addr,0,sizeof(addr));
    addr.sin_family=AF_INET;
    addr.sin_addr.s_addr=INADDR_ANY;
    addr.sin_port=htons(port);

    if (bind(listen_fd,(struct sockaddr*)&addr,sizeof(addr))<0) die("bind");
    if (listen(listen_fd, BACKLOG)<0) die("listen");

    printf("Broker TCP escuchando en puerto %d...\n", port);

    while (1){
        fd_set rfds; FD_ZERO(&rfds);
        FD_SET(listen_fd,&rfds);
        int maxfd = listen_fd;
        for (int i=0;i<MAX_CLIENTS;i++){
            if (clients[i].fd!=-1){
                FD_SET(clients[i].fd,&rfds);
                if (clients[i].fd>maxfd) maxfd=clients[i].fd;
            }
        }
        int rc = select(maxfd+1, &rfds, NULL, NULL, NULL);
        if (rc<0){
            if (errno==EINTR) continue;
            die("select");
        }
        if (FD_ISSET(listen_fd,&rfds)){
            int cfd = accept(listen_fd,NULL,NULL);
            if (cfd>=0) add_client(cfd);
        }
        for (int i=0;i<MAX_CLIENTS;i++){
            if (clients[i].fd!=-1 && FD_ISSET(clients[i].fd,&rfds)){
                process_input(i);
            }
        }
    }
    return 0;
}