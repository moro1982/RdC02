#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <errno.h>

#include "server_ftp.h"
#include "utils.h"
#include "config.h"
#include "pi.h"
#include "session.h"

/* return master_socket */
extern int master_socket;

int server_init(const char *ip, int port) {

	// Create server address structure
	struct sockaddr_in server_addr;

	// Create server socket
	int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_fd < 0) {
		fprintf(stderr, "Error creating socket: ");
		perror(NULL);
		return -1;
	}

	// avoid problem with reuse inmeditely after force quiting
	const int opt = 1;
	if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
	  fprintf(stderr, "Error setting SO_REUSEADDR: ");
	  perror(NULL);
	  close(listen_fd);
	  return -1;
	}

	#ifdef SO_REUSEPORT
	  if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
	    fprintf(stderr, "Error setting SO_REUSEPORT: ");
	    perror(NULL);
	    close(listen_fd);
	    return -1;
	  }
	#endif

	// Initialize server address structure
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);

	// inet_pton() -> Translate IP address from text to binary format
	if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0) {
	  fprintf(stderr, "Invalid IP address: %s\n", ip);
	  close(listen_fd);
	  return -1;
	}

	// Asign server address (IP + port) to socket (listen_fd) 
	if (bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
	  fprintf(stderr, "Bind failed: ");
	  perror(NULL);
	  close(listen_fd); // S
	  return -1;
	}

	// Translate back IP address from binary to text, in order to print it
	char ip_buf[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &server_addr.sin_addr, ip_buf, sizeof(ip_buf));
	printf("Listening on %s:%d\n", ip_buf, port);

	// Listen to server socket for new connections
	if (listen(listen_fd, SOMAXCONN) < 0) {
	  fprintf(stderr, "Listen failed: ");
	  perror(NULL);
	  close(listen_fd);
	  return -1;
	}

	master_socket = listen_fd;
  	return listen_fd;

	/* CÓDIGO ANTERIOR */
	/*
	
	master_socket = socket(AF_INET, SOCK_STREAM, 0);	// Construimos el master_socket local

	if(server_socket < 0) {		// Validar (si es <0)
		perror("Error socket creation");
		return -1;
	}

    // avoid problem at reusing socket after force quitting
	const int opt = 1;
	if( setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0 ) {
		perror("Error setting SO_REUSEADDR");
		close(server_socket);
		return -1;
	}

    struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);

	if( inet_pton(AF_INET, ip, &server_addr.sin_addr) < 0 ) {	//--> inet_pton ?
		fprintf(stderr, "Invalid IP address %s\n", ip);
		close(server_socket);
		return -1;
	}

    // Hacemos y validamos el bind
	if( bind(server_socket, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0 ) {
		perror("Bind failed");
		close(server_socket);
		return -1;
	}

    // Averiguamos en que interfaz esta realmente escuchando el server.
	// ANY_ADDR	--> no se sabe que interfaz
	if( listen(server_socket, SOMAXCON) < 0) {
		perror("Listen failed");
		close(server_socket);
		return -1;
	}

	return server_socket;

	*/
}


/* server_accept */
int server_accept(int listen_fd, struct sockaddr_in *client_addr) {

	// client_addr can be NULL if caller doesn't need client info
	socklen_t addrlen = sizeof(*client_addr);
	int new_socket = accept(listen_fd, (struct sockaddr *)client_addr, &addrlen);

	// EINTR for avoid errors by signal reentry
	// https://stackoverflow.com/questions/41474299/checking-if-errno-eintr-what-does-it-mean
	if (new_socket < 0 && errno != EINTR) {
	  fprintf(stderr, "Accept failed: ");
	  perror(NULL);
	  return -1;
	}

	return new_socket;


}
/* CÓDIGO ANTERIOR */
/*
int server_accept(int socket) {

	struct sockaddr slave_addr;
	socklen_t slave_addr_len = sizeof(slave_addr);
	slave_socket = accept( socket, (struct sockaddr *) &slave_addr, &slave_addr_len);
	
	// Si falla el accept:
	if( slave_socket < 0) {
		perror("Accept failed");
		return -1;
	}
 
	// Si no falla:
	char ip_str[INET_ADDRLEN];
	inet_ntop(AF_INET, &slave_addr.sin_addr, ip_str, sizeof(ip_str));	// No verificamos si es negativo
	fprintf(stderr, "Connection from %s %d\n", ip_str, ntohs(slave_addr.sin_port));

	return slave_socket;
}
*/


/* server_loop */
void server_loop(int socket) {
	// welcome
	// while() {...toma comandos...}
	// cierre de sesion
	session_init(socket);
	if( welcome(current_sess) < 0 ) {
		return;
	}
	while(1) {
		if( getexe_command(current_sess) < 0 ) {
			break;
		}
	}
	session_cleanup();
}

////////////////////////////////////////////////////////////////////////////////////////////

/* CÓDIGO ANTERIOR */
/*
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
        port = PORT_DEFAULT;  // Puerto x default FTP
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

        // Aquí comienza la recepción y respuesta continua de comandos desde el Servidor

        while (1) {
            if (recv_cmd(slave_socket, command, buffer) != 0) {
                close(slave_socket);
                fprintf(stderr, "Error: no se pudo recibir el comando.\n");
                break;
            }
            if (strcasecmp(command, "SYST") == 0) {
                if ( send(slave_socket, MSG_215, sizeof(MSG_215) - 1, 0) < 0) {
                    close(slave_socket);
                    fprintf(stderr, "Error: no se pudo enviar el mensaje MSG_215.\n");
                    break;
                }
                continue;
            }
            if (strcasecmp(command, "FEAT") == 0) {
                if (send(slave_socket, MSG_211, sizeof(MSG_211) - 1, 0) < 0) {
                    close(slave_socket);
                    fprintf(stderr, "Error: no se pudo enviar el mensaje MSG_211.\n");
                    break;
                }
                continue;
            }
            if (strcasecmp(command, "QUIT") == 0) {
                if ( send(slave_socket, MSG_221, sizeof(MSG_221) - 1, 0) < 0 ) {
                    close(slave_socket);
                    fprintf(stderr, "Error: no se pudo enviar el mensaje MSG_221.\n");
                    break;
                }
                close(slave_socket);
                break;
            }
        }   
    }

    close(master_socket);

    return 0;
}
*/