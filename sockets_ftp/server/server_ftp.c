#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "server_ftp.h"

// #define VERSION "1.0"    // Lo cortamos y lo pegamos en "server_dtp.h"
#define PTODEFAULT 21       // a Refactoring

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
    char buffer[BUFSIZE];
    char command[BUFSIZE];
    int data_len;

    master_socket = socket(AF_INET, SOCK_STREAM, 0);

    master_addr.sin_family = AF_INET;
    master_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    master_addr.sin_port = htons(port);
    
    bind(master_socket, (struct sockaddr *) &master_addr, sizeof(master_addr) );

    listen(master_socket, 5);

    while (true) {

        slave_addr_len = sizeof(slave_addr);

        // Acepta conexión del socket
        slave_socket = accept(master_socket, (struct sockaddr *) &slave_addr, &slave_addr_len);

        // Envía al cliente mensaje de confirmación.
        if ( send(slave_socket, MSG_220, sizeof(MSG_220) - 1, 0) != sizeof(MSG_220) - 1 ) {
            close(slave_socket); 
            fprintf(stderr, "Error: Falló el envío del mensaje.\n");
            break;
        }

        // Recibe el mensaje del cliente con el comando USER (y el nombre de usuario).
        if ( recv_cmd(slave_socket, command, user_name) != 0 ) {
            close(slave_socket);
            fprintf(stderr, "Error: no se pudo recibir el comando USER.\n");
            break;
        }

        // Verificamos si el comando es "USER"
        if (strcmp(command, "USER") != 0) {
            close(slave_socket);
            fprintf(stderr, "Error: se esperaba el comando USER.\n");
            continue;
        }
        
        // Envía el mensaje 331 tras recibir el mensaje con el comando USER
        data_len = snprintf(buffer, BUFSIZE, MSG_331, user_name);
        if ( send(slave_socket, MSG_331, data_len, 0) < 0 ) {
            close(slave_socket);
            fprintf(stderr, "Error: no se pudo enviar el mensaje MSG_331.\n");
            break;
        }
        
        // Recibe el mensaje del cliente con el comando PASS (y el password).
        if ( recv_cmd(slave_socket, command, user_pass) != 0 ) {
            close(slave_socket);
            fprintf(stderr, "Error: no se pudo recibir el comando PASS.\n");
            break;
        }

        // Verificamos si el comando es "PASS"
        if (strcmp(command, "PASS") != 0) {
            close(slave_socket);
            fprintf(stderr, "Error: se esperaba el comando PASS.\n");
            continue;
        }

        // Verificamos las credenciales (si es false, enviamos mensaje 530).
        if ( !check_credentials(user_name, user_pass) ) {
            data_len = snprintf(buffer, BUFSIZE, MSG_530);
            if (send(slave_socket, buffer, data_len, 0) < 0) {
                close(slave_socket);
                fprintf(stderr, "Error: no se pudo enviar el mensaje MSG_530.\n");
                break;
            }
            close(slave_socket);
            continue;
        }

        // Enviamos el mensaje 230 tras confirmar el login
        data_len = snprintf(buffer, BUFSIZE, MSG_230, user_name);
        if ( send(slave_socket, MSG_230, data_len, 0) < 0 ) {
            close(slave_socket);
            fprintf(stderr, "Error: no se pudo enviar el mensaje MSG_230.\n");
            break;
        }

        /** Aquí comienza la recepción y respuesta continua de comandos desde el Servidor */

        // while (1) {
            // 
        // }
        
    }

    close(master_socket);

    return 0;
}
