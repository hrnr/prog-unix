#define _XOPEN_SOURCE 700

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "protocol.h"

void usage() {
 fprintf(stderr, "usage: a.out <filename> <server> <port>\n");

 exit(1);
}

int main(int argc, char **argv)
{
    if(argc < 4) {
        usage();
    }

    char *filename = argv[1];
    char *server = argv[2];
    char *port = argv[3];

    // socket for communication with server
    int client_sock = -1;

    // structs for adress setup
    struct addrinfo hints, *it, *serv_addr;

    // we will try to get all adresses for both ipv4 and ipv6
    memset(&hints, 0, sizeof (hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    // get address info struct
    getaddrinfo(server, port, &hints, &serv_addr);

    // connect via first sucessful option
    for (it = serv_addr; it != NULL; it = it->ai_next) {
      client_sock = socket(it->ai_family, it->ai_socktype, it->ai_protocol);
      if(client_sock == -1) {
        continue;
      }

      /* send initial filename */
      if(sendto(client_sock, filename, strlen(filename) + 1, 0,
        it->ai_addr, it->ai_addrlen) != -1) {
        /* this adress works, so keep it */
        break;
      } else {
        /* try another adress options */
        close(client_sock);
      }
    }
    freeaddrinfo(serv_addr);

    /* receive port */
    struct sockaddr_storage server_addr;
    socklen_t server_addr_size = sizeof(server_addr);
    printf("waiting for port\n");
    size_t s = 0; /* we don't really dont need this */
    recvfrom(client_sock, &s, sizeof(size_t), 0, (struct sockaddr *)&server_addr, &server_addr_size);

    /* connect to transfer port */
    if(connect(client_sock, (struct sockaddr *)&server_addr, server_addr_size) != 0) {
        goto errors;
    }
    printf("connected to trasfer port, waiting for input\n");

    char buf[BUF_LEN];
    ssize_t n;
    while((n = read(0, buf, BUF_LEN)) > 0) {
        send(client_sock, buf, n, 0);
    }

    /* protocol: end connection */
    send(client_sock, "", 1, 0);

    close(client_sock);
    return 0;

    errors:
        close(client_sock);
        perror("error occured");
        return -1;
}
