#include <stdio.h>
#include <unistd.h>
int main(){
    char str[100];
    gets(str);
    printf("This is the hello2 program, %s\n",str);
    return 0;
}