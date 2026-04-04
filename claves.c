#include "claves.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>  

/* Estructura interna del nodo para la lista enlazada */
struct Nodo {
    char key[256];
    char value1[256];
    int N_value2;
    float value2[32];
    struct Paquete value3;
    struct Nodo *sig;
};

/* Puntero al inicio de la lista */
struct Nodo *lista_claves = NULL;

/* Mutex para garantizar atomicidad en el servidor concurrente */
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;   // Como no tenemos main lo inicializamos asi de manera global

int destroy(void) {
    pthread_mutex_lock(&mutex);

    /* Empezamos a recorrer la lista desde el principio */
    struct Nodo *actual = lista_claves;
    struct Nodo *siguiente;

    /* Bucle hasta recorrer la lista entera */
    while (actual != NULL) {
        /* Marcamos cuál es el siguiente nodo */
        siguiente = actual->sig;

        /* Liberamos memoria */
        free(actual);

        /* Pasamos al nodo siguiente */
        actual = siguiente;
    }
    /* La lista vuelve a estar vacía */
    lista_claves = NULL;

    pthread_mutex_unlock(&mutex);

    return 0;
}

int set_value(char *key, char *value1, int N_value2, float *V_value2, struct Paquete value3) {
    /* Si el valor de N no está entre 1 y 32, da un error */
    if (N_value2 < 1 || N_value2 > 32){
        return -1;
    }

    /* Si el tamaño de value1 se sobrepasa, da un error */
    if (strlen(value1) > 255){
        return -1;
    }

    pthread_mutex_lock(&mutex); 

    /* Si la clave ya existe, no podemos duplicarla */
    struct Nodo *aux = lista_claves;
    while (aux != NULL) {
        if (strcmp(aux->key, key) == 0) {
            pthread_mutex_unlock(&mutex);
            return -1;
        }
        
        aux = aux->sig;
    }

    /* Hacemos reserva de memoria */
    struct Nodo *nuevo = (struct Nodo *)malloc(sizeof(struct Nodo));
    if (nuevo == NULL){
        pthread_mutex_unlock(&mutex);
        return -1;
    }
   
    /* Copiamos los datos al nuevo nodo */
    strcpy(nuevo->key, key);
    strcpy(nuevo->value1, value1);
    nuevo->N_value2 = N_value2;

    for (int i = 0; i < N_value2; i++){
        nuevo->value2[i] = V_value2[i];
    }
    
    nuevo->value3 = value3;

    /* Metemos el nodo en la lista enlazada */
    nuevo->sig = lista_claves;
    lista_claves = nuevo;

    pthread_mutex_unlock(&mutex);
    return 0;
}

int get_value(char *key, char *value1, int *N_value2, float *V_value2, struct Paquete *value3) {
    pthread_mutex_lock(&mutex);

    /* Empezamos a recorrer la lista por el principio */
    struct Nodo *actual = lista_claves;

    /* Repetimos el bucle hasta recorrer toda la lista */
    while (actual != NULL) {
        /* Si la key que buscamos es la que hemos encontrado nos metemos a la condición */
        if (strcmp(actual->key, key) == 0) {
            /* Copiamos los datos del nodo a nuestras variables */
            strcpy(value1, actual->value1);
            *N_value2 = actual->N_value2;
            
            for (int i = 0; i < actual->N_value2; i++){
                V_value2[i] = actual->value2[i];
            }

            *value3 = actual->value3;

            pthread_mutex_unlock(&mutex);

            return 0;
        }
        /* Pasamos al siguiente nodo */
        actual = actual->sig;
    }
    /* Si llegamos hasta aquí, no existe el nodo que buscábamos */
    pthread_mutex_unlock(&mutex);
    return -1;
}

int modify_value(char *key, char *value1, int N_value2, float *V_value2, struct Paquete value3) {
    if (N_value2 < 1 || N_value2 > 32){
        return -1;
    }

    pthread_mutex_lock(&mutex); 

    struct Nodo *actual = lista_claves;
    while (actual != NULL) {
        if (strcmp(actual->key, key) == 0) {
            strcpy(actual->value1, value1);
            actual->N_value2 = N_value2;
            for (int i = 0; i < N_value2; i++){
                actual->value2[i] = V_value2[i];
            }

            actual->value3 = value3;

            pthread_mutex_unlock(&mutex); 

            return 0;
        }
        actual = actual->sig;
    }
    pthread_mutex_unlock(&mutex);
    return -1;
}

int delete_key(char *key) {
    pthread_mutex_lock(&mutex);

    /* Empezamos a recorrer la lista por el principio y creamos un nodo que apunte al anterior */
    struct Nodo *actual = lista_claves;
    struct Nodo *anterior = NULL;

    /* Nos metemos en el bucle hasta recorrer toda la lista */
    while (actual != NULL) {
        /* Si la key es la que buscamos, nos metemos en la condición */
        if (strcmp(actual->key, key) == 0) {
            /* Si el nodo es el primero, ahora el primer nodo de la lista es el siguiente */
            if (anterior == NULL){
                lista_claves = actual->sig;
            }
            /* En otro caso, unimos el nodo anterior con el siguiente */
            else {
                anterior->sig = actual->sig;
            }
            /* Liberamos memoria */
            free(actual);

            pthread_mutex_unlock(&mutex);

            return 0;
        }
        /* Recorremos el siguiente nodo */
        anterior = actual;
        actual = actual->sig;
    }
    /* Si llegamos aquí, el nodo buscado no existía */
    pthread_mutex_unlock(&mutex);
    return -1;
}

int exist(char *key) {
    pthread_mutex_lock(&mutex);

    /* Empezamos por el inicio de la lista */
    struct Nodo *actual = lista_claves;

    /* Estamos en el bucle hasta recorrer la lista entera */
    while (actual != NULL) {
        /* Si encontramos la key, salimos del bucle y devolvemos 1 (encontrada)*/
        if (strcmp(actual->key, key) == 0){
            pthread_mutex_unlock(&mutex);
            return 1;
        }

        /* Pasamos al siguiente nodo */
        actual = actual->sig;
    }
    /* Si llegamos aquí, no se ha encontrado el nodo */
    pthread_mutex_unlock(&mutex); 
    return 0;
}