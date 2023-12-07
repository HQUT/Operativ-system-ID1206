#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>


#define MSG_SIZE 1024
// definera a message structure
struct message {
    long msg_type;
    char msg_text[MSG_SIZE];
};

int wordcounting(const char *str){
    int count = 0, inWord = 0;

    while(*str){
        if(*str == ' ' || *str == '\n' || *str == '\t'){
            inWord = 0;
        }else if (!inWord) {
            inWord = 1;
            count++;
        }
        str++;
    }
    return count;
}

int main(){
    int msgID;
    pid_t new_child;
  
    msgID=msgget(IPC_PRIVATE, 0666  | IPC_CREAT);

    new_child = fork();

    if (new_child < 0){
        printf("no new child :( sad");
        exit(EXIT_FAILURE);
    }
    if(new_child == 0){
        struct message msg;
        int totalWords = 0;

    while(1){
        msgrcv(msgID, &msg, sizeof(msg.msg_text), 1, 0);
       if(strcmp(msg.msg_text, "wow, End of transmission") == 0){
        break;
       }
        totalWords += wordcounting(msg.msg_text);
    }
    
    printf("Total words: %d\n", totalWords);
    msgctl(msgID, IPC_RMID, NULL );
    exit(0);
    }else{
     struct message msg;
     FILE *file = fopen("filen.txt", "r");
     if(file== NULL){
        printf("off error");
        exit(EXIT_FAILURE);
     }
      while(fgets(msg.msg_text, MSG_SIZE, file) != NULL){
        msg.msg_type = 1;
        msgsnd(msgID, &msg, sizeof(msg.msg_text), 0);
    }
    
    strcpy(msg.msg_text, "wow, End of transmission");
    msgsnd(msgID, &msg, sizeof(msg.msg_text), 0);
    
    fclose(file);
    wait(NULL);
      msgctl(msgID, IPC_RMID, NULL);
    }
    return 0;
}
