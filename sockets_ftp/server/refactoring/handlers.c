// handlers.c

#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include "responses.h"
#include "pi.h"
#include "dtp.h"
#include "session.h"
#include "utils.h"

void handle_USER(const char *args) {
  
  ftp_session_t *sess = session_get();

  if (!args || strlen(args) == 0) {
    safe_dprintf(sess->control_sock, MSG_501); // Syntax error in parameters
    return;
  }

  strncpy(sess->current_user, args, sizeof(sess->current_user) - 1);
  sess->current_user[sizeof(sess->current_user) - 1] = '\0';
  safe_dprintf(sess->control_sock, MSG_331); // Username okay, need password
}

void handle_PASS(const char *args) {

  ftp_session_t *sess = session_get();

  if (sess->current_user[0] == '\0') {
    safe_dprintf(sess->control_sock, MSG_503); // Bad sequence of commands
    return;
  }

  if (!args || strlen(args) == 0) {
    safe_dprintf(sess->control_sock, MSG_501); // Syntax error in parameters
    return;
  }

  if (check_credentials(sess->current_user, (char *)args) == 0) {
    sess->logged_in = 1;
    safe_dprintf(sess->control_sock, MSG_230); // User logged in
  } else {
    safe_dprintf(sess->control_sock, MSG_530); // Not logged in
    sess->current_user[0] = '\0'; // Reset user on failed login
    sess->logged_in = 0;
  }
}

void handle_QUIT(const char *args) {
  ftp_session_t *sess = session_get();
  (void)args; // unused

  safe_dprintf(sess->control_sock, MSG_221); // 221 Goodbye.
  sess->current_user[0] = '\0'; // Close session
  close_fd(sess->control_sock, "client socket"); // Close socket
  sess->control_sock = -1;
}

void handle_SYST(const char *args) {
  ftp_session_t *sess = session_get();
  (void)args; // unused

  safe_dprintf(sess->control_sock, MSG_215); // 215 <system type>
}

void handle_TYPE(const char *args) {
  ftp_session_t *sess = session_get();
  (void)args;
  (void)sess;

  if (!args || strlen(args) == 0) {
      sess->transfer_type = BINARY;
      safe_dprintf(sess->control_sock, "Default: Modo binario activado.\n");
      safe_dprintf(sess->control_sock, MSG_200);
      return;
  }

  if (strcmp(args, "I") == 0) {
      sess->transfer_type = BINARY;
      safe_dprintf(sess->control_sock, MSG_200);
  } else if (strcmp(args, "A") == 0) {
      sess->transfer_type = ASCII;
      safe_dprintf(sess->control_sock, MSG_200);
  } else {
      safe_dprintf(sess->control_sock, MSG_504);
  }
  
}

void handle_PORT(const char *args) {
  ftp_session_t *sess = session_get();
  (void)args;
  (void)sess;

  // args -> comando y argumento(s)
  // sess -> datos de la conexion de cliente

  // (1) Parsear el string: extraer los 6 argumentos del string que acompaña a PORT
  int h1, h2, h3, h4, p1, p2;
  sscanf(args, "%d,%d,%d,%d,%d,%d", &h1, &h2, &h3, &h4, &p1, &p2);
  
  char client_ip[16];
  sprintf(client_ip, "%d.%d.%d.%d", h1, h2, h3, h4);
  printf("IP: %s\n", client_ip);

  // (2) Calcular el puerto de datos: port = (p1 * 256) + p2
  int port = (p1 * 256) + p2;
  printf("Puerto: %d\n", port);

  // (3) Seteamos data_addr
  memset(&sess->data_addr, 0, sizeof(sess->data_addr));
  sess->data_addr.sin_family = AF_INET;
  sess->data_addr.sin_port = htons(port);

  // (4) Conectarse al puerto del cliente (modo activo)
  if (inet_pton(AF_INET, client_ip, &sess->data_addr.sin_addr) <= 0) {
      safe_dprintf(sess->control_sock, MSG_501);
      return;
  }

  safe_dprintf(sess->control_sock, MSG_200);

}

void handle_RETR(const char *args) {
  ftp_session_t *sess = session_get();
  (void)args;
  (void)sess;

  // printf("%s\n", args);

  if (!args || strlen(args) == 0) {
      safe_dprintf(sess->control_sock, MSG_501);  // No arguments
  }

  if (sess->data_addr.sin_port == 0) {
      safe_dprintf(sess->control_sock, MSG_503);  // Falta puerto (comando PORT)
  }

  // Open file
  FILE *file = fopen(args, "rb");
  if (!file) {
      safe_dprintf(sess->control_sock, MSG_550, args);  // Archivo no disponible
      return;
  }

  // Create socket for data transfer
  int data_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (data_socket < 0) {
      safe_dprintf(sess->control_sock, MSG_425);  // Cannot open data connection
      fclose(file);
      return;
  }

  // Establish Data Connection
  if (connect(data_socket, (struct sockaddr *)&sess->data_addr, sizeof(sess->data_addr)) < 0 ) {
      safe_dprintf(sess->control_sock, MSG_425);  // Cannot open data connection
      fprintf(stderr, "Falló la conexión de datos:\n");
      perror(NULL);
      fclose(file);
      close(data_socket);
      return;
  }

  // Inform client that connection is ready for transmision
  safe_dprintf(sess->control_sock, MSG_150);

  // Begin transmission
  char buffer[1024];
  ssize_t bytes;
  while ((bytes = fread(buffer, 1, sizeof(buffer), file)) > 0) {
      if (send(data_socket, buffer, bytes, 0) != bytes) {
        perror("Error al transmitir bloque.\n");
        break;
      }
  }

  // We close file and socket
  fclose(file);
  close(data_socket);

  // We send confirmation of transfer
  safe_dprintf(sess->control_sock, MSG_226);
  
}

void handle_STOR(const char *args) {
  ftp_session_t *sess = session_get();
  (void)args;
  (void)sess;

  // Placeholder
}

void handle_NOOP(const char *args) {
  ftp_session_t *sess = session_get();
  (void)args;
  (void)sess;

  safe_dprintf(sess->control_sock, MSG_200);

}
