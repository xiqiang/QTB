#ifndef Editor_Util
#define Editor_Util

#include <stdlib.h>

const float PI = 3.14159265f;
const float PI2 = PI * 2;

int RangeRand(int min, int max)
{
    return rand() % (max - min) + min;
}

float RangeRand(float min, float max)
{
    return rand() / (float)RAND_MAX * (max - min) + min;
}

#endif
