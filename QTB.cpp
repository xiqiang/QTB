// QTB.cpp : 定义应用程序的入口点。
//

#include <time.h>
#include <objidl.h>
#include <gdiplus.h>
#include <windowsx.h>

#define _CRTDBG_MAP_ALLOC
#define _CRT_SECURE_NO_WARNINGS
#include <crtdbg.h>

#include <list>

#include "framework.h"
#include "QTB.h"
#include "base/Land.h"
#include "base/Debug.h"
#include "editor/Util.h"
#include "editor/DrawData.h"
#include "editor/PerfTool.h"
#include "editor/Robot.h"

#pragma comment (lib,"Gdiplus.lib")
using namespace Gdiplus;

#define MAX_LOADSTRING 100

#ifdef _DEBUG
const int   LAND_WIDTH      = 2000;
const int   LAND_HEIGHT     = 2000;
#else
const int   LAND_WIDTH      = 4000;
const int   LAND_HEIGHT     = 4000;
#endif
const int   MIN_ZONE_SIZE   = 10;

#define STATIC_AREA_COUNT   (LAND_WIDTH * LAND_HEIGHT / PER_BUSH_AREA)
#define ROBOT_COUNT         (LAND_WIDTH * LAND_HEIGHT / PER_ROBOT_AREA)
#define DYNAMIC_AREA_COUNT  (STATIC_AREA_COUNT >> 1)

const int ZONE_VIEW_QTREE       = 1 << 0;
const int ZONE_VIEW_STATICAREAS = 1 << 1;
const int ZONE_VIEW_STATICBUSH  = 1 << 2;
const int ZONE_VIEW_DYNAMICBUSH = 1 << 3;
const int ZONE_VIEW_BUSHGROUP   = 1 << 4;

// 全局变量:
HINSTANCE hInst;                                // 当前实例
HWND hWndMain;
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名

GdiplusStartupInput gdiplusStartupInput;
ULONG_PTR           gdiplusToken;
Graphics*           graphics = NULL;

HDC                 hdcMem = NULL;
HBITMAP             hbmMem = NULL;
HBITMAP             hbmOld = NULL;

DrawData            drawData;
qtb::Land*          land = NULL;

std::list<Robot>        robotList;
std::list<unsigned int> bushIDList;

RECT                rcClient;
RectF               rcViewport;
float               viewportScale = 0.7f;

float               viewZoom = 1.0f;
PointF              viewPosCache;
PointF              viewPos;

size_t              visibleZone = 0;

bool                bLMouseDown = false;
bool                bRMouseDown = false;
bool                bMMouseDown = false;
Point               mouseDownPosL;
Point               mouseUpPosL;
Point               mouseDownPosM;
Point               mouseUpPosM;
Point               cursorPos;
PointF              cursorScalePos;
unsigned int        cursorBushGroupID = -1;
unsigned int        cursorBushID = -1;

int                 zoneViewFlag = ZONE_VIEW_QTREE | ZONE_VIEW_STATICAREAS | ZONE_VIEW_DYNAMICBUSH;
bool                bViewSelectedBushGroup = true;
bool                bViewRobot = true;
bool                bViewMask = true;

bool                bEnableRobot = true;
bool                bRobotRandMove = true;
bool                bRobotAutoBush = true;

int                 nRebuildCount = 0;
double              dRebuildTime = 0;
double              dRebuildTimeTotal = 0;
double              dRebuildTimeAvg = 0;

// 此代码模块中包含的函数的前向声明:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
VOID                TermInstance(HINSTANCE);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

VOID                ResetViewport(HWND hWnd);
VOID                UpdateViewportRect(HWND hWnd);
VOID                RefreshMenuCheck(HWND hWnd);

VOID                InitLand();
VOID                TermLand();
VOID                InitRobot();
VOID                EnableRobotMove(bool value);
VOID                EnableRobotBush(bool value);
VOID                RandomStaticBush();
VOID                RandomDynamicBush();

VOID                OnUpdate();
VOID                GetMouseArea(qtb::Area& area);
VOID                MouseBushHit();
VOID                RobotTick(float appTime);

VOID                PrepareMemBitmap(HWND hWnd);
VOID                CleanupMemBitmap();
VOID                OnPaint(HWND hWnd, PAINTSTRUCT* ps);
VOID                DrawMain();
VOID                DrawZones();
VOID                DrawRobot();
VOID                DrawSelectedBushGroup();
VOID                DrawMouseOperate();
VOID                DrawMask();
VOID                DrawTexts();

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 在此处放置代码。
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

   // Initialize GDI+.
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    // 初始化全局字符串
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_QTB, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 执行应用程序初始化:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_QTB));

    MSG msg;

    // 主消息循环:
    while (TRUE)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
                break;

            if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else
        {
            OnUpdate();
            InvalidateRect(hWndMain, NULL, FALSE);
        }
    }

    CleanupMemBitmap();
    GdiplusShutdown(gdiplusToken);
    TermInstance(hInstance);
    //_CrtDumpMemoryLeaks();

    return (int) msg.wParam;
}


