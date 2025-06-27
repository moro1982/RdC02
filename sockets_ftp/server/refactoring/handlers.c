// handlers.c

#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
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

  // Placeholder

  
  
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

  sess->data_addr.sin_family = AF_INET;
  sess->data_addr.sin_port = htons(port);

  // (3) Conectarse al puerto del cliente (modo activo)
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

  // Placeholder
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
