#define _XOPEN_SOURCE 700

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netdb.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "protocol.h"

#define PORT_BUF_LEN 6

/* program config */

/* port range */
int low;
int high;

/* return valid fd or -1 on error */
int bind_server_sock(const char *port) {
    // socket for server
    int serv_sock = -1;

    // structs for adress setup
    struct addrinfo hints, *it, *serv_addr;

    // we will bind on all adresses for both ipv4 and ipv6
    memset(&hints, 0, sizeof (hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    // get adress info struct
    getaddrinfo(NULL, port, &hints, &serv_addr);

    // create sockets and bind on first successful adresses
    int success = 0;
    for (it = serv_addr; it != NULL; it = it->ai_next) {
        serv_sock = socket(it->ai_family, it->ai_socktype, it->ai_protocol);
        if(serv_sock == -1) {
            continue;
        }

        if (bind(serv_sock, it->ai_addr, it->ai_addrlen) == 0) {
            success = 1;
            /* we listen just on 1 family */
            break;
        } else {
            /* try another adress options */
            close(serv_sock);
        }
    }
    freeaddrinfo(serv_addr);

    if(success == 0) {
        return -1;
    }

    return serv_sock;
}

int respond(char *filename, struct sockaddr_storage *client_addr, 
    socklen_t claddr_size) {
    printf("started new client session\n");

    // socket for server
    int serv_sock = -1;

    printf("trying to bind port for client in range [%d, %d)\n", low, high);
    char str_port[PORT_BUF_LEN];
    for(int i = low; i < high; ++i) {
        printf("trying port %s\n", str_port);
        if(snprintf(str_port, PORT_BUF_LEN, "%d", i) >= PORT_BUF_LEN) {
            /* port number too big, str_port buffer is too short */
            return -1;
        }

        serv_sock = bind_server_sock(str_port);
        if(serv_sock != -1) {
            break;
        }
    }
    if(serv_sock == -1) {
        goto errors3;
    }

    /* connect to client */
    /* so we refuse all other connections and for convenience use of send */
    if(connect(serv_sock, (struct sockaddr *)client_addr, claddr_size) != 0) {
        goto errors;
    }

    /* send portname to client */
    send(serv_sock, str_port, strlen(str_port) + 1, 0);


    printf("waiting for content\n");
    char buf[BUF_LEN];
    ssize_t n;
    int write_file;

    /* open local file where we store client data */
    if((write_file = open(filename, O_CREAT | O_TRUNC | O_WRONLY, 0666)) == -1) {
        goto errors;
    }

    /* receive content */
    while ((n = recv(serv_sock, buf, BUF_LEN, 0)) > 0) {
        if(n == 1 && buf[0] == '\0') {
            break;
        }

        ssize_t written = write(write_file, buf, n);
        if(written != n) {
            goto errors2;
        }
    }

    printf("ended writing\n");

    close(write_file);
    close(serv_sock);

    return 0;

    errors2:
        close(write_file);

    errors:
        close(serv_sock);

    errors3:
        perror("error occured");
        return -1;
}

void usage() {
 fprintf(stderr, "usage: a.out <port> <low> <high>\n");

 exit(1);
}

int main(int argc, char **argv)
{   
    if(argc < 4) {
        usage();
    }

    char *port = argv[1];
    /* todo error handling */
    low = atoi(argv[2]);
    high = atoi(argv[3]);

    int serv_sock = bind_server_sock(port);
    if(serv_sock == -1) {
        goto errors;
    }

    /* accept clients */
    struct sockaddr_storage client_addr;
    socklen_t claddr_size = sizeof(client_addr);
    char buf[BUF_LEN + 1];
    ssize_t n;

    while ((n = recvfrom(serv_sock, buf, BUF_LEN, 0, (struct sockaddr *)&client_addr,&claddr_size)) > 0) {
        printf("newclient\n");
        /* this is a very important security measure, do not remove!
           we will interpret data send to us as string so we want it to be
           always null terminated, even if client didn't send any \0 */
        buf[n] = '\0';

        /* fork for each client */
        switch(fork()) {
            case -1:
                goto errors;
            case 0:
                /* in child */
                close(serv_sock);
                respond(buf, &client_addr, claddr_size);
                return 0;
            default:
                /* in parent */
                break;
        }

        /* collect returned children (ended connections) */
        waitpid(-1, NULL, WNOHANG);
    }

    close(serv_sock);

    return 0;

    errors:
        perror("serious error occured");
        return -1;
}