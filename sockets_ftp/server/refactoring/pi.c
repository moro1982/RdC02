#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "server_pi.h"
#include "server_ftp.h"

int is_valid_command(const char *command) {
    int i = 0;
    
    while (valid_commands[i] != NULL) {
        if (strcmp(command, valid_commands[i]) == 0) {
            return arg_commands[i];
        }
        i++;
    }

    return -1;  // Comando no válido
}

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
    int args_number;

    if ( recv(socket_descriptor, buffer, BUFSIZE, 0) < 0 ) {
        fprintf(stderr, "Error: Problema en la recepción.\n");
        return 1;
    }

    buffer[strcspn(buffer, "\r\n")] = 0;
    token = strtok(buffer, " ");
    
    if ( token == NULL || strlen(token) < 3 || (args_number = is_valid_command(token)) < 0 ) {
        fprintf(stderr, "Error: comando no válido.\n");
        return 1;
    }
    
    strcpy(operation, token);   // Aquí se producía el Segmentation Fault.
    
    if (!args_number) {
        return 0;   // No hay parámetro, se retorna inmediatamente.
    }
    
    token = strtok(NULL, " ");  // Le paso el buffer que tenia, para que pase el siguiente token
    
    #if DEBUG 
        printf("par %s\n", token);  // Cuando compilamos con -D DEBUG, se compila con lo que está entre #if ... #endif
    #endif

    if (token != NULL) {
        strcpy(param, token);
    } else {
        fprintf(stderr, "Error: se esperaba un argumento para el comando %s.\n", operation);
        return 1;
    }
    return 0;
}