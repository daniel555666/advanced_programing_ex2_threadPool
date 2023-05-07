
#include <stdio.h>
#include <unistd.h>

void main(){
    printf("%d",sysconf(_SC_NPROCESSORS_CONF));
}