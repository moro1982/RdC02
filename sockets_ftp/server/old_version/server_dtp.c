#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

// #include "server_dtp.h"

#define PWDFILE "/etc/ausftp/ftpusers"

bool check_credentials(char *user, char *pass) {
    FILE *file;
    char *path = PWDFILE, *line = NULL, cred[100];
    size_t len = 0;
    bool found = false;

    // make the credential string
    sprintf(cred, "%s:%s", user, pass);

    // check if it is present in any ftpusers line
    file = fopen(path, "r");
    if (file == NULL) {
        fprintf(stderr, "Error al abrir el archivo %s\n", path);
        return false;
    }
    while (getline(&line, &len, file) != -1) {
        strtok(line, "\n");
        if (strcmp(line, cred) == 0) {
            found = true;
            break;
        }
    }
    fclose(file);
    if (line) free(line);
    return found;
}
