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

const float LAND_WIDTH              = 700.0f;
const float LAND_HEIGHT             = 700.0f;
const float MIN_ZONE_SIZE           = 20.0f;
const int   STATIC_AREA_COUNT       = 1000;
const int   DYNAMIC_AREA_COUNT      = 500;
const float RAND_AREA_SIZE_MIN      = 3.0f;
const float RAND_AREA_SIZE_MAX      = 15.0f;
const int   ROBOT_COUNT             = 500;

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
Point               cursorDownPos;
Point               cursorUpPos;
Point               cursorPos;
unsigned int        cursorBushGroupID = -1;
unsigned int        cursorBushID = -1;

DrawData            drawData;
qtb::Land*          land = NULL;
qtb::AreaList       staticAreas;

BOOL                bViewQTree = FALSE;
BOOL                bViewStaticAreas = TRUE;
BOOL                bViewStaticBush = FALSE;
BOOL                bViewDynamicBush = TRUE;
BOOL                bViewRobot = FALSE;
BOOL                bViewSelectedBushGroup = TRUE;
BOOL                bViewBushGroup = FALSE;

PerfTool            perfTool;

int                 nRebuildCount = 0;
double              dRebuildTime = 0;
double              dRebuildTimeTotal = 0;
double              dRebuildTimeAvg = 0;

int                 nCreateBushCount = 0;
double              dCreateBushTime = 0;
double              dCreateBushTimeTotal = 0;
double              dCreateBushTimeAvg = 0;

int                 nRemoveBushCount = 0;
double              dRemoveBushTime = 0;
double              dRemoveBushTimeTotal = 0;
double              dRemoveBushTimeAvg = 0;

int                 nBushCrossCount = 0;
double              dBushCrossTime = 0;
double              dBushCrossTimeTotal = 0;
double              dBushCrossTimeAvg = 0;

// 此代码模块中包含的函数的前向声明:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
VOID                TermInstance(HINSTANCE);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

VOID                InitLand();
VOID                TermLand();
VOID                InitRobot();
VOID                RandomStaticBush();
VOID                RandomDynamicBush();
VOID                CreateDynamicBush(const qtb::Area& area); 
VOID                RemoveDynamicBush(unsigned int bushID);

VOID                OnUpdate();
VOID                GetMouseArea(qtb::Area& area);

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
                    RemoveDynamicBush(*it);
                    bushIDList.erase(it);
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
            case ID_VIEW_ROBOT:
                {
                    bViewRobot = !bViewRobot;
                    UINT check = bViewRobot ? MF_CHECKED : MF_UNCHECKED;
                    CheckMenuItem(hmenuBar, ID_VIEW_ROBOT, MF_BYCOMMAND | check);
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
            bLMouseDown = TRUE;
        }
        break;
    case WM_LBUTTONUP:
        {
            cursorUpPos.X = GET_X_LPARAM(lParam);
            cursorUpPos.Y = GET_Y_LPARAM(lParam);
            bLMouseDown = FALSE;

            if (land)
            {
                qtb::Area area;
                GetMouseArea(area);
                if(area.width() > 1 && area.height() > 1)
                    CreateDynamicBush(area);
            }
        }
        break;
    case WM_RBUTTONDOWN:
        {
            bRMouseDown = TRUE;

            if (cursorBushID != -1)
                RemoveDynamicBush(cursorBushID);
        }
        break;
    case WM_RBUTTONUP:
    {
        bRMouseDown = FALSE;
    }
    break;
    case WM_MOUSEMOVE:
        {
            cursorPos.X = GET_X_LPARAM(lParam);
            cursorPos.Y = GET_Y_LPARAM(lParam);

            if (!land)
                break;

            cursorBushGroupID = -1;
            cursorBushID = -1;

            perfTool.Start();
            land->bushContains((float)cursorPos.X, (float)cursorPos.Y, &cursorBushGroupID, &cursorBushID);
            dBushCrossTime = perfTool.End();

            dBushCrossTimeTotal += dBushCrossTime;
            ++nBushCrossCount;
            dBushCrossTimeAvg = dBushCrossTimeTotal / nBushCrossCount;

            if (bRMouseDown && cursorBushID != -1)
                RemoveDynamicBush(cursorBushID);
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

VOID InitLand()
{
    srand((unsigned int)time(NULL));

    qtb::Area area(0, LAND_WIDTH, 0, LAND_HEIGHT);
    land = new qtb::Land(area);
    land->devide(MIN_ZONE_SIZE);

    //RandomStaticBush();
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
        float w = RangeRand(RAND_AREA_SIZE_MIN, RAND_AREA_SIZE_MAX) * 0.5f;
        float h = RangeRand(RAND_AREA_SIZE_MIN, RAND_AREA_SIZE_MAX) * 0.5f;

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
        float w = RangeRand(RAND_AREA_SIZE_MIN, RAND_AREA_SIZE_MAX) * 0.5f;
        float h = RangeRand(RAND_AREA_SIZE_MIN, RAND_AREA_SIZE_MAX) * 0.5f;

        CreateDynamicBush(qtb::Area(x - w, x + w, y - h, y + h));
    }
}