//
//  函数: MyRegisterClass()
//
//  目标: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_QTB));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_QTB);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目标: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 将实例句柄存储在全局变量中

   hWndMain = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWndMain)
   {
      return FALSE;
   }

   ShowWindow(hWndMain, nCmdShow);
   UpdateWindow(hWndMain);

   appTime = GetTickCount64() / 1000.0f;
   initFrameTime = appTime;

   RefreshMenuCheck(hWndMain);

   InitLand();
   InitRobot();
   ResetViewport(hWndMain);

   return TRUE;
}

VOID TermInstance(HINSTANCE hInstance)
{
    TermLand();
}
//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目标: 处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            HMENU hmenuBar = GetMenu(hWnd);
            int wmId = LOWORD(wParam);
            // 分析菜单选择:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            case ID_LAND_CLEAR:
                if (land)
                    land->clear();
                bushIDList.clear();
                break;
            case IDM_LAND_RANDOMSTATICBUSH:
                RandomStaticBush();
                break;
            case ID_LAND_RANDOMDYNAMICAREAS:
                RandomDynamicBush();
                break;
            case ID_LAND_RANDOMREMOVEBUSH:
                for (int i = 0; i < DYNAMIC_AREA_COUNT; ++i)
                {
                    size_t size = bushIDList.size();
                    if (0 == size)
                        break;

                    std::list<unsigned int>::iterator it = bushIDList.begin();
                    std::advance(it, rand() % size);
                    if(RemoveBush(land, *it))
                        bushIDList.erase(it);
                }
                break;
            case ID_VIEW_RESET:
                {
                    ResetViewport(hWnd);
                }
                break;
            case ID_VIEW_RESETAVG:
                {
                    appTime = GetTickCount64() / 1000.0f;
                    initFrameTime = appTime;
                    frameCountTotal = 0;

                    nCreateBushCount = 0;
                    dCreateBushTime = 0;
                    dCreateBushTimeTotal = 0;
                    dCreateBushTimeAvg = 0;

                    nRemoveBushCount = 0;
                    dRemoveBushTime = 0;
                    dRemoveBushTimeTotal = 0;
                    dRemoveBushTimeAvg = 0;

                    nBushCrossCount = 0;
                    dBushCrossTime = 0;
                    dBushCrossTimeTotal = 0;
                    dBushCrossTimeAvg = 0;

                    nRebuildCount = 0;
                    dRebuildTime = 0;
                    dRebuildTimeTotal = 0;
                    dRebuildTimeAvg = 0;
                }
                break;
            case ID_VIEW_QTREE:
                {
                    zoneViewFlag ^= ZONE_VIEW_QTREE;
                    UINT check = (zoneViewFlag & ZONE_VIEW_QTREE) ? MF_CHECKED : MF_UNCHECKED;
                    CheckMenuItem(hmenuBar, ID_VIEW_QTREE, MF_BYCOMMAND | check);
                }
                break;
            case ID_VIEW_STATICAREAS:
                {
                    zoneViewFlag ^= ZONE_VIEW_STATICAREAS;
                    UINT check = (zoneViewFlag & ZONE_VIEW_STATICAREAS) ? MF_CHECKED : MF_UNCHECKED;
                    CheckMenuItem(hmenuBar, ID_VIEW_STATICAREAS, MF_BYCOMMAND | check);
                }
                break;
            case ID_VIEW_STATICBUSH:
                {
                    zoneViewFlag ^= ZONE_VIEW_STATICBUSH;
                    UINT check = (zoneViewFlag & ZONE_VIEW_STATICBUSH) ? MF_CHECKED : MF_UNCHECKED;
                    CheckMenuItem(hmenuBar, ID_VIEW_STATICBUSH, MF_BYCOMMAND | check);
                }
                break;
            case ID_VIEW_DYNAMICBUSH:
                {
                    zoneViewFlag ^= ZONE_VIEW_DYNAMICBUSH;
                    UINT check = (zoneViewFlag & ZONE_VIEW_DYNAMICBUSH) ? MF_CHECKED : MF_UNCHECKED;
                    CheckMenuItem(hmenuBar, ID_VIEW_DYNAMICBUSH, MF_BYCOMMAND | check);
                }
                break;
            case ID_VIEW_SELECTEDBUSHGROUP:
                {
                    bViewSelectedBushGroup = !bViewSelectedBushGroup;
                    UINT check = bViewSelectedBushGroup ? MF_CHECKED : MF_UNCHECKED;
                    CheckMenuItem(hmenuBar, ID_VIEW_SELECTEDBUSHGROUP, MF_BYCOMMAND | check);
                }
                break;
            case ID_VIEW_BUSHGROUP:
                {
                    zoneViewFlag ^= ZONE_VIEW_BUSHGROUP;
                    UINT check = (zoneViewFlag & ZONE_VIEW_BUSHGROUP) ? MF_CHECKED : MF_UNCHECKED;
                    CheckMenuItem(hmenuBar, ID_VIEW_BUSHGROUP, MF_BYCOMMAND | check);
                }
                break;
            case ID_ENABLE_ROBOT:
                {
                    bEnableRobot = !bEnableRobot;
                    UINT check = bEnableRobot ? MF_CHECKED : MF_UNCHECKED;
                    CheckMenuItem(hmenuBar, ID_ENABLE_ROBOT, MF_BYCOMMAND | check);
                }
                break;
            case ID_VIEW_ROBOT:
                {
                    bViewRobot = !bViewRobot;
                    UINT check = bViewRobot ? MF_CHECKED : MF_UNCHECKED;
                    CheckMenuItem(hmenuBar, ID_VIEW_ROBOT, MF_BYCOMMAND | check);
                }
                break;
            case ID_ROBOT_RANDMOVE:
                {
                    bRobotRandMove = !bRobotRandMove;
                    UINT check = bRobotRandMove ? MF_CHECKED : MF_UNCHECKED;
                    CheckMenuItem(hmenuBar, ID_ROBOT_RANDMOVE, MF_BYCOMMAND | check);

                    EnableRobotMove(bRobotRandMove);
                }
                break;
            case ID_ROBOT_RANDOMBUSH:
                {
                    bRobotAutoBush = !bRobotAutoBush;
                    UINT check = bRobotAutoBush ? MF_CHECKED : MF_UNCHECKED;
                    CheckMenuItem(hmenuBar, ID_ROBOT_RANDOMBUSH, MF_BYCOMMAND | check);

                    EnableRobotBush(bRobotAutoBush);
                }
                break;
            case ID_VIEW_MASK:
                {
                    bViewMask = !bViewMask;
                    UINT check = bViewMask ? MF_CHECKED : MF_UNCHECKED;
                    CheckMenuItem(hmenuBar, ID_VIEW_MASK, MF_BYCOMMAND | check);
                }
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: 在此处添加使用 hdc 的任何绘图代码...
            OnPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_LBUTTONDOWN:
        {
            mouseDownPosL.X = GET_X_LPARAM(lParam);
            mouseDownPosL.Y = GET_Y_LPARAM(lParam);
            bLMouseDown = true;
        }
        break;
    case WM_LBUTTONUP:
        {
            mouseUpPosL.X = GET_X_LPARAM(lParam);
            mouseUpPosL.Y = GET_Y_LPARAM(lParam);
            bLMouseDown = false;

            if (land)
            {
                qtb::Area area;
                GetMouseArea(area);
                if (area.width() > 1 && area.height() > 1)
                {
                    area.left = area.left * (1 / viewZoom) - viewPos.X;
                    area.bottom = area.bottom * (1 / viewZoom) - viewPos.Y;
                    area.right = area.right * (1 / viewZoom) - viewPos.X;
                    area.top = area.top * (1 / viewZoom) - viewPos.Y;

                    unsigned int bushID = CreateBush(land, area);
                    bushIDList.push_back(bushID);
                }
            }
        }
        break;
    case WM_RBUTTONDOWN:
        {
            bRMouseDown = true;

            if (cursorBushID != -1)
            {
                if (RemoveBush(land, cursorBushID))
                    cursorBushID = -1;
            }
        }
        break;
    case WM_RBUTTONUP:
        {
            bRMouseDown = false;
        }
        break;

    case WM_MBUTTONDOWN:
        {
            mouseDownPosM.X = GET_X_LPARAM(lParam);
            mouseDownPosM.Y = GET_Y_LPARAM(lParam);
            viewPosCache = viewPos;
            bMMouseDown = true;

        }
        break;
    case WM_MBUTTONUP:
        {
            mouseUpPosM.X = GET_X_LPARAM(lParam);
            mouseUpPosM.Y = GET_Y_LPARAM(lParam);

            bMMouseDown = false;
        }
        break;
    case WM_MOUSEMOVE:
        {
            cursorPos.X = GET_X_LPARAM(lParam);
            cursorPos.Y = GET_Y_LPARAM(lParam);

            MouseBushHit();

            if (bRMouseDown && cursorBushID != -1)
            {
                if (RemoveBush(land, cursorBushID))
                    cursorBushID = -1;
            }

            if (bMMouseDown)
            {
                viewPos.X = viewPosCache.X + (1 / viewZoom) * (cursorPos.X - mouseDownPosM.X);
                viewPos.Y = viewPosCache.Y + (1 / viewZoom) * (cursorPos.Y - mouseDownPosM.Y);
            }

            cursorScalePos.X = (1 / viewZoom) * cursorPos.X;
            cursorScalePos.Y = (1 / viewZoom) * cursorPos.Y;
        }
        break;
    case WM_MOUSEWHEEL:
        {
            short zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
            WORD fwKeys = GET_KEYSTATE_WPARAM(wParam);

            if (fwKeys & MK_CONTROL)
            {
                viewportScale += (float)zDelta / WHEEL_DELTA * 0.01f;
                viewportScale = max(0.000001f, viewportScale);
                UpdateViewportRect(hWnd);
            }
            else
            {
                viewZoom += (float)zDelta / WHEEL_DELTA * 0.2f;
                viewZoom = max(0.000001f, viewZoom);

                PointF scalePos;
                scalePos.X = (1 / viewZoom) * cursorPos.X;
                scalePos.Y = (1 / viewZoom) * cursorPos.Y;

                viewPos.X += scalePos.X - cursorScalePos.X;
                viewPos.Y += scalePos.Y - cursorScalePos.Y;
                cursorScalePos = scalePos;
            }                
        }
        break;
    case WM_SIZE:
        {
            UpdateViewportRect(hWnd);
        }
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// “关于”框的消息处理程序。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

