#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "claves.h"
#include "lines.h"

void *atender_peticion(void *ptr) {
    /* Recogemos el socket */
    int sc = *(int *)ptr;
    free(ptr);

    printf("Petición aceptada (Socket: %d)\n", sc);
    
    char buffer[1024];
    char respuesta[1024];

    if (readLine(sc, buffer, 1024) < 0) {
        printf("Error al leer del socket %d\n", sc);
        close(sc);
        pthread_exit(NULL);
    }

    /* Tratamos de leer el primer token del buffer (la operación) */
    char *token = strtok(buffer, ";");
    if (token == NULL) {
        close(sc);
        pthread_exit(NULL);
    }

    printf("Datos recibidos: %s\n", buffer);
    
    int op = atoi(token);

    if (op == 0) { // destroy
        sprintf(respuesta, "%d", destroy());
    }
    else if (op == 1 || op == 3) { // set_value (1) o modify_value (3)
        /* Sacamos key */
        char *key = strtok(NULL, ";");

        /* Sacamos v1 */
        char *v1 = strtok(NULL, ";");

        /* Sacamos N */
        int N = atoi(strtok(NULL, ";"));

        /* Sacamos cada float uno a uno */
        float v2[32];
        for (int i = 0; i < N; i++){
            v2[i] = atof(strtok(NULL, ";"));
        }

        /* Sacamos los tres valores del struct */
        struct Paquete p;
        p.x = atoi(strtok(NULL, ";"));
        p.y = atoi(strtok(NULL, ";"));
        p.z = atoi(strtok(NULL, ";"));
        
        int res;
        if (op == 1) {
            res = set_value(key, v1, N, v2, p);
        } else {
            res = modify_value(key, v1, N, v2, p);
        } 
        
        sprintf(respuesta, "%d", res);
    }
    else if (op == 2) { // get_value
        /* Sacamos key */
        char *key = strtok(NULL, ";");

        /* Declaramos el resto de variables */
        char v1[256]; 
        int n2; 
        float vv2[32]; 
        struct Paquete v3;

        int res = get_value(key, v1, &n2, vv2, &v3);

        /* Si la clave existe, creamos la respuesta con todos los valores */
        if (res == 0) {
            sprintf(respuesta, "0;%s;%d", v1, n2);
            for (int i=0; i<n2; i++) {
                /* Añadimos cada float uno por uno */
                char tmp[32]; 
                sprintf(tmp, ";%f", vv2[i]); 
                strcat(respuesta, tmp);
            }

            /* Añadimos los valores del struct */
            char p_tmp[64]; 
            sprintf(p_tmp, ";%d;%d;%d", v3.x, v3.y, v3.z);
            
            strcat(respuesta, p_tmp);
        } else {
            sprintf(respuesta, "-1");
        }
    }
    else if (op == 4) { // delete_key
        char *key = strtok(NULL, ";");
        sprintf(respuesta, "%d", delete_key(key));
    }
    else if (op == 5) { // exist
        char *key = strtok(NULL, ";");
        sprintf(respuesta, "%d", exist(key));
    }

    sendMessage(sc, respuesta, strlen(respuesta) + 1);
    close(sc);
    pthread_exit(NULL);
}


int main(int argc, char *argv[]) {
    /* Si no pasamos el puerto, devolvemos -1 */
    if (argc != 2) {
        printf("Uso: %s <puerto>\n", argv[0]);
        return -1;
    }

    /* Creamos el socket */
    int sd = socket(AF_INET, SOCK_STREAM, 0);

    /* Cambiamos las opciones del socket para poder reutilizarlo cuando queramos */
    int val = 1;
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(int));

    /* Creamos la dirección */
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(atoi(argv[1]));

    /* Vinculamos el socket con la dirección */
    bind(sd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(sd, SOMAXCONN);

    printf("Servidor listo en puerto %s...\n", argv[1]);

    while (1) {
        /* Esperamos un cliente */
        struct sockaddr_in client_addr;
        socklen_t size = sizeof(client_addr);
        int sc = accept(sd, (struct sockaddr *)&client_addr, &size);

        printf("Ha llegado un cliente.");

        if (sc == -1){
            continue;
        }

        pthread_t th;
        int *sc_copy = malloc(sizeof(int));
        *sc_copy = sc;
        
        /* Creamos el hilo y lo soltamos para que no ocupe memoria al terminar */
        if (pthread_create(&th, NULL, atender_peticion, sc_copy) != 0) {
            close(sc);
            free(sc_copy);
        }
        pthread_detach(th);
    }
    return 0;
}
