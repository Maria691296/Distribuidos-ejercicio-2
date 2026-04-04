#include <stdio.h>
#include <string.h>
#include "claves.h"

#define N_CLAVES 1000

int main(void) {

    printf("Prueba de estrés (%d claves)\n", N_CLAVES);

    char key[64];
    char val[64];
    float fv[1];
    struct Paquete pk;
    int errores;

    /* Destroy inicial */
    printf("Destroy inicial\n");
    printf("  destroy() -> %d (esperado 0)\n", destroy());

    /* Insertar N_CLAVES */
    printf("Insertar %d claves\n", N_CLAVES);
    errores = 0;
    for (int i = 0; i < N_CLAVES; i++) {
        snprintf(key, sizeof(key), "clave_%d", i);
        snprintf(val, sizeof(val), "valor_%d", i);
        fv[0] = (float)i;
        pk.x = i; pk.y = i*2; pk.z = i*3;
        if (set_value(key, val, 1, fv, pk) != 0)
            errores++;
    }
    printf("  Errores al insertar: %d (esperado 0)\n", errores);

    /* Comprobar que existen todas */
    printf("Comprobar que existen todas\n");
    errores = 0;
    for (int i = 0; i < N_CLAVES; i++) {
        snprintf(key, sizeof(key), "clave_%d", i);
        if (exist(key) != 1)
            errores++;
    }
    printf("  Claves no encontradas: %d (esperado 0)\n\n", errores);

    /* Recuperar y verificar valores */
    printf("Recuperar y verificar valores\n");
    errores = 0;
    for (int i = 0; i < N_CLAVES; i++) {
        snprintf(key, sizeof(key), "clave_%d", i);
        snprintf(val, sizeof(val), "valor_%d", i);
        char   val_out[256];
        int    nv2;
        float  vv2[32];
        struct Paquete v3;
        if (get_value(key, val_out, &nv2, vv2, &v3) != 0) {
            errores++;
        } else {
            /* Verificar que los valores son correctos */
            if (strcmp(val_out, val) != 0)   errores++;
            if (vv2[0] != (float)i)          errores++;
            if (v3.x != i || v3.y != i*2 || v3.z != i*3) errores++;
        }
    }
    printf("  Errores al recuperar: %d (esperado 0)\n\n", errores);

    /* Modificar todas las claves */
    printf("Modificar todas las claves\n");
    errores = 0;
    for (int i = 0; i < N_CLAVES; i++) {
        snprintf(key, sizeof(key), "clave_%d", i);
        fv[0] = (float)(i + 1000);
        pk.x = i+1; pk.y = i+2; pk.z = i+3;
        if (modify_value(key, "modificado", 1, fv, pk) != 0)
            errores++;
    }
    printf("  Errores al modificar: %d (esperado 0)\n", errores);

    /* Borrar todas las claves */
    printf("Borrar todas las claves\n");
    errores = 0;
    for (int i = 0; i < N_CLAVES; i++) {
        snprintf(key, sizeof(key), "clave_%d", i);
        if (delete_key(key) != 0)
            errores++;
    }
    printf("  Errores al borrar: %d (esperado 0)\n\n", errores);

    /* Comprobar que ya no existen */
    printf("Comprobar que ya no existen\n");
    errores = 0;
    for (int i = 0; i < N_CLAVES; i++) {
        snprintf(key, sizeof(key), "clave_%d", i);
        if (exist(key) != 0)
            errores++;
    }
    printf("  Claves que siguen existiendo: %d (esperado 0)\n\n", errores);

    /* Volver a insertar y destruir con destroy */
    printf("Reinsertar y destruir con destroy\n");
    errores = 0;
    for (int i = 0; i < N_CLAVES; i++) {
        snprintf(key, sizeof(key), "clave_%d", i);
        snprintf(val, sizeof(val), "valor_%d", i);
        fv[0] = (float)i;
        pk.x = i; pk.y = i; pk.z = i;
        if (set_value(key, val, 1, fv, pk) != 0)
            errores++;
    }
    printf("  Errores al reinsertar: %d (esperado 0)\n", errores);
    printf("  destroy() -> %d (esperado 0)\n", destroy());
    snprintf(key, sizeof(key), "clave_0");
    printf("  exist clave_0 tras destroy -> %d (esperado 0)\n\n", exist(key));

    printf("Fin de la prueba de estrés\n");
    return 0;
}