VOID UpdateViewportRect(HWND hWnd)
{
    GetClientRect(hWnd, &rcClient);
    INT clientWidth = (rcClient.right - rcClient.left);
    INT clientHeight = rcClient.bottom - rcClient.top;
    float viewWidth = clientWidth * viewportScale;
    float viewHeight = clientHeight * viewportScale;

    rcViewport.X = rcClient.left + (clientWidth - viewWidth) * 0.5f;
    rcViewport.Y = rcClient.top + (clientHeight - viewHeight) * 0.5f;
    rcViewport.Width = viewWidth;
    rcViewport.Height = viewHeight;

    CleanupMemBitmap();
    PrepareMemBitmap(hWnd);
}

VOID ResetViewport(HWND hWnd)
{
    viewportScale = 0.7f;
    UpdateViewportRect(hWnd);

    viewZoom = 3.0f;
    viewPos.X = (1 / viewZoom) * (rcViewport.X + rcViewport.Width * 0.5f) - LAND_WIDTH * 0.5f;
    viewPos.Y = (1 / viewZoom) * (rcViewport.Y + rcViewport.Height * 0.5f) - LAND_HEIGHT * 0.5f;
}

VOID RefreshMenuCheck(HWND hWnd)
{
    assert(hWnd != NULL);
    HMENU hmenuBar = GetMenu(hWnd);
    UINT check = 0;

    // view
    check = bViewMask ? MF_CHECKED : MF_UNCHECKED;
    CheckMenuItem(hmenuBar, ID_VIEW_MASK, MF_BYCOMMAND | check);

    check = (zoneViewFlag & ZONE_VIEW_QTREE) ? MF_CHECKED : MF_UNCHECKED;
    CheckMenuItem(hmenuBar, ID_VIEW_QTREE, MF_BYCOMMAND | check);

    check = (zoneViewFlag & ZONE_VIEW_STATICAREAS) ? MF_CHECKED : MF_UNCHECKED;
    CheckMenuItem(hmenuBar, ID_VIEW_STATICAREAS, MF_BYCOMMAND | check);

    check = (zoneViewFlag & ZONE_VIEW_STATICBUSH) ? MF_CHECKED : MF_UNCHECKED;
    CheckMenuItem(hmenuBar, ID_VIEW_STATICBUSH, MF_BYCOMMAND | check);

    check = (zoneViewFlag & ZONE_VIEW_DYNAMICBUSH) ? MF_CHECKED : MF_UNCHECKED;
    CheckMenuItem(hmenuBar, ID_VIEW_DYNAMICBUSH, MF_BYCOMMAND | check);

    check = bViewSelectedBushGroup ? MF_CHECKED : MF_UNCHECKED;
    CheckMenuItem(hmenuBar, ID_VIEW_SELECTEDBUSHGROUP, MF_BYCOMMAND | check);

    check = (zoneViewFlag & ZONE_VIEW_BUSHGROUP) ? MF_CHECKED : MF_UNCHECKED;
    CheckMenuItem(hmenuBar, ID_VIEW_BUSHGROUP, MF_BYCOMMAND | check);

    check = bViewRobot ? MF_CHECKED : MF_UNCHECKED;
    CheckMenuItem(hmenuBar, ID_VIEW_ROBOT, MF_BYCOMMAND | check);

    // robot
    check = bEnableRobot ? MF_CHECKED : MF_UNCHECKED;
    CheckMenuItem(hmenuBar, ID_ENABLE_ROBOT, MF_BYCOMMAND | check);

    check = bRobotRandMove ? MF_CHECKED : MF_UNCHECKED;
    CheckMenuItem(hmenuBar, ID_ROBOT_RANDMOVE, MF_BYCOMMAND | check);

    check = bRobotAutoBush ? MF_CHECKED : MF_UNCHECKED;
    CheckMenuItem(hmenuBar, ID_ROBOT_RANDOMBUSH, MF_BYCOMMAND | check);
}

