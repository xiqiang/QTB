// QTB.cpp : 定义应用程序的入口点。
//

#include <time.h>
#include <objidl.h>
#include <gdiplus.h>
#include <windowsx.h>

#define _CRTDBG_MAP_ALLOC
#define _CRT_SECURE_NO_WARNINGS
#include <crtdbg.h>

#include "framework.h"
#include "QTB.h"
#include "base/Land.h"
#include "editor/DrawData.h"

#pragma comment (lib,"Gdiplus.lib")
using namespace Gdiplus;

#define MAX_LOADSTRING 100

const int   LAND_WIDTH              = 600;
const int   LAND_HEIGHT             = 600;
const int   MIN_ZONE_SIZE           = 20;
const int   STATIC_AREA_COUNT       = 200;
const int   DYNAMIC_AREA_COUNT      = 100;
const float RAND_AREA_SIZE_MIN      = 3.0f;
const float RAND_AREA_SIZE_MAX      = 30.0f;

// 全局变量:
HINSTANCE hInst;                                // 当前实例
HWND hWndMain;
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名

GdiplusStartupInput gdiplusStartupInput;
ULONG_PTR           gdiplusToken;

Point               cursorDownPos;
Point               cursorUpPos;
Point               cursorPos;
unsigned int        cursorBushGroupID = -1;

DrawData            drawData;
qtb::Land*          land = NULL;
qtb::AreaMap        staticAreaMap;
qtb::AreaMap        dynamicAreaMap;

BOOL                bViewStaticAreas = TRUE;
BOOL                bViewDynamicAreas = TRUE;
BOOL                bViewStaticBush = TRUE;
BOOL                bViewDynamicBush = TRUE;
BOOL                bViewBushGroup = TRUE;

// 此代码模块中包含的函数的前向声明:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
VOID                TermInstance(HINSTANCE);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

VOID                InitLand();
VOID                TermLand();
VOID                RandomStaticBush();
VOID                RandomDynamicBush();

