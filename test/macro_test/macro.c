#include <stdio.h> 
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include <string.h>

#define xxx __func__
#define D1(x) x
#define S1(x) #x
#define S2(x) S1(x)
#define f3(x, y) x##y
#define f2(x, y) f3(#x, y)
#define f1(x,y) f2(x, y)
#define f0(x) f1(x,vvv)
#define LOCATION __FILE__":"
#define MY(function) __FILE__""function
#define JOIN2(y) #y
#define JOIN(x, y) x y

#define PPCAT_NX(A, B) A ## B
#define PPCAT(A, B) PPCAT_NX(A, B)
#define STRINGIZE_NX(A) #A
#define STRINGIZE(A) STRINGIZE_NX(A)

char tmp[1024];

void foo(const char *name) {
    strcpy(tmp, name);
}

int main () {

    //for(uint8_t i = 0; i < 10; i++) printf("%"PRIu8 "\n", i);
    //foo(LOCATION);

    foo(__FILE__);
    foo(S1(__FILE__));
    foo(S2(__FILE__));

    //foo(__func__);
    //foo(S1(__func__));
    //foo(S2(__func__));

    //foo(f0(xxx));
    //foo(PPCAT(__func__, __func__));

    printf("%s\n", tmp);
    return 0;
}