VOID InitLand()
{
    srand((unsigned int)time(NULL));

    qtb::Area area(0, (float)LAND_WIDTH, 0, (float)LAND_HEIGHT);

    try
    {
        QTB_RAND_BAD_ALLOC(1);
        land = new qtb::Land(area);
        if (!land)
            return;
    }
    catch (std::bad_alloc&)
    {
        qtbLog("bad_alloc:  InitLand\n");
        return;
    }

    land->devide((float)MIN_ZONE_SIZE);

    RandomStaticBush();
    //RandomDynamicBush();
}

VOID TermLand()
{
    if (land)
    {
        delete land;
        land = NULL;
    }
}

VOID InitRobot()
{
    for (INT i = 0; i < ROBOT_COUNT; ++i)
        robotList.push_back(Robot());

    if (land)
    {
        std::list<Robot>::iterator itEnd = robotList.end();
        for (std::list<Robot>::iterator it = robotList.begin(); it != itEnd; ++it)
            it->Init(land->area());
    }

    EnableRobotMove(bRobotRandMove);
    EnableRobotBush(bRobotAutoBush);
}

VOID EnableRobotMove(bool value)
{
    std::list<Robot>::iterator itEnd = robotList.end();
    for (std::list<Robot>::iterator it = robotList.begin(); it != itEnd; ++it)
        it->EnableMove(value);
}