VOID                OnPaint(HWND hWnd, PAINTSTRUCT* ps);
VOID                DrawMain(Graphics& graphics);
VOID                DrawQTree(Graphics& graphics, qtb::QTree* tree);
VOID                DrawStaticAreas(Graphics& graphics);
VOID                DrawDynamicAreas(Graphics& graphics);
VOID                DrawStaticBush(Graphics& graphics);
VOID                DrawDynamicBush(Graphics& graphics);
VOID                DrawBushGroup(Graphics& graphics);

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

   InitLand();

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
            case IDM_LAND_RANDOMSTATICBUSH:
                RandomStaticBush();
                break;
            case ID_LAND_RANDOMDYNAMICAREAS:
                RandomDynamicBush();
                break;
            case ID_VIEW_STATICAREAS:
                {
                    bViewStaticAreas = !bViewStaticAreas;
                    UINT check = bViewStaticAreas ? MF_CHECKED : MF_UNCHECKED;
                    CheckMenuItem(hmenuBar, ID_VIEW_STATICAREAS, MF_BYCOMMAND | check);
                }
                break;
            case ID_VIEW_DYNAMICAREAS:
                {
                    bViewDynamicAreas = !bViewDynamicAreas;
                    UINT check = bViewDynamicAreas ? MF_CHECKED : MF_UNCHECKED;
                    CheckMenuItem(hmenuBar, ID_VIEW_DYNAMICAREAS, MF_BYCOMMAND | check);
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
            case ID_VIEW_BUSHGROUP:
                {
                    bViewBushGroup = !bViewBushGroup;
                    UINT check = bViewBushGroup ? MF_CHECKED : MF_UNCHECKED;
                    CheckMenuItem(hmenuBar, ID_VIEW_BUSHGROUP, MF_BYCOMMAND | check);
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
            cursorDownPos.X = GET_X_LPARAM(lParam);
            cursorDownPos.Y = GET_Y_LPARAM(lParam);
        }
        break;
    case WM_LBUTTONUP:
        {
            cursorUpPos.X = GET_X_LPARAM(lParam);
            cursorUpPos.Y = GET_Y_LPARAM(lParam);
        }
        break;
    case WM_MOUSEMOVE:
        {
            cursorPos.X = GET_X_LPARAM(lParam);
            cursorPos.Y = GET_Y_LPARAM(lParam);

            if (land)
                cursorBushGroupID = land->crossBushGroupID((float)cursorPos.X, (float)cursorPos.Y);
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

int RangeRand(int min, int max)
{
    return rand() % (max - min) + min;
}

float RangeRand(float min, float max)
{
    return rand() / (float)RAND_MAX * (max - min) + min;
}

VOID InitLand()
{
    srand((unsigned int)time(NULL));

    qtb::Area area(0, LAND_WIDTH, 0, LAND_HEIGHT);
    land = new qtb::Land(area);
    land->devide(MIN_ZONE_SIZE);

    RandomStaticBush();
    RandomDynamicBush();
}

VOID TermLand()
{
    if (land)
    {
        delete land;
        land = NULL;
    }
}

VOID RandomStaticBush()
{
    if (!land)
        return;

    staticAreaMap.clear();

    for (int i = 0; i < STATIC_AREA_COUNT; ++i)
    {
        float x = RangeRand(0.0f, (float)LAND_WIDTH);
        float y = RangeRand(0.0f, (float)LAND_HEIGHT);
        float w = RangeRand(RAND_AREA_SIZE_MIN, RAND_AREA_SIZE_MAX) * 0.5f;
        float h = RangeRand(RAND_AREA_SIZE_MIN, RAND_AREA_SIZE_MAX) * 0.5f;

        staticAreaMap[land->AllocAreaID()]=(qtb::Area(x - w, x + w, y - h, y + h));
    }

    land->resetStaticBush(staticAreaMap);
    land->updateBushGroup();
}

VOID RandomDynamicBush()
{
    if (!land)
        return;

    dynamicAreaMap.clear();

    for (int i = 0; i < DYNAMIC_AREA_COUNT; ++i)
    {
        float x = RangeRand(0.0f, (float)LAND_WIDTH);
        float y = RangeRand(0.0f, (float)LAND_HEIGHT);
        float w = RangeRand(RAND_AREA_SIZE_MIN, RAND_AREA_SIZE_MAX) * 0.5f;
        float h = RangeRand(RAND_AREA_SIZE_MIN, RAND_AREA_SIZE_MAX) * 0.5f;

        dynamicAreaMap[land->AllocAreaID()] = (qtb::Area(x - w, x + w, y - h, y + h));
    }

    land->setDynamicAreas(dynamicAreaMap);
    land->updateBushGroup();
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
    SolidBrush solidBrush(Color(255, 0, 0, 0));
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

    DrawQTree(graphics, land);

    if(bViewStaticAreas)
        DrawStaticAreas(graphics);
    if (bViewDynamicAreas)
        DrawDynamicAreas(graphics);
    if (bViewStaticBush)
        DrawStaticBush(graphics);
    if (bViewDynamicBush)
        DrawDynamicBush(graphics);
    if (bViewBushGroup)
        DrawBushGroup(graphics);

    // mouse
    /*
    Pen penMD(Color(255, 0, 255, 0));
    Rect rcMD(cursorDownPos.X - 3, cursorDownPos.Y - 3, 6, 6);
    graphics.DrawEllipse(&penMD, rcMD);
    Rect rcMM(cursorPos.X - 3, cursorPos.Y - 3, 6, 6);
    graphics.DrawEllipse(&penMD, rcMM);

    Pen penMU(Color(255, 0, 255, 255));
    Rect rcMU(cursorUpPos.X - 3, cursorUpPos.Y - 3, 6, 6);
    graphics.DrawEllipse(&penMU, rcMU);
    */
}

VOID DrawQTree(Graphics& graphics, qtb::QTree* tree)
{
    if (!land)
        return;

    assert(tree);

    const qtb::Area& area = tree->area();
    RectF rc(area.left, area.bottom, area.width(), area.height());
    Pen pen(Color(32, 0, 128, 128));
    graphics.DrawRectangle(&pen, rc);

    for (int i = 0; i < qtb::QTree::CHILD_COUNT; ++i)
    {
        qtb::QTree* child = tree->getChild(i);
        if (child)
            DrawQTree(graphics, child);
    }
}

VOID DrawStaticAreas(Graphics& graphics)
{
    SolidBrush brush(Color(64, 128, 192, 0));
    for (qtb::AreaMap::const_iterator it = staticAreaMap.begin(); it != staticAreaMap.end(); ++it)
    {
        RectF rc(it->second.left, it->second.bottom, it->second.width(), it->second.height());
        graphics.FillRectangle(&brush, rc);
    }
}

VOID DrawDynamicAreas(Graphics& graphics)
{
    SolidBrush brush(Color(64, 0, 192, 128));
    for (qtb::AreaMap::const_iterator it = dynamicAreaMap.begin(); it != dynamicAreaMap.end(); ++it)
    {
        RectF rc(it->second.left, it->second.bottom, it->second.width(), it->second.height());
        graphics.FillRectangle(&brush, rc);
    }
}

VOID DrawStaticBush(Graphics& graphics)
{
    if (!land)
        return;

    const qtb::BushPMap& staticBush = land->getStaticBush();
    for (qtb::BushPMap::const_iterator it = staticBush.begin(); it != staticBush.end(); ++it)
    {
        const qtb::Bush* bush = it->second;
        const qtb::Area& area = bush->overall();
        RectF rc(area.left, area.bottom, area.width(), area.height());

        Pen pen(drawData.GetBushRes(bush->id()).color);
        graphics.DrawRectangle(&pen, rc);
    }
}

VOID DrawDynamicBush(Graphics& graphics)
{
    if (!land)
        return;

    const qtb::BushPMap& dynamicBush = land->getDynamicBush();
    for (qtb::BushPMap::const_iterator it = dynamicBush.begin(); it != dynamicBush.end(); ++it)
    {
        const qtb::Bush* bush = it->second;
        const qtb::Area& area = bush->overall();
        RectF rc(area.left, area.bottom, area.width(), area.height());

        Pen pen(drawData.GetBushRes(bush->id()).color);
        graphics.DrawRectangle(&pen, rc);
    }
}

VOID DrawBushGroup(Graphics& graphics)
{
    if (!land)
        return;

    const qtb::BushGroupPMap& bushGroupMap = land->getBushGroup();
    for (qtb::BushGroupPMap::const_iterator it = bushGroupMap.begin(); it != bushGroupMap.end(); ++it)
    {
        const qtb::BushGroup* bushGroup = it->second;
        const qtb::Area& area = bushGroup->overall();
        RectF rc(area.left, area.bottom, area.width(), area.height());

        if (bushGroup->id() == cursorBushGroupID)
        {
            SolidBrush solidBrush(drawData.GetBushGroupRes(bushGroup->id()).color);
            graphics.FillRectangle(&solidBrush, rc);
        }

        Pen pen(drawData.GetZoneGenerationRes(bushGroup->zone()->generation()).color);
        graphics.DrawRectangle(&pen, rc);
    }
}
