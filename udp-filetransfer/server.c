#define _XOPEN_SOURCE 700

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netdb.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "protocol.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define PORT_BUF_LEN 20

/* program config */
int low;
int high;

int respond(char *filename, struct sockaddr_storage *client_addr, 
    socklen_t claddr_size) {
    printf("would respond\n");

    // socket for server
    int serv_sock = -1;

    // structs for adress setup
    struct addrinfo hints, *it, *serv_addr;

    // we will bind on all adresses for both ipv4 and ipv6
    memset(&hints, 0, sizeof (hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;


    char str_port[PORT_BUF_LEN];
    int success = 0;
    int i;
    printf("low %d, high %d\n", low, high);
    for(i = low; success == 0 && i < high; ++i) {
        // get adress info struct
        snprintf(str_port, PORT_BUF_LEN, "%d", i);
        printf("trying port %s\n", str_port);
        getaddrinfo(NULL, str_port, &hints, &serv_addr);

        // create sockets and bind on first successful adresses
        for (it = serv_addr; it != NULL; it = it->ai_next) {
            serv_sock = socket(it->ai_family, it->ai_socktype, it->ai_protocol);
            if (bind(serv_sock, it->ai_addr, it->ai_addrlen) == 0) {
                success = 1;
                printf("success binding transfer socket\n");
                break;
            }
        }
        freeaddrinfo(serv_addr);
    }
    if(success == 0) {
        /* todo err hadling can't bind any port */
    }

    /* connect to client */
    /* just so we refuse all other connections */
    if(connect(serv_sock, (struct sockaddr *)client_addr, claddr_size) != 0) {
        goto errors;
    }

    /* send portname to client */
    sendto(serv_sock, str_port, strlen(str_port) + 1, 0, (struct sockaddr *)client_addr, claddr_size);


    printf("waiting for content\n");
    char buf[BUF_LEN +1];
    ssize_t n;
    int write_file;

    if((write_file = open(filename, O_CREAT | O_TRUNC | O_RDWR, 0666)) == -1) {
        goto errors;
    }

    /* receive content */
    while ((n = recvfrom(serv_sock, buf, BUF_LEN, 0, (struct sockaddr *)&client_addr,&claddr_size)) > 0) {
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
        perror("error occured");
        return -1;
}

void usage() {
 fprintf(stderr,
    "usage: a.out <port> <low> <high>\n");

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
        if (bind(serv_sock, it->ai_addr, it->ai_addrlen) == 0) {
            success = 1;
            // switch(fork()) {
            // case -1:
            //     perror("couldn't fork");
            //     return (-1);
            //     break;
            // case 0:
            //     /* in child */
            //     close(serv_sock);
            //     continue;
            //     break;
            // default:
            //     /* in parent */
            //     break;
            // }
            break;  // we listen just on 1 family 
        }
    }
    freeaddrinfo(serv_addr);

    if(success == 0) {
        /* report error here */
    }

    // accept clients
    struct sockaddr_storage client_addr;
    socklen_t claddr_size = sizeof(client_addr);
    char buf[BUF_LEN +1];
    ssize_t n;

    while ((n = recvfrom(serv_sock, buf, BUF_LEN, 0, (struct sockaddr *)&client_addr,&claddr_size)) > 0) {
        printf("newclient\n");
        buf[n] = '\0';

        /* fork for each client */
        switch(fork()) {
            case -1:
                perror("couldn't fork");
                return (-1);
                break;
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

   /* errors:
        perror("error occured");
        return -1;*/
}