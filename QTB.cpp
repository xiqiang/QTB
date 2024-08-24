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
#include "editor/Util.h"
#include "editor/DrawData.h"
#include "editor/PerfTool.h"
#include "editor/Robot.h"

#pragma comment (lib,"Gdiplus.lib")
using namespace Gdiplus;

#define MAX_LOADSTRING 100

#ifdef _DEBUG
const float LAND_WIDTH = 1000.0f;
const float LAND_HEIGHT = 1000.0f;
const float MIN_ZONE_SIZE = 30.0f;
const int   STATIC_AREA_COUNT = 5000;
const int   DYNAMIC_AREA_COUNT = 2500;
const int   ROBOT_COUNT = 1000;
#else
const float LAND_WIDTH              = 2000.0f;
const float LAND_HEIGHT             = 2000.0f;
const float MIN_ZONE_SIZE           = 30.0f;
const int   STATIC_AREA_COUNT       = 20000;
const int   DYNAMIC_AREA_COUNT      = 10000;
const int   ROBOT_COUNT             = 5000;
#endif

// 全局变量:
HINSTANCE hInst;                                // 当前实例
HWND hWndMain;
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名

GdiplusStartupInput gdiplusStartupInput;
ULONG_PTR           gdiplusToken;

std::list<Robot>        robotList;
std::list<unsigned int> bushIDList;

BOOL                bLMouseDown = FALSE;
BOOL                bRMouseDown = FALSE;
BOOL                bMMouseDown = FALSE;
Point               mouseDownPosL;
Point               mouseUpPosL;
Point               mouseDownPosM;
Point               mouseUpPosM;
Point               cursorPos;
PointF              cursorScalePos;

unsigned int        cursorBushGroupID = -1;
unsigned int        cursorBushID = -1;

DrawData            drawData;
qtb::Land*          land = NULL;
qtb::AreaList       staticAreas;

float               viewScale = 1.0f;
PointF              viewPosCache;
PointF              viewPos;

RectF               rcViewport;

BOOL                bViewQTree = TRUE;
BOOL                bViewStaticAreas = TRUE;
BOOL                bViewStaticBush = FALSE;
BOOL                bViewDynamicBush = TRUE;
BOOL                bViewSelectedBushGroup = TRUE;
BOOL                bViewBushGroup = FALSE;

BOOL                bViewRobot = TRUE;
BOOL                bRobotAutoBush = TRUE;

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

VOID                InitLand();
VOID                TermLand();
VOID                InitRobot();
VOID                EnableRobotBush(BOOL value);
VOID                RandomStaticBush();
VOID                RandomDynamicBush();

VOID                OnUpdate();
VOID                GetMouseArea(qtb::Area& area);
VOID                MouseBushHit();
VOID                RobotTick();

