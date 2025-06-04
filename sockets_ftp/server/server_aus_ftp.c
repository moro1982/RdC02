#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "server_dtp.h"

#define VERSION "1.0"
#define PTODEFAULT 21

int main(int argc, char const *argv[])
{
    int port;

    if (argc > 2 ) {
        fprintf(stderr, "Error: Nro. de argumentos incorrecto");
        return 1;
    }

    if (argc == 2) {
        port = atoi(argv[1]);
    } else {
        port = 21;  // Puerto x default FTP
    }
    
    if (port == 0) {
        fprintf(stderr, "Error: Nro. de Puerto inválido");
        return 1;
    }
    
    printf("%d\n", port);

    printf("Retorno de la funcion: %d\n", check_credentials("test", "test"));

    int master_socket, slave_socket;
    struct sockaddr_in master_addr, slave_addr;
    socklen_t slave_addr_len;

    master_socket = socket(AF_INET, SOCK_STREAM, 0);

    master_addr.sin_family = AF_INET;
    master_addr.sin_addr.s_addr = INADDR_ANY;
    master_addr.sin_port = htons(port);
    
    bind(master_socket, (struct sockaddr *) &master_addr, sizeof(master_addr) );

    listen(master_socket, 5);

    while (true) {
        slave_addr_len = sizeof(slave_addr);
        slave_socket = accept(master_socket, (struct sockaddr *) &slave_addr, &slave_addr_len);
        send(slave_socket, "220 1", sizeof("220 1"), 0);
        printf("Envío funcionando.\n");
    }

    close(master_socket);

    return 0;
}
