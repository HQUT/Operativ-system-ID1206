//worked with Ayah
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <sys/wait.h> 

int main(void){
    int fd[2]; 
    if(pipe(fd) == -1){
        printf("Pipe crating failed");
        exit(EXIT_FAILURE);
    }

    pid_t child_process= fork();
    if(child_process == -1){
        printf("The fork  failed :(, no new child");
        exit(EXIT_FAILURE);
    }
    
    if(child_process > 0){
        close(fd[1]);
    dup2(fd[0], STDIN_FILENO);
    close(fd[0]);  
    wait(NULL);
    execlp("wc", "wc", "-l", NULL);
    printf("execpl failded");
    exit(EXIT_FAILURE);
    }
    
    if(child_process == 0){
        close(fd[0]);

        dup2(fd[1], STDOUT_FILENO);
        close(fd[1]);

        execlp("ls", "ls", "/", NULL);
        printf("faileddddddd");
        exit(EXIT_FAILURE);
    }

    return 0;


}