VOID                OnPaint(HWND hWnd, PAINTSTRUCT* ps);
VOID                DrawMain(Graphics& graphics);
VOID                DrawQTree(Graphics& graphics, qtb::QTree* tree);
VOID                DrawStaticAreas(Graphics& graphics);
VOID                DrawStaticBush(Graphics& graphics);
VOID                DrawDynamicBush(Graphics& graphics);
VOID                DrawRobot(Graphics& graphics);
VOID                DrawSelectedBushGroup(Graphics& graphics);
VOID                DrawBushGroup(Graphics& graphics);
VOID                DrawMouseOperate(Graphics& graphics);
VOID                DrawTexts(Graphics& graphics);

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

    TermInstance(hInstance);
    GdiplusShutdown(gdiplusToken);
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

   ResetViewport(hWndMain);
   InitLand();
   InitRobot();

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
                staticAreas.clear();
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
                    bViewQTree = !bViewQTree;
                    UINT check = bViewQTree ? MF_CHECKED : MF_UNCHECKED;
                    CheckMenuItem(hmenuBar, ID_VIEW_QTREE, MF_BYCOMMAND | check);
                }
                break;
            case ID_VIEW_STATICAREAS:
                {
                    bViewStaticAreas = !bViewStaticAreas;
                    UINT check = bViewStaticAreas ? MF_CHECKED : MF_UNCHECKED;
                    CheckMenuItem(hmenuBar, ID_VIEW_STATICAREAS, MF_BYCOMMAND | check);
                }
                break;
            case ID_VIEW_STATICBUSH:
                {
                    bViewStaticBush = !bViewStaticBush;
                    UINT check = bViewStaticBush ? MF_CHECKED : MF_UNCHECKED;
                    CheckMenuItem(hmenuBar, ID_VIEW_STATICBUSH, MF_BYCOMMAND | check);
                }
                break;
            case ID_VIEW_DYNAMICBUSH:
                {
                    bViewDynamicBush = !bViewDynamicBush;
                    UINT check = bViewDynamicBush ? MF_CHECKED : MF_UNCHECKED;
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
                    bViewBushGroup = !bViewBushGroup;
                    UINT check = bViewBushGroup ? MF_CHECKED : MF_UNCHECKED;
                    CheckMenuItem(hmenuBar, ID_VIEW_BUSHGROUP, MF_BYCOMMAND | check);
                }
                break;
            case ID_VIEW_ROBOT:
                {
                    bViewRobot = !bViewRobot;
                    UINT check = bViewRobot ? MF_CHECKED : MF_UNCHECKED;
                    CheckMenuItem(hmenuBar, ID_VIEW_ROBOT, MF_BYCOMMAND | check);
                }
                break;
            case ID_ROBOT_AUTOBUSH:
                {
                    bRobotAutoBush = !bRobotAutoBush;
                    UINT check = bRobotAutoBush ? MF_CHECKED : MF_UNCHECKED;
                    CheckMenuItem(hmenuBar, ID_ROBOT_AUTOBUSH, MF_BYCOMMAND | check);

                    EnableRobotBush(bRobotAutoBush);
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
            bLMouseDown = TRUE;
        }
        break;
    case WM_LBUTTONUP:
        {
            mouseUpPosL.X = GET_X_LPARAM(lParam);
            mouseUpPosL.Y = GET_Y_LPARAM(lParam);
            bLMouseDown = FALSE;

            if (land)
            {
                qtb::Area area;
                GetMouseArea(area);
                if (area.width() > 1 && area.height() > 1)
                {
                    area.left = area.left * (1 / viewScale) - viewPos.X;
                    area.bottom = area.bottom * (1 / viewScale) - viewPos.Y;
                    area.right = area.right * (1 / viewScale) - viewPos.X;
                    area.top = area.top * (1 / viewScale) - viewPos.Y;

                    unsigned int bushID = CreateBush(land, area);
                    bushIDList.push_back(bushID);
                }
            }
        }
        break;
    case WM_RBUTTONDOWN:
        {
            bRMouseDown = TRUE;

            if (cursorBushID != -1)
            {
                if (RemoveBush(land, cursorBushID))
                    cursorBushID = -1;
            }
        }
        break;
    case WM_RBUTTONUP:
        {
            bRMouseDown = FALSE;
        }
        break;

    case WM_MBUTTONDOWN:
        {
            mouseDownPosM.X = GET_X_LPARAM(lParam);
            mouseDownPosM.Y = GET_Y_LPARAM(lParam);
            viewPosCache = viewPos;
            bMMouseDown = TRUE;

        }
        break;
    case WM_MBUTTONUP:
        {
            mouseUpPosM.X = GET_X_LPARAM(lParam);
            mouseUpPosM.Y = GET_Y_LPARAM(lParam);

            bMMouseDown = FALSE;
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
                viewPos.X = viewPosCache.X + (1 / viewScale) * (cursorPos.X - mouseDownPosM.X);
                viewPos.Y = viewPosCache.Y + (1 / viewScale) * (cursorPos.Y - mouseDownPosM.Y);
            }

            cursorScalePos.X = (1 / viewScale) * cursorPos.X;
            cursorScalePos.Y = (1 / viewScale) * cursorPos.Y;
        }
        break;
    case WM_MOUSEWHEEL:
        {
            INT zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
            viewScale += (float)zDelta / WHEEL_DELTA * 0.2f;

            PointF scalePos;
            scalePos.X = (1 / viewScale) * cursorPos.X;
            scalePos.Y = (1 / viewScale) * cursorPos.Y;

            viewPos.X += scalePos.X - cursorScalePos.X;
            viewPos.Y += scalePos.Y - cursorScalePos.Y;
            cursorScalePos = scalePos;
        }
        break;
    case WM_SIZE:
        {
            RECT rcClient;
            GetClientRect(hWnd, &rcClient);
            rcViewport.X = (float)rcClient.left;
            rcViewport.Y = (float)rcClient.top;
            rcViewport.Width = (float)(rcClient.right - rcClient.left);
            rcViewport.Height = (float)(rcClient.bottom - rcClient.top);
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

VOID ResetViewport(HWND hWnd)
{
    viewScale = 3.0f;

    RECT rcClient;
    GetClientRect(hWnd, &rcClient);
    rcViewport.X = (float)rcClient.left;
    rcViewport.Y = (float)rcClient.top;
    rcViewport.Width = (float)(rcClient.right - rcClient.left);
    rcViewport.Height = (float)(rcClient.bottom - rcClient.top);

    viewPos.X = (1 / viewScale) * (rcClient.right - rcClient.left) * 0.5f - LAND_WIDTH * 0.5f;
    viewPos.Y = (1 / viewScale) * (rcClient.bottom - rcClient.top) * 0.5f - LAND_HEIGHT * 0.5f;
}

VOID InitLand()
{
    srand((unsigned int)time(NULL));

    qtb::Area area(0, LAND_WIDTH, 0, LAND_HEIGHT);
    land = new qtb::Land(area);
    land->devide(MIN_ZONE_SIZE);

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
        for (std::list<Robot>::iterator it = robotList.begin(); it != robotList.end(); ++it)
            it->Init(land->area());
    }

    EnableRobotBush(bRobotAutoBush);
}

