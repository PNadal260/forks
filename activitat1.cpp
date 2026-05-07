#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <iostream>
#include <string>
#include <cstring>

using namespace std;

// Función simple de cifrado
string cifrar(string texto) {
    for (int i = 0; i < texto.length(); i++) {
        texto[i] = texto[i] + 1; // desplaza cada carácter
    }
    return texto;
}

int main() {

    // Pipes hijo 1
    int padre_hijo1[2];
    int hijo1_padre[2];

    // Pipes hijo 2
    int padre_hijo2[2];
    int hijo2_padre[2];

    pipe(padre_hijo1);
    pipe(hijo1_padre);

    pipe(padre_hijo2);
    pipe(hijo2_padre);

    // ===== HIJO 1 =====
    pid_t pid1 = fork();

    if (pid1 == 0) {

        // Cerrar extremos no usados
        close(padre_hijo1[1]);
        close(hijo1_padre[0]);

        char buffer[100];

        // Leer contraseña del padre
        read(padre_hijo1[0], buffer, sizeof(buffer));

        string texto = buffer;

        // Cifrar
        string cifrado = cifrar(texto);

        // Enviar al padre
        write(hijo1_padre[1], cifrado.c_str(), cifrado.length() + 1);

        close(padre_hijo1[0]);
        close(hijo1_padre[1]);

        return 0;
    }

    // ===== HIJO 2 =====
    pid_t pid2 = fork();

    if (pid2 == 0) {

        // Cerrar extremos no usados
        close(padre_hijo2[1]);
        close(hijo2_padre[0]);

        char buffer[100];

        // Leer contraseña del padre
        read(padre_hijo2[0], buffer, sizeof(buffer));

        string texto = buffer;

        // Cifrar
        string cifrado = cifrar(texto);

        // Enviar al padre
        write(hijo2_padre[1], cifrado.c_str(), cifrado.length() + 1);

        close(padre_hijo2[0]);
        close(hijo2_padre[1]);

        return 0;
    }

    // ===== PADRE =====

    // Cerrar extremos no usados
    close(padre_hijo1[0]);
    close(hijo1_padre[1]);

    close(padre_hijo2[0]);
    close(hijo2_padre[1]);

    // Contraseñas
    string passwd1 = "123456";
    string passwd2 = "qwerty";

    // Enviar a los hijos
    write(padre_hijo1[1], passwd1.c_str(), passwd1.length() + 1);
    write(padre_hijo2[1], passwd2.c_str(), passwd2.length() + 1);

    // Buffers de recepción
    char resultado1[100];
    char resultado2[100];

    // Leer respuestas
    read(hijo1_padre[0], resultado1, sizeof(resultado1));
    read(hijo2_padre[0], resultado2, sizeof(resultado2));

    // Mostrar resultados
    cout << "Contrasenya original 1: " << passwd1 << endl;
    cout << "Contrasenya xifrada 1: " << resultado1 << endl;

    cout << "Contrasenya original 2: " << passwd2 << endl;
    cout << "Contrasenya xifrada 2: " << resultado2 << endl;

    // Esperar hijos
    wait(NULL);
    wait(NULL);

    return 0;
}