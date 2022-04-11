#include <stdio.h>

int main(int argc, char* argv[]){
    printf("There is %d argument\n", argc);
    printf("This is the arg program, given numbers are: %s %s %s\n",argv[1],argv[2],argv[3]);
    return 0;
}