VOID EnableRobotBush(bool value)
{
    if (land)
    {
        std::list<Robot>::iterator itEnd = robotList.end();
        for (std::list<Robot>::iterator it = robotList.begin(); it != itEnd; ++it)
            it->EnableBush(land, value);
    }
}

VOID RandomStaticBush()
{
    if (!land)
        return;

    qtb::AreaList staticAreas;

    for (int i = 0; i < STATIC_AREA_COUNT; ++i)
    {
        float x = RangeRand(0.0f, (float)LAND_WIDTH);
        float y = RangeRand(0.0f, (float)LAND_HEIGHT);
        float w = RangeRand(AREA_SIZE_MIN, AREA_SIZE_MAX) * 0.5f;
        float h = RangeRand(AREA_SIZE_MIN, AREA_SIZE_MAX) * 0.5f;

        try
        {
            QTB_RAND_BAD_ALLOC(1);
            staticAreas.push_back(qtb::Area(x - w, x + w, y - h, y + h));
        }
        catch (std::bad_alloc&)
        {
            qtbLog("bad_alloc:  RandomStaticBush\n");
            break;
        }
    }

    perfTool.Start();
    land->rebuild(staticAreas);
    dRebuildTime = perfTool.End();

    dRebuildTimeTotal += dRebuildTime;
    ++nRebuildCount;
    dRebuildTimeAvg = dRebuildTimeTotal / nRebuildCount;
}

VOID RandomDynamicBush()
{
    if (!land)
        return;

    for (int i = 0; i < DYNAMIC_AREA_COUNT; ++i)
    {
        float x = RangeRand(0.0f, (float)LAND_WIDTH);
        float y = RangeRand(0.0f, (float)LAND_HEIGHT);
        float w = RangeRand(AREA_SIZE_MIN, AREA_SIZE_MAX) * 0.5f;
        float h = RangeRand(AREA_SIZE_MIN, AREA_SIZE_MAX) * 0.5f;

        unsigned int bushID = CreateBush(land, qtb::Area(x - w, x + w, y - h, y + h));
        bushIDList.push_back(bushID);
    }
}

VOID OnUpdate()
{
    appTime = GetTickCount64() / 1000.0f;

    RobotTick(appTime);
    MouseBushHit();

    ++frameCountTotal;
    fpsAvg = frameCountTotal / (appTime - initFrameTime);

    float elapseTime = appTime - lastFrameTime;
    if (elapseTime < 1.0f)
    {
        ++frameCount;
    }
    else
    {
        fps = frameCount / elapseTime;
        frameCount = 0;
        lastFrameTime = appTime;
    }
}

VOID GetMouseArea(qtb::Area& area)
{
    area.left = (float)(mouseDownPosL.X < cursorPos.X ? mouseDownPosL.X : cursorPos.X);
    area.bottom = (float)(mouseDownPosL.Y < cursorPos.Y ? mouseDownPosL.Y : cursorPos.Y);
    area.right = area.left + abs(mouseDownPosL.X - cursorPos.X);
    area.top = area.bottom + abs(mouseDownPosL.Y - cursorPos.Y);
}

VOID MouseBushHit()
{
    if (!land)
        return;

    cursorBushGroupID = -1;
    cursorBushID = -1;

    float x = cursorPos.X * (1 / viewZoom) - viewPos.X;
    float y = cursorPos.Y * (1 / viewZoom) - viewPos.Y;
    BushContains(land, x, y, &cursorBushGroupID, &cursorBushID);
}

VOID RobotTick(float appTime)
{
    if (!land)
        return;

    if (!bEnableRobot)
        return;

    std::list<Robot>::iterator itBegin = robotList.begin();
    std::list<Robot>::iterator itEnd = robotList.end();

    for (std::list<Robot>::iterator it = itBegin; it != itEnd; ++it)
        it->Tick(land, appTime);

    for (std::list<Robot>::iterator it = itBegin; it != itEnd; ++it)
    {
        unsigned int bushGroupID = -1;
        unsigned int bushID = -1;
        BushContains(land, it->x(), it->y(), &bushGroupID, &bushID);
        it->setBushGroupID(bushGroupID);
    }
}

