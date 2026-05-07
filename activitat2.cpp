#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <cstring>
#include <fstream>
#include <ctime>

#define BUFFER_SIZE 512
#define WINDOW_SECONDS 10

pid_t pidA = -1;
pid_t pidB = -1;
pid_t pidC = -1;

/*
 * XOR simple per xifrar/desxifrar
 */
std::string xor_encrypt(const std::string &text, char key = 'K') {
    std::string result = text;

    for (char &c : result) {
        c ^= key;
    }

    return result;
}

/*
 * Handler SIGINT
 */
void sigint_handler(int sig) {

    std::cout << "\n[PARA] Finalitzacio controlada...\n";

    if (pidA > 0) kill(pidA, SIGTERM);
    if (pidB > 0) kill(pidB, SIGTERM);
    if (pidC > 0) kill(pidC, SIGTERM);
}

/*
 * =========================
 * PROCÉS A — CAPTURADOR
 * =========================
 */
void processA(int pipeAB[2]) {

    close(pipeAB[0]);

    std::string line;

    while (true) {

        std::cout << "[A] Introdueix text: ";
        std::getline(std::cin, line);

        if (line == "ESC" || line == "sortir") {

            write(pipeAB[1], "EXIT\n", 5);

            break;
        }

        line += "\n";

        write(pipeAB[1], line.c_str(), line.size());
    }

    close(pipeAB[1]);

    exit(0);
}

/*
 * =========================
 * PROCÉS B — DETECTOR
 * =========================
 */
void processB(int pipeAB[2], int pipeBC[2]) {

    close(pipeAB[1]);
    close(pipeBC[0]);

    char buffer[BUFFER_SIZE];

    bool capture_window = false;
    time_t capture_start = 0;

    std::ofstream hiddenFile("registre_b_xifrat.txt");

    while (true) {

        memset(buffer, 0, BUFFER_SIZE);

        int n = read(pipeAB[0], buffer, BUFFER_SIZE);

        if (n <= 0)
            break;

        std::string msg(buffer);

        if (msg == "EXIT\n") {

            write(pipeBC[1], "EXIT\n", 5);

            break;
        }

        /*
         * Detecta '@'
         */
        if (msg.find('@') != std::string::npos) {

            capture_window = true;
            capture_start = time(nullptr);

            std::cout << "[B] '@' detectat -> finestra oberta "
                      << WINDOW_SECONDS
                      << " segons\n";
        }

        /*
         * Xifra
         */
        std::string encrypted = xor_encrypt(msg);

        /*
         * Si finestra activa
         */
        if (capture_window) {

            time_t now = time(nullptr);

            if ((now - capture_start) <= WINDOW_SECONDS) {

                write(pipeBC[1],
                      encrypted.c_str(),
                      encrypted.size());

            } else {

                capture_window = false;

                std::cout << "[B] Finestra tancada\n";
            }
        }

        /*
         * Sempre guarda al fitxer local xifrat
         */
        hiddenFile << encrypted;
    }

    hiddenFile.close();

    close(pipeAB[0]);
    close(pipeBC[1]);

    exit(0);
}

/*
 * =========================
 * PROCÉS C — PROCESSADOR
 * =========================
 */
void processC(int pipeBC[2]) {

    close(pipeBC[1]);

    char buffer[BUFFER_SIZE];

    std::ofstream outputFile("registre_c.txt");

    while (true) {

        memset(buffer, 0, BUFFER_SIZE);

        int n = read(pipeBC[0], buffer, BUFFER_SIZE);

        if (n <= 0)
            break;

        std::string msg(buffer);

        if (msg == "EXIT\n")
            break;

        outputFile << msg;
    }

    outputFile.close();

    close(pipeBC[0]);

    exit(0);
}

/*
 * =========================
 * MAIN
 * =========================
 */
int main() {

    signal(SIGINT, sigint_handler);

    int pipeAB[2];
    int pipeBC[2];

    if (pipe(pipeAB) == -1) {

        perror("pipeAB");
        return 1;
    }

    if (pipe(pipeBC) == -1) {

        perror("pipeBC");
        return 1;
    }

    /*
     * Crear Procés C
     */
    pidC = fork();

    if (pidC == 0) {

        processC(pipeBC);
    }

    /*
     * Crear Procés B
     */
    pidB = fork();

    if (pidB == 0) {

        processB(pipeAB, pipeBC);
    }

    /*
     * Crear Procés A
     */
    pidA = fork();

    if (pidA == 0) {

        processA(pipeAB);
    }

    /*
     * Pare
     */
    close(pipeAB[0]);
    close(pipeAB[1]);

    close(pipeBC[0]);
    close(pipeBC[1]);

    /*
     * Espera fills
     */
    waitpid(pidA, nullptr, 0);

    /*
     * Si A acaba -> tancar sistema
     */
    kill(pidB, SIGTERM);
    kill(pidC, SIGTERM);

    waitpid(pidB, nullptr, 0);
    waitpid(pidC, nullptr, 0);

    std::cout << "\n[PARA] Sistema finalitzat correctament.\n";

    return 0;
}