#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <errno.h>

#include "server_ftp.h"
#include "pi.h"
#include "responses.h"
#include "utils.h"

static ftp_command_t ftp_commands[] = {
  { "USER", handle_USER },
  { "PASS", handle_PASS },
  { "QUIT", handle_QUIT },
  { "SYST", handle_SYST },
  { "TYPE", handle_TYPE },
  { "PORT", handle_PORT },
  { "RETR", handle_RETR },
  { "STOR", handle_STOR },
  { "NOOP", handle_NOOP },
  { NULL, NULL }
};

int welcome(ftp_session_t *sess) {

  // Send initial FTP welcome message
  if (safe_dprintf(sess->control_sock, MSG_220) != sizeof(MSG_220) - 1) {
    fprintf(stderr, "Send error\n");
    close_fd(sess->control_sock, "cliente socket");
    return -1;
  }

  return 0;
}

int getexe_command(ftp_session_t *sess) {

  char buffer[BUFFER_SIZE];

  // Receive string from CC
  ssize_t len = recv(sess->control_sock, buffer, sizeof(buffer) - 1, 0);
  if (len < 0) {
    perror("Receive fail: ");
    close_fd(sess->control_sock, "cliente socket");
    return -1;
  }

  // The connection was closed improperly and we close it
  if (len == 0) {
    sess->current_user[0] = '\0'; // Close session
    close_fd(sess->control_sock, "client socket"); // Close socket
    sess->control_sock = -1;
    return -1;
  }

  buffer[len] = '\0';

  // Strip CRLF
  char *cr = strchr(buffer, '\r');
  if (cr) *cr = '\0';
  char *lf = strchr(buffer, '\n');
  if (lf) *lf = '\0';

  // Separate command and argument
  char *arg = NULL;
  char *cmd = buffer;

  // Case null command
  if (cmd[0] == '\0') {
    safe_dprintf(sess->control_sock, "500 Empty command.\r\n");
    return 0;
  }

  // Locate arguments in command string
  char *space = strchr(buffer, ' ');
  if (space) {
    *space = '\0';
    arg = space + 1;
    while (*arg == ' ') arg++;
  }

  // Search for command in command-list. 
  ftp_command_t *entry = ftp_commands;
  while (entry->name) {
    if (strcasecmp(entry->name, cmd) == 0) {
      entry->handler(arg ? arg : "");   // Call command handler and pass arguments to it
      return (sess->control_sock < 0) ? -1 : 0;
    }
    entry++;
  }

  safe_dprintf(sess->control_sock, "502 Command not implemented.\r\n");
  return 0;
}


////////////////////////////////////////////////////////////////////////////////////////////

/** CÓDIGO VERSIÓN INICIAL **/
/*
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
*/

/*
    recv_cmd    --> Recepciona un comando desde el socket_descriptor.
    
    recv        --> Realiza la recepción (hace el Receive)

    strtok      --> No es una función (aunque se escribe como una)
                --> Es la base para escribir parsers
                --> Cada vez que se llama con el argumento NULL, devuelve un token diferente
                --> Al volver a la función, la variable persiste en el mismo estado, como si cambiara su scope
    
    En "operation" obtengo la operación - comandos básicos de FTP - gralmente. de 4 letras - lo que mandó el Cliente

*/

/*
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
*/
