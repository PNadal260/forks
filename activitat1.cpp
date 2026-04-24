#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <iostream>
#include <string> 

    int main() {


    int fd[2];
    pipe(fd);

    if (fork() == 0){
        close(fd[1]);
        char buffer[7];
        int n = read (fd[0], buffer, sizeof(buffer));
        printf(buffer);
    } else{
        close(fd[0]);
        char passwd1[] = "123456\n";
        write(fd[1], passwd1, 7);
    }

    if (fork() == 0){

    } else {
        std::string passwd2 = "qwerty";

    } 

    return 0;
}