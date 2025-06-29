#include "arguments.h"
#include "server_ftp.h"
#include "utils.h"
#include "signals.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>

int main(int argc, char *argv[])
{
    struct arguments args;

    if(parse_arguments(argc, argv, &args) != 0) {
		return EXIT_FAILURE;
	}

    printf("Start server on %s %d\n", args.address, args.port);

    int master_socket = server_init(args.address, args.port);

	if(master_socket < 0) {
		return EXIT_FAILURE;
	}

	setup_signals();

	while(1) {
		struct sockaddr_in slave_addr;

        // Accept connection request from new client
		int slave_socket = server_accept(master_socket, &slave_addr);	// Estructura del socket creada afuera
		if( slave_socket < 0 ) {
			continue;
		}

        // Translate client IP address to text in order to print it.
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &slave_addr.sin_addr, client_ip, sizeof(client_ip));
        printf("Connection from %s:%d accepted\n", client_ip, ntohs(slave_addr.sin_port));

        // Entering connection loop to start client session and listen for commands.
        server_loop(slave_socket);
        
        printf("Connection from %s:%d closed\n", client_ip, ntohs(slave_addr.sin_port));

        // NEVER GO HERE
        close_fd(master_socket, "listening socket");
        
        // https://en.cppreference.com/w/c/program/EXIT_status
        return EXIT_SUCCESS;
	}
	// No salgo del while

/////////////////////////////////////////////////////////////////////////////////////////////

    /* CÓDIGO VERSIÓN INICIAL */
    // struct sockaddr_in master_addr, slave_addr;
    // socklen_t slave_addr_len;
    // char user_name[BUFSIZE];
    // char user_pass[BUFSIZE];
    // char buffer[BUFSIZE];
    // char command[BUFSIZE];
    // int data_len;

    // master_socket = socket(AF_INET, SOCK_STREAM, 0);
    // master_addr.sin_family = AF_INET;
    // master_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    // master_addr.sin_port = htons(port);
    
    // bind(master_socket, (struct sockaddr *) &master_addr, sizeof(master_addr) );

    // listen(master_socket, 5);

    // while (true) {

    //     slave_addr_len = sizeof(slave_addr);

    //     // Acepta conexión del socket
    //     slave_socket = accept(master_socket, (struct sockaddr *) &slave_addr, &slave_addr_len);

    //     // Envía al cliente mensaje de confirmación.
    //     if ( send(slave_socket, MSG_220, sizeof(MSG_220) - 1, 0) != sizeof(MSG_220) - 1 ) {
    //         close(slave_socket); 
    //         fprintf(stderr, "Error: Falló el envío del mensaje.\n");
    //         break;
    //     }

    //     // Recibe el mensaje del cliente con el comando USER (y el nombre de usuario).
    //     if ( recv_cmd(slave_socket, command, user_name) != 0 ) {
    //         close(slave_socket);
    //         fprintf(stderr, "Error: no se pudo recibir el comando USER.\n");
    //         break;
    //     }

    //     // Verificamos si el comando es "USER"
    //     if (strcmp(command, "USER") != 0) {
    //         close(slave_socket);
    //         fprintf(stderr, "Error: se esperaba el comando USER.\n");
    //         continue;
    //     }
        
    //     // Envía el mensaje 331 tras recibir el mensaje con el comando USER
    //     data_len = snprintf(buffer, BUFSIZE, MSG_331, user_name);
    //     if ( send(slave_socket, MSG_331, data_len, 0) < 0 ) {
    //         close(slave_socket);
    //         fprintf(stderr, "Error: no se pudo enviar el mensaje MSG_331.\n");
    //         break;
    //     }
        
    //     // Recibe el mensaje del cliente con el comando PASS (y el password).
    //     if ( recv_cmd(slave_socket, command, user_pass) != 0 ) {
    //         close(slave_socket);
    //         fprintf(stderr, "Error: no se pudo recibir el comando PASS.\n");
    //         break;
    //     }

    //     // Verificamos si el comando es "PASS"
    //     if (strcmp(command, "PASS") != 0) {
    //         close(slave_socket);
    //         fprintf(stderr, "Error: se esperaba el comando PASS.\n");
    //         continue;
    //     }

    //     // Verificamos las credenciales (si es false, enviamos mensaje 530).
    //     if ( !check_credentials(user_name, user_pass) ) {
    //         data_len = snprintf(buffer, BUFSIZE, MSG_530);
    //         if (send(slave_socket, buffer, data_len, 0) < 0) {
    //             close(slave_socket);
    //             fprintf(stderr, "Error: no se pudo enviar el mensaje MSG_530.\n");
    //             break;
    //         }
    //         close(slave_socket);
    //         continue;
    //     }

    //     // Enviamos el mensaje 230 tras confirmar el login
    //     data_len = snprintf(buffer, BUFSIZE, MSG_230, user_name);
    //     if ( send(slave_socket, MSG_230, data_len, 0) < 0 ) {
    //         close(slave_socket);
    //         fprintf(stderr, "Error: no se pudo enviar el mensaje MSG_230.\n");
    //         break;
    //     }

    //     /** Aquí comienza la recepción y respuesta continua de comandos desde el Servidor */

    //     while (1) {
    //         if (recv_cmd(slave_socket, command, buffer) != 0) {
    //             close(slave_socket);
    //             fprintf(stderr, "Error: no se pudo recibir el comando.\n");
    //             break;
    //         }
    //         if (strcasecmp(command, "SYST") == 0) {
    //             if ( send(slave_socket, MSG_215, sizeof(MSG_215) - 1, 0) < 0) {
    //                 close(slave_socket);
    //                 fprintf(stderr, "Error: no se pudo enviar el mensaje MSG_215.\n");
    //                 break;
    //             }
    //             continue;
    //         }
    //         if (strcasecmp(command, "FEAT") == 0) {
    //             if (send(slave_socket, MSG_211, sizeof(MSG_211) - 1, 0) < 0) {
    //                 close(slave_socket);
    //                 fprintf(stderr, "Error: no se pudo enviar el mensaje MSG_211.\n");
    //                 break;
    //             }
    //             continue;
    //         }
    //         if (strcasecmp(command, "QUIT") == 0) {
    //             if ( send(slave_socket, MSG_221, sizeof(MSG_221) - 1, 0) < 0 ) {
    //                 close(slave_socket);
    //                 fprintf(stderr, "Error: no se pudo enviar el mensaje MSG_221.\n");
    //                 break;
    //             }
    //             close(slave_socket);
    //             break;
    //         }
    //     }   
    // }

    // close(master_socket);

    // return 0;
}
