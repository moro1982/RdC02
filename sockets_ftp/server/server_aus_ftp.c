#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "server_aus_ftp.h"

// #define VERSION "1.0"    // Lo cortamos y lo pegamos en "server_dtp.h"
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

    int master_socket, slave_socket;
    struct sockaddr_in master_addr, slave_addr;
    socklen_t slave_addr_len;
    char user_name[BUFSIZE];
    char user_pass[BUFSIZE];

    master_socket = socket(AF_INET, SOCK_STREAM, 0);

    master_addr.sin_family = AF_INET;
    master_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    master_addr.sin_port = htons(port);
    
    bind(master_socket, (struct sockaddr *) &master_addr, sizeof(master_addr) );

    listen(master_socket, 5);

    while (true) {
        slave_addr_len = sizeof(slave_addr);
        slave_socket = accept(master_socket, (struct sockaddr *) &slave_addr, &slave_addr_len);
        // Envía al cliente mensaje de confirmación.
        if (send(slave_socket, MSG_220, sizeof(MSG_220) - 1, 0) != sizeof(MSG_220) - 1) {
            close(slave_socket); 
            fprintf(stderr, "Error: Falló el envío del mensaje.\n");
            break;
        }
        // Recibe el mensaje del cliente con el comando USER (y el nombre de usuario).
        if (recv_cmd(slave_socket, "USER", user_name) != 0) {
            close(slave_socket);
            fprintf(stderr, "Error: no se pudo recibir el comando USER.\n");
            break;
        }
        // Recibe el mensaje del cliente con el comando PASS (y el password).
        if (recv_cmd(slave_socket, "PASS", user_pass) != 0) {
            close(slave_socket);
            fprintf(stderr, "Error: no se pudo recibir el comando PASS.\n");
            break;
        }
        
    }

    close(master_socket);

    return 0;
}
