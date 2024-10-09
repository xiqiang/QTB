#ifndef Editor_Util
#define Editor_Util

#include <stdlib.h>
#include "PerfTool.h"

const float PI = 3.14159265f;
const float PI2 = PI * 2;

const float AREA_SIZE_MIN = 1.0f;
const float AREA_SIZE_MAX = 5.0f;

const int   PER_ROBOT_AREA = 2000;
const int   PER_BUSH_AREA = 1000;

PerfTool    perfTool;

int         nCreateBushCount = 0;
double      dCreateBushTime = 0;
double      dCreateBushTimeTotal = 0;
double      dCreateBushTimeAvg = 0;

int         nRemoveBushCount = 0;
double      dRemoveBushTime = 0;
double      dRemoveBushTimeTotal = 0;
double      dRemoveBushTimeAvg = 0;

int         nBushCrossCount = 0;
double      dBushCrossTime = 0;
double      dBushCrossTimeTotal = 0;
double      dBushCrossTimeAvg = 0;

float       appTime = 0;
float       lastFrameTime = 0;
float       initFrameTime = 0;
int         frameCount = 0;
int         frameCountTotal = 0;
float       fps = 0;
float       fpsAvg = 0;

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

bool BushContains(qtb::Land* land, float x, float y, unsigned int* bushGroupID = NULL, unsigned int* bushID = NULL)
{
    if (!land)
        return false;

    bool ret = false;

    perfTool.Start();
    ret = land->bushTest(x, y, bushGroupID, bushID);
    dBushCrossTime = perfTool.End();

    dBushCrossTimeTotal += dBushCrossTime;
    ++nBushCrossCount;
    dBushCrossTimeAvg = dBushCrossTimeTotal / nBushCrossCount;

    return ret;
}

#endif
