#include <stdio.h>
#include "claves.h"

int main(void) {

    printf("Plan de pruebas\n");

    /* Destroy: inicializa el servicio */
    printf("Inicializamos el servicio\n");
    int r = destroy();
    printf("  destroy()          -> %d (esperado 0)\n", r);

    /* Set value: insertar tuplas */
    printf("Insetamos tuplas\n");

    float v2a[] = {1.0, 2.0, 3.0};
    struct Paquete p1;
    p1.x = 10; p1.y = 20; p1.z = 30;
    r = set_value("clave1", "valor_uno", 3, v2a, p1);
    printf("  set clave1         -> %d (esperado  0)\n", r);

    float v2b[] = {9.9};
    struct Paquete p2;
    p2.x = -1; p2.y = 0; p2.z = 1;
    r = set_value("clave2", "valor_dos", 1, v2b, p2);
    printf("  set clave2         -> %d (esperado  0)\n", r);

    r = set_value("clave1", "otro", 1, v2b, p2);
    printf("  set clave1 dup     -> %d (esperado -1)\n", r);

    r = set_value("clave3", "x", 0, v2b, p2);
    printf("  set N=0            -> %d (esperado -1)\n", r);
    r = set_value("clave3", "x", 33, v2b, p2);
    printf("  set N=33           -> %d (esperado -1)\n\n", r);

    /* Comprobamos exist */
    printf("Comprobando si existen las claves\n");
    printf("  exist clave1       -> %d (esperado  1)\n", exist("clave1"));
    printf("  exist clave2       -> %d (esperado  1)\n", exist("clave2"));
    printf("  exist no_existe    -> %d (esperado  0)\n\n", exist("no_existe"));

    /* Comprobamos get_value */
    printf("Obtenemos valores\n");
    {
        char val1[256];
        int nv2;
        float vv2[32];
        struct Paquete v3;

        r = get_value("clave1", val1, &nv2, vv2, &v3);
        printf("  get clave1         -> %d (esperado  0)\n", r);
        if (r == 0) {
            printf("    value1='%s'\n", val1);
            printf("    N=%d floats:", nv2);
            for (int i = 0; i < nv2; i++) printf(" %.1f", vv2[i]);
            printf("\n");
            printf("    paquete: x=%d y=%d z=%d\n", v3.x, v3.y, v3.z);
        }

        r = get_value("no_existe", val1, &nv2, vv2, &v3);
        printf("  get no_existe      -> %d (esperado -1)\n\n", r);
    }

    /* Comprobamos modify_value */
    printf("Modificamos valores\n");
    {
        float mod[] = {7.7, 8.8};
        struct Paquete pm;
        pm.x = 100; pm.y = 200; pm.z = 300;

        r = modify_value("clave1", "nuevo_valor", 2, mod, pm);
        printf("  modify clave1      -> %d (esperado  0)\n", r);

        char val1[256]; int nv2; float vv2[32]; struct Paquete v3;
        get_value("clave1", val1, &nv2, vv2, &v3);
        printf("    value1='%s'\n", val1);
        printf("    N=%d floats:", nv2);
        for (int i = 0; i < nv2; i++) printf(" %.1f", vv2[i]);
        printf("\n");
        printf("    paquete: x=%d y=%d z=%d\n", v3.x, v3.y, v3.z);

        r = modify_value("no_existe", "v", 1, mod, pm);
        printf("  modify no_existe   -> %d (esperado -1)\n", r);

        r = modify_value("clave1", "v", 0, mod, pm);
        printf("  modify N=0         -> %d (esperado -1)\n\n", r);
    }

    /* Comprobamos delete_key */
    printf("Borramos claves\n");
    r = delete_key("clave2");
    printf("  delete clave2      -> %d (esperado  0)\n", r);
    printf("  exist clave2       -> %d (esperado  0)\n", exist("clave2"));
    r = delete_key("clave2");
    printf("  delete clave2 otra -> %d (esperado -1)\n", r);
    r = delete_key("no_existe");
    printf("  delete no_existe   -> %d (esperado -1)\n\n", r);

    /* Con destroy limpiamos todo */
    printf("Limpiamos todo\n");
    destroy();
    printf("  exist clave1 tras destroy -> %d (esperado 0)\n\n", exist("clave1"));

    printf("Fin de las pruebas\n");
    return 0;
}