VOID EnableRobotBush(BOOL value)
{
    if (land)
    {
        for (std::list<Robot>::iterator it = robotList.begin(); it != robotList.end(); ++it)
            it->EnableBush(land, value);
    }
}

VOID RandomStaticBush()
{
    if (!land)
        return;

    staticAreas.clear();

    for (int i = 0; i < STATIC_AREA_COUNT; ++i)
    {
        float x = RangeRand(0.0f, (float)LAND_WIDTH);
        float y = RangeRand(0.0f, (float)LAND_HEIGHT);
        float w = RangeRand(AREA_SIZE_MIN, AREA_SIZE_MAX) * 0.5f;
        float h = RangeRand(AREA_SIZE_MIN, AREA_SIZE_MAX) * 0.5f;

        staticAreas.push_back(qtb::Area(x - w, x + w, y - h, y + h));
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
    MouseBushHit();
    RobotTick();
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

    float x = cursorPos.X * (1 / viewScale) - viewPos.X;
    float y = cursorPos.Y * (1 / viewScale) - viewPos.Y;
    BushContains(land, x, y, &cursorBushGroupID, &cursorBushID);
}

VOID RobotTick()
{
    if (!land)
        return;

    if (!bViewRobot)
        return;

    for (std::list<Robot>::iterator it = robotList.begin(); it != robotList.end(); ++it)
    {
        it->Tick(land, GetTickCount64() / 1000.0f);

        unsigned int bushGroupID = -1;
        unsigned int bushID = -1;
        BushContains(land, it->x(), it->y(), &bushGroupID, &bushID);
        it->setBushGroupID(bushGroupID);
    }
}

VOID OnPaint(HWND hWnd, PAINTSTRUCT* ps)
{
    if (!land)
        return;
    
    RECT rc;
    GetClientRect(hWnd, &rc);

    // ready
    HDC hdcMem = CreateCompatibleDC(ps->hdc);
    HBITMAP hbmMem = CreateCompatibleBitmap(ps->hdc, rc.right - rc.left, rc.bottom - rc.top);
    HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hbmMem);
    Graphics graphics(hdcMem);

    // erase
    SolidBrush solidBrush(Color(255, 255, 255, 255));
    Rect grc((INT)rc.left, (INT)rc.top, (INT)(rc.right - rc.left), (INT)(rc.bottom - rc.top));
    graphics.FillRectangle(&solidBrush, grc);

    // draw
    DrawMain(graphics);

    // present
    BitBlt(ps->hdc, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, hdcMem, 0, 0,  SRCCOPY);

    // cleanup
    SelectObject(hdcMem, hbmOld);
    DeleteObject(hbmMem);
    DeleteDC(hdcMem);

    return;
}

