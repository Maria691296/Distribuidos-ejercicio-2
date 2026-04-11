#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include "claves.h"
#include "lines.h"


/* Función auxiliar para conectar con el servidor */
static int conectar_servidor() {
    char *ip = getenv("IP_TUPLAS");
    char *port = getenv("PORT_TUPLAS");
    
    /* Si no se definen las variables de entorno, error */
    if (ip == NULL || port == NULL) {
        perror("Error: Variables de entorno no definidas");
        return -1;
    }

    /* Convertimos la dirección en una estructura que entienda el hardware de red */
    struct hostent *hp = gethostbyname(ip);
    if (hp == NULL){
        return -1;
    } 

    /* Creamos el socket (TCP) */
    int sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd < 0){
        return -1;
    } 

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(port));
    memcpy(&(server_addr.sin_addr), hp->h_addr_list[0], hp->h_length);

    /* Intentamos llamar al servidor */
    if (connect(sd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        return -1;
    }
    return sd;
}

int destroy() {
    /* Llamada al servidor */
    int sd = conectar_servidor();
    if (sd < 0){
        return -1;
    }

    /* Enviamos el código de operación (0) + el '\0' (2 bytes) */
    if (sendMessage(sd, "0", 2) < 0) {    // Op 0
        close(sd);
        return -1;
    }

    /* Esperamos la respuesta del servidor (0)*/
    char res[256];
    if (readLine(sd, res, 256) < 0) {
        close(sd);
        return -1;
    }

    /* Cerramos conexión y devolvemos la respuesta */
    close(sd);
    return atoi(res);
}

int set_value(char *key, char *value1, int N_value2, float *V_value2, struct Paquete value3) {
    /* Llamada al servidor */
    int sd = conectar_servidor();
    if (sd < 0){
        return -1;
    }

    char buffer[2048];
    sprintf(buffer, "1;%s;%s;%d", key, value1, N_value2);   // Op 1
    for (int i = 0; i < N_value2; i++) {
        char tmp[32];

        /* Metemos la lista de números en el buffer */
        sprintf(tmp, ";%f", V_value2[i]);
        strcat(buffer, tmp);
    }
    char p_tmp[64];
    sprintf(p_tmp, ";%d;%d;%d", value3.x, value3.y, value3.z);
    strcat(buffer, p_tmp);

    /* Enviamos el mensaje completo (+ '\0')*/
    if (sendMessage(sd, buffer, strlen(buffer) + 1) < 0) {
        close(sd);
        return -1;
    }

    char res[256];
    if (readLine(sd, res, 256) < 0) {
        close(sd);
        return -1;
    }

    close(sd);
    return atoi(res);
}

int get_value(char *key, char *value1, int *L_value2, float *V_value2, struct Paquete *value3) {
    /* Llamada al servidor */
    int sd = conectar_servidor();
    if (sd < 0){
        return -1;
    } 

    char buffer[1024];
    sprintf(buffer, "2;%s", key);   // Op 2
    
    if (sendMessage(sd, buffer, strlen(buffer) + 1) < 0) {
        close(sd);
        return -1;
    }

    /* Recibimos la respuesta del servidor */
    char res[1024];
    if (readLine(sd, res, 1024) < 0) {
        close(sd);
        return -1;
    }

    /* Extraemos la clave */
    char *token = strtok(res, ";");
    if (token == NULL){ 
        close(sd); 
        return -1; 
    }
    
    /* Si el servidor devuelve -1 (no existe la clave) */
    int estado = atoi(token);
    if (estado != 0) {
        close(sd);
        return -1; 
    }

    /* Extraemos value1 (string) */
    char *v1_token = strtok(NULL, ";");
    if (v1_token){
        strcpy(value1, v1_token);
    } 

    /* Extraemos N (número de floats) */
    char *n_token = strtok(NULL, ";");
    if (n_token){
        *L_value2 = atoi(n_token);
    } 
    
    /* Extraemos los floats uno a uno */
    for (int i = 0; i < *L_value2; i++) {
        char *f_token = strtok(NULL, ";");
        if (f_token){
            V_value2[i] = atof(f_token);
        } 
    }
    
    /* Extraemos los campos del struct Paquete (x, y, z) */
    char *x_t = strtok(NULL, ";");
    char *y_t = strtok(NULL, ";");
    char *z_t = strtok(NULL, ";");
    
    if (x_t && y_t && z_t) {
        value3->x = atoi(x_t);
        value3->y = atoi(y_t);
        value3->z = atoi(z_t);
    }

    close(sd);
    return 0;
}

int modify_value(char *key, char *value1, int N_value2, float *V_value2, struct Paquete value3) {
    /* Llamada al servidor */
    int sd = conectar_servidor();
    if (sd < 0){
        return -1;
    } 

    char buffer[2048];
    sprintf(buffer, "3;%s;%s;%d", key, value1, N_value2);   //Op 3
    for (int i = 0; i < N_value2; i++) {
        char tmp[32];

        /* Metemos la lista de números en el buffer */
        sprintf(tmp, ";%f", V_value2[i]);
        strcat(buffer, tmp);
    }
    char p_tmp[64];
    sprintf(p_tmp, ";%d;%d;%d", value3.x, value3.y, value3.z);
    strcat(buffer, p_tmp);

    /* Enviamos el mensaje completo (+ '\0')*/
    if (sendMessage(sd, buffer, strlen(buffer) + 1) < 0) {
        close(sd);
        return -1;
    }
    
    char res[256];
    
    if (readLine(sd, res, 256) < 0) {
        close(sd);
        return -1;
    }
    
    close(sd);
    return atoi(res);
}

int delete_key(char *key) {
    /* Llamada al servidor */
    int sd = conectar_servidor();
    if (sd < 0){
        return -1;
    } 

    char buffer[256];

    /* Empaquetamos '4' (operación) y luego la clave */
    sprintf(buffer, "4;%s", key);   // Op 4

    /* Enviamos el mensaje */
    if (sendMessage(sd, buffer, strlen(buffer) + 1) < 0) {
        close(sd);
        return -1;
    }

    /* Esperamos la respuesta */
    char res[256];
    if (readLine(sd, res, 256) < 0) {
        close(sd);
        return -1;
    };

    /* Cerramos conexión con el servidor y devolvemos la respuesta (0 o -1) */
    close(sd);
    return atoi(res);
}

int exist(char *key) {
    /* Llamada al servidor */
    int sd = conectar_servidor();
    if (sd < 0){
        return -1;
    }

    char buffer[256];

    /* Empaquetamos '5' (operación) y luego la clave */
    sprintf(buffer, "5;%s", key);   // Op 5

    /* Enviamos el mensaje */
    if (sendMessage(sd, buffer, strlen(buffer) + 1) < 0) {
        close(sd);
        return -1;
    }

    /* Esperamos la respuesta */
    char res[256];
    if (readLine(sd, res, 256) < 0) {
        close(sd);
        return -1;
    }

    /* Cerramos conexión con el servidor y devolvemos la respuesta (0 o 1) */
    close(sd);
    return atoi(res);
}
