#include <stdio.h>
#include "bf20.h"

void put_bf20(bf20 in)
{
    int i;
    for ( i=0; i < 26; ++i ) {
        putchar((in & 0x8000U) ? '1' : '0');
        in <<= 1;
    }
}

int main(void)
{
    int input1;
    bf20 input2;
    float input3;
    bf20 input4;
    bf20 input5, input6;
    bf20 input7, input8;
    bf20 input9, input10;
    union {float f; unsigned u;} result4;

    scanf("%d\n", &input1);
    printf("int2bf20(%d) = 0x%05x\n", input1, int2bf20(input1));
    scanf("%x\n", &input2);
    printf("bf202int(0x%05x) = %d\n", input2, bf202int(input2));
    scanf("%f\n", &input3);
    printf("float2bf20(%f) = 0x%05x\n", input3, float2bf20(input3));
    scanf("%x\n", &input4);
    result4.f = bf202float(input4);
    printf("bf202float(0x%05x) = %f (0x%08x)\n", input4, result4.f, result4.u);
    scanf("%x %x\n", &input5, &input6);
    printf("bf20_add(0x%05x, 0x%05x) = 0x%05x\n", input5, input6, bf20_add(input5, input6));
    scanf("%x %x\n", &input7, &input8);
    printf("bf20_mul(0x%05x, 0x%05x) = 0x%05x\n", input7, input8, bf20_mul(input7, input8));
    scanf("%x %x\n", &input9, &input10);
    printf("bf20_compare(0x%05x, 0x%05x) = %d\n", input9, input10, bf20_compare(input9, input10));

    printf("put_bf20(0x%05x) is ", input1);
    put_bf20(input1);
    putchar('\n');

    return 0;
}

