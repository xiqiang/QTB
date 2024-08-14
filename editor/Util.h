#ifndef Editor_Util
#define Editor_Util

#include <stdlib.h>
#include "PerfTool.h"

const float PI = 3.14159265f;
const float PI2 = PI * 2;

PerfTool        perfTool;

static int      nCreateBushCount = 0;
static double   dCreateBushTime = 0;
static double   dCreateBushTimeTotal = 0;
static double   dCreateBushTimeAvg = 0;

static int      nRemoveBushCount = 0;
static double   dRemoveBushTime = 0;
static double   dRemoveBushTimeTotal = 0;
static double   dRemoveBushTimeAvg = 0;

int RangeRand(int min, int max)
{
    return rand() % (max - min) + min;
}

float RangeRand(float min, float max)
{
    return rand() / (float)RAND_MAX * (max - min) + min;
}

unsigned int CreateBush(qtb::Land* land, const qtb::Area& area)
{
    if (!land)
        return -1;

    unsigned int bushID = -1;

    perfTool.Start();
    bushID = land->createBush(area);
    dCreateBushTime = perfTool.End();

    dCreateBushTimeTotal += dCreateBushTime;
    ++nCreateBushCount;
    dCreateBushTimeAvg = dCreateBushTimeTotal / nCreateBushCount;

    return bushID;
}

bool RemoveBush(qtb::Land* land, unsigned int bushID)
{
    if (!land)
        return false;

    bool removed = false;

    perfTool.Start();
    removed = land->removeBush(bushID);
    dRemoveBushTime = perfTool.End();

    dRemoveBushTimeTotal += dRemoveBushTime;
    ++nRemoveBushCount;
    dRemoveBushTimeAvg = dRemoveBushTimeTotal / nRemoveBushCount;

    return removed;
}

#endif