VOID DrawMain(Graphics& graphics)
{
    if (!land)
        return;

    if(bViewQTree)
        DrawQTree(graphics, land);
    if(bViewStaticAreas)
        DrawStaticAreas(graphics);
    if (bViewStaticBush)
        DrawStaticBush(graphics);
    if (bViewDynamicBush)
        DrawDynamicBush(graphics);
    if (bViewRobot)
        DrawRobot(graphics);
    if (bViewSelectedBushGroup)
        DrawSelectedBushGroup(graphics);
    if (bViewBushGroup)
        DrawBushGroup(graphics);

    DrawMouseOperate(graphics);
    DrawTexts(graphics);
}

VOID DrawQTree(Graphics& graphics, qtb::QTree* tree)
{
    if (!land)
        return;

    assert(tree);

    const qtb::Area& area = tree->area();
    RectF rc(viewScale * (area.left + viewPos.X), viewScale * (area.bottom + viewPos.Y), viewScale * area.width(), viewScale * area.height());
    if (!rcViewport.IntersectsWith(rc))
        return;

    Pen pen(Color(32, 0, 128, 128));
    graphics.DrawRectangle(&pen, rc);

    for (int i = 0; i < qtb::QTree::CHILD_COUNT; ++i)
    {
        qtb::QTree* c = tree->child(i);
        if (c)
            DrawQTree(graphics, c);
    }
}

VOID DrawStaticAreas(Graphics& graphics)
{
    SolidBrush brush(Color(128, 128, 192, 0));
    for (qtb::AreaList::const_iterator it = staticAreas.begin(); it != staticAreas.end(); ++it)
    {
        RectF rc(viewScale * (viewPos.X + it->left), viewScale * (viewPos.Y + it->bottom), viewScale * it->width(), viewScale * it->height());

        if (!rcViewport.IntersectsWith(rc))
            continue;

        graphics.FillRectangle(&brush, rc);
    }
}

VOID DrawStaticBush(Graphics& graphics)
{
    if (!land)
        return;

    const qtb::BushPMap& staticBush = land->staticBushes();
    for (qtb::BushPMap::const_iterator it = staticBush.begin(); it != staticBush.end(); ++it)
    {
        const qtb::Bush* bush = it->second;
        const qtb::Area& area = bush->overall();
        RectF rc(viewScale * (viewPos.X + area.left), viewScale * (viewPos.Y + area.bottom), viewScale * area.width(), viewScale * area.height());

        if (!rcViewport.IntersectsWith(rc))
            continue;

        Pen pen(drawData.GetBushRes(bush->id()).color);
        graphics.DrawRectangle(&pen, rc);
    }
}

VOID DrawDynamicBush(Graphics& graphics)
{
    if (!land)
        return;

    SolidBrush brush(Color(128, 0, 192, 128));

    const qtb::BushPMap& dynamicBush = land->bushes();
    for (qtb::BushPMap::const_iterator it = dynamicBush.begin(); it != dynamicBush.end(); ++it)
    {
        const qtb::Bush* bush = it->second;
        const qtb::Area& area = bush->overall();
        RectF rc(viewScale * (viewPos.X + area.left), viewScale * (viewPos.Y + area.bottom), viewScale * area.width(), viewScale * area.height());

        if (!rcViewport.IntersectsWith(rc))
            continue;

        graphics.FillRectangle(&brush, rc);

        //Pen pen(drawData.GetBushRes(bush->id()).color);
        //graphics.DrawRectangle(&pen, rc);
    }
}

VOID DrawRobot(Graphics& graphics)
{
    for (std::list<Robot>::iterator it = robotList.begin(); it != robotList.end(); ++it)
    {
        RectF rc(viewScale * (viewPos.X + it->x() - 1.0f),
            viewScale * (viewPos.Y + it->y() - 1.0f),
            viewScale * 2.0f, 
            viewScale * 2.0f);

        if (!rcViewport.IntersectsWith(rc))
            continue;

        Pen pen(drawData.GetBushGroupRes(it->getBrushGroupID()).color);
        if (it->getBrushGroupID() != -1)
        {
            SolidBrush brush(drawData.GetBushGroupRes(it->getBrushGroupID()).color);
            graphics.FillEllipse(&brush, rc);
        }
        graphics.DrawEllipse(&pen, rc);
    }
}

