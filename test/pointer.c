#include <stdio.h>
#include <stdlib.h>

int main() {
    int c = 1;
    int *b = &c;
    int **const a = &b;
    int d = 2;
    int *e = &d;

    printf("a = %x -> %x -> %x\n", a, *a, **a);
    printf("b = %x -> %x\n", b, *b);
    printf("c = %x\n", c);

    //c = 2;
    //*b = 2;
    **a = 2;
    printf("a = %x -> %x -> %x\n", a, *a, **a);
    printf("b = %x -> %x\n", b, *b);
    printf("c = %x\n", c);
}
