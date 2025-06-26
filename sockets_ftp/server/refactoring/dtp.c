#define _GNU_SOURCE
#include "dtp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int check_credentials(char *user, char *pass) {
    FILE *file;
    char *path = PWDFILE, *line = NULL, cred[100];
    size_t len = 0;
    int found = -1;

    // make the credential string
    sprintf(cred, "%s:%s", user, pass);

    // check if it is present in any ftpusers line
    file = fopen(path, "r");
    if (file == NULL) {
        fprintf(stderr, "Error al abrir el archivo de usuarios %s\n", path);
        return -1;
    }
    while (getline(&line, &len, file) != -1) {
        strtok(line, "\n");
        if (strcmp(line, cred) == 0) {
            found = 0;
            break;
        }
    }
    fclose(file);
    if (line) free(line);
    return found;
}
