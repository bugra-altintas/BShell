#include <stdio.h>
#include <unistd.h>

int main(){
    char str[1];
    printf("READER: ");
    while(read(STDIN_FILENO,str,1) != 0){
        printf("%s", str);                         
    }
    printf("**********************\n");

}