VOID PrepareMemBitmap(HWND hWnd)
{
    assert(NULL == graphics);
    assert(NULL == hbmMem);
    assert(NULL == hdcMem);

    HDC hdc = GetDC(hWnd);
    if (NULL == hdc)
        return;

    hdcMem = CreateCompatibleDC(hdc);
    if (NULL == hdcMem)
        return;

    hbmMem = CreateCompatibleBitmap(hdc, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top);
    if (NULL == hbmMem)
        return;

    hbmOld = (HBITMAP)SelectObject(hdcMem, hbmMem);
    graphics = new Graphics(hdcMem);
}

VOID CleanupMemBitmap()
{
    if (graphics != NULL)
    {
        delete graphics;
        graphics = NULL;
    }

    if (hbmOld != NULL)
    {
        SelectObject(hdcMem, hbmOld);
        hbmOld = NULL;
    }

    if (hbmMem != NULL)
    {
        DeleteObject(hbmMem);
        hbmMem = NULL;
    }

    if (hdcMem != NULL)
    {
        DeleteDC(hdcMem);
        hdcMem = NULL;
    }
}


VOID OnPaint(HWND hWnd, PAINTSTRUCT* ps)
{
    if (!land)
        return;

    if (!graphics)
        return;
    
    // ready
    const RECT& rc = rcClient;

    // erase
    SolidBrush solidBrush(Color(255, 255, 255, 255));
    Rect grc((INT)rc.left, (INT)rc.top, (INT)(rc.right - rc.left), (INT)(rc.bottom - rc.top));
    graphics->FillRectangle(&solidBrush, grc);

    // draw
    DrawMain();

    // present
    BitBlt(ps->hdc, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, hdcMem, 0, 0,  SRCCOPY);

    return;
}

VOID DrawMain()
{
    assert(land);
    assert(graphics);

    if(zoneViewFlag)
        DrawZones();

    if (bViewSelectedBushGroup)
        DrawSelectedBushGroup();

    if (bViewRobot)
        DrawRobot();

    DrawMouseOperate();

    if(bViewMask)
        DrawMask();

    DrawTexts();
}

VOID DrawZones()
{
    assert(land);
    assert(graphics);

    Color staticBushColor(162, 200, 192, 0);
    Pen treePen(Color(32, 128, 128, 128));
    SolidBrush staticAreaBrush(staticBushColor);
    SolidBrush dynamicBushBrush(Color(160, 112, 216, 0));

    qtb::Area area(
        rcViewport.X * (1 / viewZoom) - viewPos.X,
        (rcViewport.X + rcViewport.Width) * (1 / viewZoom) - viewPos.X,
        rcViewport.Y * (1 / viewZoom) - viewPos.Y,
        (rcViewport.Y + rcViewport.Height) * (1 / viewZoom) - viewPos.Y);

    std::list<qtb::QTree*> viewZoneList;
    land->layer(area, viewZoneList, false);
    visibleZone = viewZoneList.size();

    std::list<qtb::QTree*>::const_iterator itZoneEnd = viewZoneList.end();
    for (std::list<qtb::QTree*>::const_iterator itZone = viewZoneList.begin(); itZone != itZoneEnd; ++itZone)
    {
        const qtb::Zone* zone = dynamic_cast<qtb::Zone*>(*itZone);

        if (zoneViewFlag & ZONE_VIEW_QTREE)
        {
            const qtb::Area& areaTree = zone->area();
            RectF rcTree(viewZoom * (areaTree.left + viewPos.X), viewZoom * (areaTree.bottom + viewPos.Y), viewZoom * areaTree.width(), viewZoom * areaTree.height());
            graphics->DrawRectangle(&treePen, rcTree);
        }

        const qtb::BushGroupPMap& boundBushGroups = zone->boundBushGroups();
        qtb::BushGroupPMap::const_iterator itGroupEnd = boundBushGroups.end();
        for (qtb::BushGroupPMap::const_iterator itGroup = boundBushGroups.begin(); itGroup != itGroupEnd; ++itGroup)
        {
            const qtb::BushGroup* bushGroup = itGroup->second;
            const qtb::Area& groupArea = bushGroup->overall();
            RectF rcGroup(viewZoom * (viewPos.X + groupArea.left), viewZoom * (viewPos.Y + groupArea.bottom), viewZoom * groupArea.width(), viewZoom * groupArea.height());
            if (!rcViewport.IntersectsWith(rcGroup))
                continue;

            const qtb::BushPMap& bushes = bushGroup->bushes();
            qtb::BushPMap::const_iterator itBushEnd = bushes.end();
            for (qtb::BushPMap::const_iterator itBush = bushes.begin(); itBush != itBushEnd; ++itBush)
            {
                const qtb::Bush* bush = itBush->second;
                const qtb::Area& area = bush->overall();
                RectF rcBush(viewZoom * (viewPos.X + area.left), viewZoom * (viewPos.Y + area.bottom), viewZoom * area.width(), viewZoom * area.height());

                if (!rcViewport.IntersectsWith(rcBush))
                    continue;

                if (bush->isStatic())
                {
                    if (zoneViewFlag & ZONE_VIEW_STATICAREAS)
                    {
                        const qtb::AreaList& areaList = bush->areas();
                        qtb::AreaList::const_iterator itAreaEnd = areaList.end();
                        for (qtb::AreaList::const_iterator itArea = areaList.begin(); itArea != itAreaEnd; ++itArea)
                        {
                            RectF rcArea(viewZoom * (viewPos.X + itArea->left), viewZoom * (viewPos.Y + itArea->bottom), viewZoom * itArea->width(), viewZoom * itArea->height());
                            if (rcViewport.IntersectsWith(rcArea))
                                graphics->FillRectangle(&staticAreaBrush, rcArea);
                        }
                    }

                    if (zoneViewFlag & ZONE_VIEW_STATICBUSH)
                    {
                        Pen pen(staticBushColor);
                        graphics->DrawRectangle(&pen, rcBush);
                    }
                }
                else
                {
                    if (zoneViewFlag & ZONE_VIEW_DYNAMICBUSH)
                    {
                        graphics->FillRectangle(&dynamicBushBrush, rcBush);
                    }
                }
            }

            if (zoneViewFlag & ZONE_VIEW_BUSHGROUP)
            {
                Pen pen(drawData.GetZoneGenerationRes(zone->generation()).color);
                graphics->DrawRectangle(&pen, rcGroup);
            }
        }
    }

}