VOID CreateDynamicBush(const qtb::Area& area)
{
    if (!land)
        return;

    unsigned int bushID = -1;

    perfTool.Start();
    bushID = land->createBush(area);
    dCreateBushTime = perfTool.End();

    dCreateBushTimeTotal += dCreateBushTime;
    ++nCreateBushCount;
    dCreateBushTimeAvg = dCreateBushTimeTotal / nCreateBushCount;

    bushIDList.push_back(bushID);
}

VOID OnUpdate()
{
    if (!land)
        return;

    if (bViewRobot)
    {
        for (std::list<Robot>::iterator it = robotList.begin(); it != robotList.end(); ++it)
        {
            it->Tick(GetTickCount() / 1000.0f);
            unsigned int bushGroupID = -1;

            perfTool.Start();
            land->bushContains(it->x(), it->y(), &bushGroupID);
            dBushCrossTime = perfTool.End();

            dBushCrossTimeTotal += dBushCrossTime;
            ++nBushCrossCount;
            dBushCrossTimeAvg = dBushCrossTimeTotal / nBushCrossCount;

            it->setBushGroupID(bushGroupID);
        }
    }
}

VOID RemoveDynamicBush(unsigned int bushID)
{
    if (!land)
        return;

    bool removed = false;

    perfTool.Start();
    removed = land->removeBush(bushID);
    dRemoveBushTime = perfTool.End();

    dRemoveBushTimeTotal += dRemoveBushTime;
    ++nRemoveBushCount;
    dRemoveBushTimeAvg = dRemoveBushTimeTotal / nRemoveBushCount;

    if(removed)
        cursorBushID = -1;
}

VOID GetMouseArea(qtb::Area& area)
{
    area.left = (float)(cursorDownPos.X < cursorPos.X ? cursorDownPos.X : cursorPos.X);
    area.bottom = (float)(cursorDownPos.Y < cursorPos.Y ? cursorDownPos.Y : cursorPos.Y);
    area.right = area.left + abs(cursorDownPos.X - cursorPos.X);
    area.top = area.bottom + abs(cursorDownPos.Y - cursorPos.Y);
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
    RectF rc(area.left, area.bottom, area.width(), area.height());
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
        RectF rc(it->left, it->bottom, it->width(), it->height());
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
        RectF rc(area.left, area.bottom, area.width(), area.height());

        Pen pen(drawData.GetBushRes(bush->id()).color);
        graphics.DrawRectangle(&pen, rc);
    }
}

VOID DrawDynamicBush(Graphics& graphics)
{
    if (!land)
        return;

    const qtb::BushPMap& dynamicBush = land->bushes();
    for (qtb::BushPMap::const_iterator it = dynamicBush.begin(); it != dynamicBush.end(); ++it)
    {
        const qtb::Bush* bush = it->second;
        const qtb::Area& area = bush->overall();
        RectF rc(area.left, area.bottom, area.width(), area.height());

        SolidBrush brush(Color(128, 0, 192, 128));
        graphics.FillRectangle(&brush, rc);

        //Pen pen(drawData.GetBushRes(bush->id()).color);
        //graphics.DrawRectangle(&pen, rc);
    }
}

