#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdio.h>

int main(){
    int fd[2];
    pipe(fd);
    pid_t pid = fork();
    if(pid == 0){//child process
        close(fd[0]); //close read end
        dup2(fd[1],1);
        close(fd[1]);
        //printf("hello from child");
        pid_t pid2 = fork();
        if(pid2 == 0){
            printf("hello from grandchild1 ");
            return 1;
        }
        else{
            printf("hello from grandchild2 ");
            return 1;
        }
    }
    else{
        close(fd[1]); //close write end
        dup2(fd[0],0);
        close(fd[0]);
        char str[128];
        gets(str);
        printf("child said: %s\n",str);
        return 1;
    }
}