VOID DrawRobot()
{
    assert(graphics);

    std::list<Robot>::iterator itEnd = robotList.end();
    for (std::list<Robot>::iterator it = robotList.begin(); it != itEnd; ++it)
    {
        RectF rc(viewZoom * (viewPos.X + it->x() - 1.0f),
            viewZoom * (viewPos.Y + it->y() - 1.0f),
            viewZoom * 2.0f, 
            viewZoom * 2.0f);

        if (!rcViewport.IntersectsWith(rc))
            continue;

        Color color(drawData.GetBushGroupRes(it->getBrushGroupID()).color);
        if (it->getBrushGroupID() != -1)
        {
            SolidBrush brush(color);
            graphics->FillEllipse(&brush, rc);
        }

        Pen pen(Color(192, color.GetR(), color.GetG(), color.GetB()));
        graphics->DrawEllipse(&pen, rc);
    }
}

VOID DrawSelectedBushGroup()
{
    assert(land);
    assert(graphics);

    if (-1 == cursorBushGroupID)
        return;

    const qtb::BushGroupPMap& bushGroupMap = land->bushGroups();
    qtb::BushGroupPMap::const_iterator it = bushGroupMap.find(cursorBushGroupID);
    if (bushGroupMap.end() == it)
        return;

    SolidBrush solidBrush(drawData.GetZoneGenerationRes(it->second->zone()->generation()).color);
    const qtb::Area& area = it->second->overall();
    RectF rc(viewZoom * (viewPos.X + area.left), viewZoom * (viewPos.Y + area.bottom), viewZoom * area.width(), viewZoom * area.height());
    graphics->FillRectangle(&solidBrush, rc);
}

VOID DrawMouseOperate()
{
    if (!bLMouseDown)
        return;

    assert(graphics);

    qtb::Area area;
    GetMouseArea(area);

    Pen pen(Color(160, 112, 216, 0));
    RectF rc(area.left, area.bottom, area.width(), area.height());
    graphics->DrawRectangle(&pen, rc);
}

VOID DrawMask()
{
    assert(graphics);
    Pen pen(Color::Black);
    graphics->DrawRectangle(&pen, rcViewport);
}

