#include <stdio.h>
#include "bf20.h"

void put_bf20(bf20 in)
{
    int i;
    for (i = 0; i < 20; ++i)
    {
        putchar((in & 0x80000U) ? '1' : '0');
        in <<= 1;
    }
}

int main(void)
{
    int input1;
    bf20 input2;
    union fl
    {
        float f;
        unsigned int u;
    } input3;
    bf20 input4;
    bf20 input5, input6;
    bf20 input7, input8;
    bf20 input9, input10;
    union { float f; unsigned u; } result4;
    scanf("%d\n", &input1);
    // input1 = 0b00000000000000000010000000000001;
    printf("int2bf20(%d) = 0x%05x\n", input1, int2bf20(input1));
    scanf("%x\n", &input2);
    // input2 = 0b00111111000000001000;
    printf("bf202int(0x%05x) = %d\n", input2, bf202int(input2));
    scanf("%f\n", &input3.f);
    // input3.u = 0b10000000000000000000000000000001;
    // printf("HELP: %f\n", 1.0f + input3.f);
    printf("float2bf20(%f) = 0x%05x\n", input3.f, float2bf20(input3.f));
    scanf("%x\n", &input4);
    result4.f = bf202float(input4);
    printf("bf202float(0x%05x) = %f (0x%08x)\n", input4, result4.f, result4.u);
    scanf("%x %x\n", &input5, &input6);
    // input5 = 0b00000000000000111111000000000000;
    input5 = float2bf20(3.25f);
    printf("input5: %.32f\n", bf202float(input5));
    // input6 = 0b00000000000010000000000000000001;
    input6 = float2bf20(2.11f);
    printf("input6: %.32f\n", bf202float(input6));
    put_bf20(bf20_add(input5, input6));
    printf("\n");
    printf("%.32f\n", bf202float(bf20_add(input5, input6)));
    printf("bf20_add(0x%05x, 0x%05x) = 0x%05x\n", input5, input6, bf20_add(input5, input6));
    scanf("%x %x\n", &input7, &input8);
    // input7 = 0b00000000000001111101000000000100;
    // input8 = 0b00000000000000000000000000000100;
    printf("input7: %.32f\n", bf202float(input7));
    printf("input8: %.32f\n", bf202float(input8));
    printf("i7 * i8 = %.32f\n", input7 * input8);
    printf("(bf2) i7 * i8 = %.32f\n", bf202float(bf20_mul(input7, input8)));
    printf("bf20_mul(0x%05x, 0x%05x) = 0x%05x\n", input7, input8, bf20_mul(input7, input8));
    scanf("%x %x\n", &input9, &input10);
    printf("bf20_compare(0x%05x, 0x%05x) = %d\n", input9, input10, bf20_compare(input9, input10));

    printf("put_bf20(0x%05x) is ", input1);
    put_bf20(input1);
    putchar('\n');

    return 0;
}

// 10110011110011010101001000
// 10110011110011010101001000
// 10110011110100000000000000
// 1011001111010
