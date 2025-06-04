#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "server_aus_ftp.h"

#define MSG_220 "220 srvFtp version 1.0\r\n"
#define MSG_331 "331 Password required for %s\r\n"
#define MSG_230 "230 User %s logged in\r\n"
#define MSG_530 "530 Login incorrect\r\n"
#define MSG_221 "221 Goodbye\r\n"
#define MSG_550 "550 %s: no such file or directory\r\n"
#define MSG_299 "299 File %s size %ld bytes\r\n"
#define MSG_226 "226 Transfer complete\r\n"

/*
    recv_cmd    --> Recepciona un comando desde el socket_descriptor.
    
    recv        --> Realiza la recepción (hace el Receive)

    strtok      --> No es una función (aunque se escribe como una)
                --> Es la base para escribir parsers
                --> Cada vez que se llama con el argumento NULL, devuelve un token diferente
                --> Al volver a la función, la variable persiste en el mismo estado, como si cambiara su scope
    
    En "operation" obtengo la operación - comandos básicos de FTP - gralmente. de 4 letras - lo que mandó el Cliente

*/

int recv_cmd(int socket_descriptor, char *operation, char *param) {
    char buffer[BUFSIZE];
    char *token;

    if ( recv(socket_descriptor, buffer, BUFSIZE, 0) < 0 ) {
        fprintf(stderr, "Error: Problema en la recepción.\n");
        return 1;
    }

    buffer[strcspn(buffer, "\r\n")] = 0;
    token = strtok(buffer, " ");
    if (token == NULL || strlen(token) < 4) {
        fprintf(stderr, "Error: Comando FTP no válido");
        return 1;
    } else {
        strcpy(operation, token);
        token = strtok(NULL, " ");  // Le paso el buffer que tenia, para que pase el siguiente token
        #if DEBUG 
            printf("par %s\n", token);  // Cuando compilamos con -D DEBUG, se compila con lo que está entre #if ... #endif
        #endif
        if (token != NULL) strcpy(param, token);
    }
}