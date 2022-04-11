#include <stdio.h>
#include <unistd.h>
int main(){
    char str[100];
    int n = scanf("%s",str);
    printf("n: %d\n",n);
    printf("This is the hello1 program, %s\n",str);
    return 0;
}