VOID DrawSelectedBushGroup(Graphics& graphics)
{
    if (!land)
        return;

    if (-1 == cursorBushGroupID)
        return;

    const qtb::BushGroupPMap& bushGroupMap = land->bushGroups();
    qtb::BushGroupPMap::const_iterator it = bushGroupMap.find(cursorBushGroupID);
    if (bushGroupMap.end() == it)
        return;

    //const Color& color = drawData.GetBushGroupRes(it->second->id()).color;
    const Color& color = drawData.GetZoneGenerationRes(it->second->zone()->generation()).color;
    SolidBrush solidBrush(Color(128, color.GetR(), color.GetG(), color.GetB()));
    const qtb::Area& area = it->second->overall();
    RectF rc(viewScale * (viewPos.X + area.left), viewScale * (viewPos.Y + area.bottom), viewScale * area.width(), viewScale * area.height());
    graphics.FillRectangle(&solidBrush, rc);
}

VOID DrawBushGroup(Graphics& graphics)
{
    if (!land)
        return;

    const qtb::BushGroupPMap& bushGroupMap = land->bushGroups();
    for (qtb::BushGroupPMap::const_iterator it = bushGroupMap.begin(); it != bushGroupMap.end(); ++it)
    {
        const qtb::BushGroup* bushGroup = it->second;
        const qtb::Area& area = bushGroup->overall();
        RectF rc(viewScale * (viewPos.X + area.left), viewScale * (viewPos.Y + area.bottom), viewScale * area.width(), viewScale * area.height());

        if (!rcViewport.IntersectsWith(rc))
            continue;

        //Pen pen(drawData.GetZoneGenerationRes(bushGroup->zone()->generation()).color);
        Pen pen(drawData.GetBushGroupRes(it->second->id()).color);
        graphics.DrawRectangle(&pen, rc);
    }
}

VOID DrawMouseOperate(Graphics& graphics)
{
    if (bLMouseDown)
    {
        qtb::Area area;
        GetMouseArea(area);

        Pen pen(Color(192, 128, 128, 128));
        RectF rc(area.left, area.bottom, area.width(), area.height());
        graphics.DrawRectangle(&pen, rc);
    }
}

