#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include "protocol.h"

int main()
{
    // socket for communication with server
    int client_sock;

    // structs for adress setup
    struct addrinfo hints, *it, *serv_addr;

    // we will try to get all adresses for both ipv4 and ipv6
    memset(&hints, 0, sizeof (hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    // get adress info struct
    getaddrinfo("localhost", "1234", &hints, &serv_addr);

    // connect via first sucessful option
    for (it = serv_addr; it != NULL; it = it->ai_next) {
      client_sock = socket(it->ai_family, it->ai_socktype, it->ai_protocol);
      if (connect(client_sock, it->ai_addr, it->ai_addrlen) == 0) break;
    }
    freeaddrinfo(serv_addr);

    /* client action here */
    char *line;
    Vec *buffer = vec_init(sizeof(char));
    while(line = readline("tac> ")) {
      write_string(client_sock, line);
      free(line);
      read_string(client_sock, buffer);
      printf("%s\n", vec_begin(buffer));
    }

    close(client_sock);
    return 0;

    errors:
        perror("error occured");
        return -1;
}