VOID DrawTexts()
{
    assert(land);
    assert(graphics);

    const qtb::Area& area = land->area();
    RectF rc(area.left, area.bottom, area.width(), area.height());

    // Initialize arguments.
    Font myFont(L"Arial", 10);
    //SolidBrush blackBrush(Color::Black);
    SolidBrush greyBrush(Color::Gray);

    TCHAR string[64] = _T("");

    // fps
    PointF origin(10.0f, 10.0f);
    _sntprintf_s(string, 64, _T("fps: %.2f avg:%.2f"), fps, fpsAvg);
    graphics->DrawString(string, (INT)_tcslen(string), &myFont, origin, &greyBrush);

    // land
    origin = origin + PointF(0.0f, 30.0f);
    _sntprintf_s(string, 64, _T("land size: %d x %d"), LAND_WIDTH, LAND_HEIGHT);
    graphics->DrawString(string, (INT)_tcslen(string), &myFont, origin, &greyBrush);

    origin = origin + PointF(0.0f, 20.0f);
    _sntprintf_s(string, 64, _T("min zone size: %d"), MIN_ZONE_SIZE);
    graphics->DrawString(string, (INT)_tcslen(string), &myFont, origin, &greyBrush);

    origin = origin + PointF(0.0f, 20.0f);
#ifdef _WIN64
    _sntprintf_s(string, 64, _T("visible zone: %llu"), visibleZone);
#else
    _sntprintf_s(string, 64, _T("visible zone: %u"), visibleZone);
#endif
    graphics->DrawString(string, (INT)_tcslen(string), &myFont, origin, &greyBrush);

    // static area count
    origin = origin + PointF(0.0f, 30.0f);
    _sntprintf_s(string, 64, _T("static area: %d"), land->staticBushes().empty() ? 0 : STATIC_AREA_COUNT);
    graphics->DrawString(string, (INT)_tcslen(string), &myFont, origin, &greyBrush);

    // static bush count
    origin = origin + PointF(0.0f, 20.0f);
#ifdef _WIN64
    _sntprintf_s(string, 64, _T("static bush: %llu"), land->staticBushes().size());
#else
    _sntprintf_s(string, 64, _T("static bush: %u"), land->staticBushes().size());
#endif
    graphics->DrawString(string, (INT)_tcslen(string), &myFont, origin, &greyBrush);

    // dynamic bush count
    origin = origin + PointF(0.0f, 20.0f);
#ifdef _WIN64
    _sntprintf_s(string, 64, _T("dynamic bush: %llu"), land->bushes().size());
#else
    _sntprintf_s(string, 64, _T("dynamic bush: %u"), land->bushes().size());
#endif
    graphics->DrawString(string, (INT)_tcslen(string), &myFont, origin, &greyBrush);

    // bush group count
    origin = origin + PointF(0.0f, 20.0f);
#ifdef _WIN64
    _sntprintf_s(string, 64, _T("bush group: %llu"), land->bushGroups().size());
#else
    _sntprintf_s(string, 64, _T("bush group: %u"), land->bushGroups().size());
#endif
    graphics->DrawString(string, (INT)_tcslen(string), &myFont, origin, &greyBrush);

    // robot count
    origin = origin + PointF(0.0f, 30.0f);
#ifdef _WIN64
    _sntprintf_s(string, 64, _T("robot count: %llu"), robotList.size());
#else
    _sntprintf_s(string, 64, _T("robot count: %u"), robotList.size());
#endif
    graphics->DrawString(string, (INT)_tcslen(string), &myFont, origin, &greyBrush);

    // view size
    origin = origin + PointF(0.0f, 30.0f);
    _sntprintf_s(string, 64, _T("view size: %.2f,%.2f"), rcViewport.Width / viewZoom, rcViewport.Height / viewZoom);
    graphics->DrawString(string, (INT)_tcslen(string), &myFont, origin, &greyBrush);

    // view pos
    origin = origin + PointF(0.0f, 20.0f);
    _sntprintf_s(string, 64, _T("view pos: %.2f,%.2f"), viewPos.X, viewPos.Y);
    graphics->DrawString(string, (INT)_tcslen(string), &myFont, origin, &greyBrush);

    // view scale
    origin = origin + PointF(0.0f, 20.0f);
    _sntprintf_s(string, 64, _T("view zoom: %.2f"), viewZoom);
    graphics->DrawString(string, (INT)_tcslen(string), &myFont, origin, &greyBrush);

    // group in mouse
    origin = origin + PointF(0.0f, 30.0f);
    _sntprintf_s(string, 64, _T("group in mouse: %d"), cursorBushGroupID);
    SolidBrush bushGroupBrush(drawData.GetBushGroupRes(cursorBushGroupID).color);
    graphics->DrawString(string, (INT)_tcslen(string), &myFont, origin, &bushGroupBrush);

    // bush in mouse
    origin = origin + PointF(0.0f, 20.0f);
    _sntprintf_s(string, 64, _T("bush in mouse: %d"), cursorBushID);
    SolidBrush bushBrush(drawData.GetBushRes(cursorBushID).color);
    graphics->DrawString(string, (INT)_tcslen(string), &myFont, origin, &bushBrush);

    // rebuild static bush
    origin = origin + PointF(0.0f, 30.0f);
    _sntprintf_s(string, 64, _T("rebuild: %f avg:%f"), dRebuildTime, dRebuildTimeAvg);
    graphics->DrawString(string, (INT)_tcslen(string), &myFont, origin, &greyBrush);

    // create dynamic bush
    origin = origin + PointF(0.0f, 20.0f);
    _sntprintf_s(string, 64, _T("create bush: %f avg:%f"), dCreateBushTime, dCreateBushTimeAvg);
    graphics->DrawString(string, (INT)_tcslen(string), &myFont, origin, &greyBrush);

    // remove dynamic bush
    origin = origin + PointF(0.0f, 20.0f);
    _sntprintf_s(string, 64, _T("remove bush: %f avg:%f"), dRemoveBushTime, dRemoveBushTimeAvg);
    graphics->DrawString(string, (INT)_tcslen(string), &myFont, origin, &greyBrush);

    // cursor hit
    origin = origin + PointF(0.0f, 20.0f);
    _sntprintf_s(string, 64, _T("hit: %f avg:%f"), dBushCrossTime, dBushCrossTimeAvg);
    graphics->DrawString(string, (INT)_tcslen(string), &myFont, origin, &greyBrush);

}
