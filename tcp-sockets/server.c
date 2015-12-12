#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include "protocol.h"

void respond(int conn_sock) {
    // Buffer to store text data
    Vec *buffer = vec_init(sizeof(char));

    while(read_string(conn_sock, buffer) == 0) {
        write_string(conn_sock, vec_begin(buffer));
    }
}

int main()
{
    // socket for server
    int serv_sock;

    // queue size for socket
    int queue_size = 100;

    // structs for adress setup
    struct addrinfo hints, *it, *serv_addr;

    // we will bind on all adresses for both ipv4 and ipv6
    memset(&hints, 0, sizeof (hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    // get adress info struct
    getaddrinfo(NULL, "1234", &hints, &serv_addr);

    // create sockets and bind on first successful adresses
    for (it = serv_addr; it != NULL; it = it->ai_next) {
        serv_sock = socket(it->ai_family, it->ai_socktype, it->ai_protocol);
        if (!bind(serv_sock, it->ai_addr, it->ai_addrlen)) break;
    }
    freeaddrinfo(serv_addr);

    // start listening on socket
    listen(serv_sock, queue_size);

    // accept clients
    struct sockaddr_storage client_addr;
    socklen_t claddr_size = sizeof(client_addr);
    int client_conn;

    for (;;) {
        client_conn = accept(serv_sock, (struct sockaddr *)&client_addr, &claddr_size);
        printf("newclient\n");

        switch(fork()) {
            case -1:
                perror("couldn't fork");
                return (-1);
                break;
            case 0:
                /* in child */
                close(serv_sock);
                respond(client_conn);
                break;
            default:
                /* in parent */
                close(client_conn);
                break;
        }
    }

    return 0;

    errors:
        perror("error occured");
        return -1;
}