VOID DrawRobot(Graphics& graphics)
{
    for (std::list<Robot>::iterator it = robotList.begin(); it != robotList.end(); ++it)
    {
        Pen pen(drawData.GetBushGroupRes(it->getBrushGroupID()).color);
        RectF rc(it->x()-2.5f, it->y()-2.5f, 5.0f, 5.0f);
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
    RectF rc(area.left, area.bottom, area.width(), area.height());
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
        RectF rc(area.left, area.bottom, area.width(), area.height());

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
    Font myFont(L"Arial", 12);
    SolidBrush blackBrush(Color(255, 64, 64, 64));
    SolidBrush greyBrush(Color(255, 128, 128, 128));

    TCHAR string[64] = _T("");

    // generate static bush
    PointF origin(area.right + MIN_ZONE_SIZE, 0.0f);
    _sntprintf_s(string, 64, _T("RebuildTime: %f"), dRebuildTime);
    graphics.DrawString( string, (INT)_tcslen(string), &myFont, origin, &blackBrush);

    origin = origin + PointF(0.0f, 20.0f);
    _sntprintf_s(string, 64, _T("RebuildTimeAvg: %f"), dRebuildTimeAvg);
    graphics.DrawString(string, (INT)_tcslen(string), &myFont, origin, &greyBrush);

    // create dynamic bush
    origin = origin + PointF(0.0f, 30.0f);
    _sntprintf_s(string, 64, _T("CreateBushTime: %f"), dCreateBushTime);
    graphics.DrawString(string, (INT)_tcslen(string), &myFont, origin, &blackBrush);

    origin = origin + PointF(0.0f, 20.0f);
    _sntprintf_s(string, 64, _T("CreateBushTimeAvg: %f"), dCreateBushTimeAvg);
    graphics.DrawString(string, (INT)_tcslen(string), &myFont, origin, &greyBrush);

    // remove dynamic bush
    origin = origin + PointF(0.0f, 30.0f);
    _sntprintf_s(string, 64, _T("RemoveBushTime: %f"), dRemoveBushTime);
    graphics.DrawString(string, (INT)_tcslen(string), &myFont, origin, &blackBrush);

    origin = origin + PointF(0.0f, 20.0f);
    _sntprintf_s(string, 64, _T("RemoveBushTimeAvg: %f"), dRemoveBushTimeAvg);
    graphics.DrawString(string, (INT)_tcslen(string), &myFont, origin, &greyBrush);

    // cursor cross
    origin = origin + PointF(0.0f, 30.0f);
    _sntprintf_s(string, 64, _T("BushCrossTime: %f"), dBushCrossTime);
    graphics.DrawString(string, (INT)_tcslen(string), &myFont, origin, &blackBrush);

    origin = origin + PointF(0.0f, 20.0f);
    _sntprintf_s(string, 64, _T("BushCrossTimeAvg: %f"), dBushCrossTimeAvg);
    graphics.DrawString(string, (INT)_tcslen(string), &myFont, origin, &greyBrush);

    origin = origin + PointF(0.0f, 30.0f);
    _sntprintf_s(string, 64, _T("PointBushGroup: %d"), cursorBushGroupID);
    SolidBrush bushGroupBrush(drawData.GetBushGroupRes(cursorBushGroupID).color);
    graphics.DrawString(string, (INT)_tcslen(string), &myFont, origin, &bushGroupBrush);

    origin = origin + PointF(0.0f, 20.0f);
    _sntprintf_s(string, 64, _T("PointBush: %d"), cursorBushID);
    SolidBrush bushBrush(drawData.GetBushRes(cursorBushID).color);
    graphics.DrawString(string, (INT)_tcslen(string), &myFont, origin, &bushBrush);
}