VOID DrawTexts(Graphics& graphics)
{
    if (!land)
        return;

    const qtb::Area& area = land->area();
    RectF rc(area.left, area.bottom, area.width(), area.height());

    // Initialize arguments.
    Font myFont(L"Arial", 10);
    SolidBrush blackBrush(Color(255, 0, 0, 0));
    SolidBrush greyBrush(Color(255, 64, 64, 64));

    TCHAR string[64] = _T("");

    // land
    PointF origin(10.0f, 10.0f);
    _sntprintf_s(string, 64, _T("land size: %d x %d"), (int)LAND_WIDTH, (int)LAND_HEIGHT);
    graphics.DrawString(string, (INT)_tcslen(string), &myFont, origin, &blackBrush);

    origin = origin + PointF(0.0f, 20.0f);
    _sntprintf_s(string, 64, _T("min zone: %d"), (int)MIN_ZONE_SIZE);
    graphics.DrawString(string, (INT)_tcslen(string), &myFont, origin, &blackBrush);

    // static area count
    origin = origin + PointF(0.0f, 30.0f);
    _sntprintf_s(string, 64, _T("static area: %d"), land->staticBushes().empty() ? 0 : STATIC_AREA_COUNT);
    graphics.DrawString(string, (INT)_tcslen(string), &myFont, origin, &blackBrush);

    // static bush count
    origin = origin + PointF(0.0f, 20.0f);
#ifdef _WIN64
    _sntprintf_s(string, 64, _T("static bush: %llu"), land->staticBushes().size());
#else
    _sntprintf_s(string, 64, _T("static bush: %u"), land->staticBushes().size());
#endif
    graphics.DrawString(string, (INT)_tcslen(string), &myFont, origin, &blackBrush);

    // dynamic bush count
    origin = origin + PointF(0.0f, 20.0f);
#ifdef _WIN64
    _sntprintf_s(string, 64, _T("dynamic bush: %llu"), land->bushes().size());
#else
    _sntprintf_s(string, 64, _T("dynamic bush: %u"), land->bushes().size());
#endif
    graphics.DrawString(string, (INT)_tcslen(string), &myFont, origin, &blackBrush);

    // bush group count
    origin = origin + PointF(0.0f, 20.0f);
#ifdef _WIN64
    _sntprintf_s(string, 64, _T("bush group: %llu"), land->bushGroups().size());
#else
    _sntprintf_s(string, 64, _T("bush group: %u"), land->bushGroups().size());
#endif
    graphics.DrawString(string, (INT)_tcslen(string), &myFont, origin, &blackBrush);

    // generate static bush
    origin = origin + PointF(0.0f, 30.0f);
    _sntprintf_s(string, 64, _T("rebuild time: %f"), dRebuildTime);
    graphics.DrawString( string, (INT)_tcslen(string), &myFont, origin, &blackBrush);

    origin = origin + PointF(0.0f, 20.0f);
    _sntprintf_s(string, 64, _T("rebuild time avg: %f"), dRebuildTimeAvg);
    graphics.DrawString(string, (INT)_tcslen(string), &myFont, origin, &greyBrush);

    // create dynamic bush
    origin = origin + PointF(0.0f, 30.0f);
    _sntprintf_s(string, 64, _T("create bush time: %f"), dCreateBushTime);
    graphics.DrawString(string, (INT)_tcslen(string), &myFont, origin, &greyBrush);

    origin = origin + PointF(0.0f, 20.0f);
    _sntprintf_s(string, 64, _T("create bush time avg: %f"), dCreateBushTimeAvg);
    graphics.DrawString(string, (INT)_tcslen(string), &myFont, origin, &blackBrush);

    // remove dynamic bush
    origin = origin + PointF(0.0f, 30.0f);
    _sntprintf_s(string, 64, _T("remove bush time: %f"), dRemoveBushTime);
    graphics.DrawString(string, (INT)_tcslen(string), &myFont, origin, &greyBrush);

    origin = origin + PointF(0.0f, 20.0f);
    _sntprintf_s(string, 64, _T("remove bush time avg: %f"), dRemoveBushTimeAvg);
    graphics.DrawString(string, (INT)_tcslen(string), &myFont, origin, &blackBrush);

    // cursor hit
    origin = origin + PointF(0.0f, 30.0f);
    _sntprintf_s(string, 64, _T("hit time: %f"), dBushCrossTime);
    graphics.DrawString(string, (INT)_tcslen(string), &myFont, origin, &greyBrush);

    origin = origin + PointF(0.0f, 20.0f);
    _sntprintf_s(string, 64, _T("hit time avg: %f"), dBushCrossTimeAvg);
    graphics.DrawString(string, (INT)_tcslen(string), &myFont, origin, &blackBrush);

    // group in mouse
    origin = origin + PointF(0.0f, 30.0f);
    _sntprintf_s(string, 64, _T("group in mouse: %d"), cursorBushGroupID);
    SolidBrush bushGroupBrush(drawData.GetBushGroupRes(cursorBushGroupID).color);
    graphics.DrawString(string, (INT)_tcslen(string), &myFont, origin, &bushGroupBrush);

    // bush in mouse
    origin = origin + PointF(0.0f, 20.0f);
    _sntprintf_s(string, 64, _T("bush in mouse: %d"), cursorBushID);
    SolidBrush bushBrush(drawData.GetBushRes(cursorBushID).color);
    graphics.DrawString(string, (INT)_tcslen(string), &myFont, origin, &bushBrush);

    // view scale
    origin = origin + PointF(0.0f, 30.0f);
    _sntprintf_s(string, 64, _T("view scale: %.2f"), viewScale);
    graphics.DrawString(string, (INT)_tcslen(string), &myFont, origin, &blackBrush);

    // view pos
    origin = origin + PointF(0.0f, 20.0f);
    _sntprintf_s(string, 64, _T("view pos: %.2f,%.2f"), viewPos.X, viewPos.Y);
    graphics.DrawString(string, (INT)_tcslen(string), &myFont, origin, &blackBrush);

    // robot count
    origin = origin + PointF(0.0f, 30.0f);
#ifdef _WIN64
    _sntprintf_s(string, 64, _T("robot count: %llu"), bViewRobot ? robotList.size() : 0);
#else
    _sntprintf_s(string, 64, _T("robot count: %u"), bViewRobot ? robotList.size() : 0);
#endif
    graphics.DrawString(string, (INT)_tcslen(string), &myFont, origin, &blackBrush);

}
