#include <stdio.h> 
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

int main(){ 
    int i; 
    printf( " The real user ID is: %d\n " ,getuid()); 
    printf( " The effective user ID is: %d\n " ,geteuid()); 
    i =setuid( 100 ); 
    if ( 0 == i){ 
        printf( " .......................\n " ); 
        printf( " The real user ID is: %d\n " ,getuid()); 
        printf( " The effective user ID is: %d\n " ,geteuid()); 
        printf( " .......................\n " ); 
    } 
    else { 
        perror( " failed to setuid " ); 
        printf( " The real user ID is: %d\n " ,getuid()); 
        printf( " The effective user ID is: %d\n " ,geteuid()); 
        exit( 1 ); 
    } 
    return 0 ; 
} 
