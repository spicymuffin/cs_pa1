#ifndef F20_H
#define F20_H

typedef unsigned int bf20;

bf20 int2bf20(int in);
int  bf202int(bf20 in);
bf20 float2bf20(float in);
float bf202float(bf20 in);

bf20 bf20_add(bf20 a, bf20 b);
bf20 bf20_mul(bf20 a, bf20 b);

int bf20_compare(bf20 a, bf20 b);

// Helper function to output binary pattern of bf20 to stdin
// Be careful! This function doesn't put '\n' to stdin. 
extern void put_bf20(bf20 in);

#endif
