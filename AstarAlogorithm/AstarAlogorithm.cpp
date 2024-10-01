// AstarAlogorithm.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include <array>
#include <list>
#include <cmath>
#include <algorithm>
#include "framework.h"
#include "AstarAlogorithm.h"
#define MAX_LOADSTRING 100

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

// >> :
#define IDC_Btn_Mode 10001
#define IDC_Btn_Simulation 10002
#define IDC_Btn_Clear 10003
#define IDC_Btn_Start 10004
#define IDC_Btn_Destination 10005
#define IDC_Btn_Wall 10006


#define MAX_X_POINT 18
#define MAX_Y_POINT 9
#define MOVE_POINT_SIZE 100
#define BTN_SPACE_SIZE 100

enum class MovePointState {DEFAULT ,WALL, START, DESTINATION, OPENLIST, CLOSELIST, PATH};
struct MovePoint
{
    int total;
    int startCost;
    int endCost;
    RECT rect;
    MovePoint* parent;
    MovePointState state;
};

std::array<std::array<MovePoint, MAX_X_POINT>, MAX_Y_POINT> movePoint;
std::list<MovePoint*> openList;

RECT windowSize{ 0,0,1800,1000 };
SIZE mapSize{ 1800, 1000 };

HWND BtnMode, BtnSimulation, BtnClear, BtnStart, BtnDestination, BtnWall;
HDC hdc, hMemDC, tempDC;
HBITMAP hBitmap, hOldBitmap;
HBRUSH hBlackBrush, hGreenBrush, hRedBrush, hNullBrush, hYellowBrush, hBlueBrush, hPinkBrush,hOldBrush;
PAINTSTRUCT ps;
BOOL isCreateMode;

POINT ptMouse;
POINT ptStart, ptDestination;

TCHAR curMode[20];
TCHAR mousePos[128];

MovePoint* curPath;

const int WeightDiagonal = 14;
const int WeightUDLR = 10;

void DrawMap(HDC hdc);

void CreateDoubbleBuffering(HWND hWnd);

void EndDoubleBuffering(HWND hWnd);

void MapClear();

void CreateMap(MovePointState state);

//bool searchAround(int x, int y);
bool searchAround(MovePoint& selectPoint);
int heuristic(int x, int y, POINT endP);

MovePoint* getPath();
// <<

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 여기에 코드를 입력합니다.

    // 전역 문자열을 초기화합니다.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_ASTARALOGORITHM, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 애플리케이션 초기화를 수행합니다:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_ASTARALOGORITHM));

    MSG msg;

    // 기본 메시지 루프입니다:
    while (true)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                break;
            }
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        {

        }
    }

    return (int) msg.wParam;
}



//
//  함수: MyRegisterClass()
//
//  용도: 창 클래스를 등록합니다.
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
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ASTARALOGORITHM));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_ASTARALOGORITHM);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   함수: InitInstance(HINSTANCE, int)
//
//   용도: 인스턴스 핸들을 저장하고 주 창을 만듭니다.
//
//   주석:
//
//        이 함수를 통해 인스턴스 핸들을 전역 변수에 저장하고
//        주 프로그램 창을 만든 다음 표시합니다.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, CW_USEDEFAULT, windowSize.right+25, windowSize.bottom+75, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  용도: 주 창의 메시지를 처리합니다.
//
//  WM_COMMAND  - 애플리케이션 메뉴를 처리합니다.
//  WM_PAINT    - 주 창을 그립니다.
//  WM_DESTROY  - 종료 메시지를 게시하고 반환합니다.
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static MovePointState btnState;
    switch (message)
    {
    case WM_CREATE:
        
        isCreateMode = false;
        BtnMode = CreateWindow(_T("button"), _T("ModeChange"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 100, 25, 100, 60, hWnd, (HMENU)IDC_Btn_Mode, hInst, NULL);
        BtnSimulation = CreateWindow(_T("button"), _T("Simulation"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 250, 25, 100, 60, hWnd, (HMENU)IDC_Btn_Simulation, hInst, NULL);
        BtnClear = CreateWindow(_T("button"), _T("Clear"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 400, 25, 100, 60, hWnd, (HMENU)IDC_Btn_Clear, hInst, NULL);

        BtnStart = CreateWindow(_T("button"), _T("Start"), WS_CHILD | BS_PUSHBUTTON, 800 , 25, 100, 60, hWnd, (HMENU)IDC_Btn_Start, hInst, NULL);
        BtnDestination = CreateWindow(_T("button"), _T("Destination"), WS_CHILD | BS_PUSHBUTTON, 950, 25, 100, 60, hWnd, (HMENU)IDC_Btn_Destination, hInst, NULL);
        BtnWall = CreateWindow(_T("button"), _T("Wall"), WS_CHILD | BS_PUSHBUTTON, 1100, 25, 100, 60, hWnd, (HMENU)IDC_Btn_Wall, hInst, NULL);

        btnState = MovePointState::DEFAULT;
        hBlackBrush = CreateSolidBrush(RGB(0,0,0));
        hRedBrush = CreateSolidBrush(RGB(205, 10, 10));
        hGreenBrush = CreateSolidBrush(RGB(10, 205, 10));
        hBlueBrush = CreateSolidBrush(RGB(10, 10, 205));
        hYellowBrush = CreateSolidBrush(RGB(255, 212, 0));
        hPinkBrush = CreateSolidBrush(RGB(255, 122, 203));
        hNullBrush = (HBRUSH)GetStockObject(NULL_BRUSH);

        ptStart = { -1, -1 };
        ptDestination = { -1,-1 };

        MapClear();
        break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 메뉴 선택을 구문 분석합니다:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            case IDC_Btn_Mode:
                isCreateMode = !isCreateMode;
                if (isCreateMode)
                {
                    ShowWindow(BtnStart, SW_SHOW);
                    ShowWindow(BtnDestination, SW_SHOW);
                    ShowWindow(BtnWall, SW_SHOW);
                }
                else
                {
                    ShowWindow(BtnStart, SW_HIDE);
                    ShowWindow(BtnDestination, SW_HIDE);
                    ShowWindow(BtnWall, SW_HIDE);
                }
                InvalidateRect(hWnd, NULL, TRUE);
                break;
            case IDC_Btn_Start:
                btnState = MovePointState::START;
                break;
            case IDC_Btn_Destination:
                btnState = MovePointState::DESTINATION;
                break;
            case IDC_Btn_Wall:
                btnState = MovePointState::WALL;
            case IDC_Btn_Simulation:
                if (!isCreateMode && ptStart.x >= 0 && ptDestination.x >= 0) 
                {
                    /*searchAround(*curPath);
                    InvalidateRect(hWnd, NULL, TRUE);*/
                    while (searchAround(*curPath) == false)
                    {
                        curPath = getPath();
                    }

                    MovePoint* p = movePoint[ptDestination.y][ptDestination.x].parent;
                    while (p != &movePoint[ptStart.y][ptStart.x])
                    {
                        p->state = MovePointState::PATH;
                        p = p->parent;
                    }
                        InvalidateRect(hWnd, NULL, TRUE);

                }
                break;
            case IDC_Btn_Clear:
                MapClear();
                InvalidateRect(hWnd, NULL, TRUE);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_MOUSEMOVE:
        ptMouse.x = LOWORD(lParam);
        ptMouse.y = HIWORD(lParam);

        InvalidateRect(hWnd, NULL, false);
        break;
    case WM_LBUTTONDOWN:
        if (isCreateMode) 
        {
            CreateMap(btnState);
            InvalidateRect(hWnd, NULL, true);
        } 
        
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: 여기에 hdc를 사용하는 그리기 코드를 추가합니다...
            DrawMap(hdc);
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        DeleteObject(hRedBrush);
        DeleteObject(hBlackBrush);
        DeleteObject(hGreenBrush);
        DeleteObject(hNullBrush);
        DeleteObject(hBlueBrush);
        DeleteObject(hYellowBrush);
        DeleteObject(hPinkBrush);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// 정보 대화 상자의 메시지 처리기입니다.
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


void DrawMap(HDC hdc)
{
    if (isCreateMode) _tcscpy_s(curMode, _countof(curMode), _T("Mode: Create"));
    else _tcscpy_s(curMode, _countof(curMode), _T("Mode: Simulation"));
    TextOut(hdc, 10, 10, curMode, _tcslen(curMode));
  
    for (int i = 0; i < MAX_Y_POINT; i++)
    {
        for (int j = 0; j < MAX_X_POINT; j++)
        {
            switch (movePoint[i][j].state)
            {
            case MovePointState::WALL:
                hOldBrush = (HBRUSH)SelectObject(hdc, hBlackBrush);
                break;
            case MovePointState::START:
                hOldBrush = (HBRUSH)SelectObject(hdc, hGreenBrush);
                break;
            case MovePointState::DESTINATION:
                hOldBrush = (HBRUSH)SelectObject(hdc, hRedBrush);
                break;
            case MovePointState::CLOSELIST:
                hOldBrush = (HBRUSH)SelectObject(hdc, hYellowBrush);
                break;
            case MovePointState::OPENLIST:
                hOldBrush = (HBRUSH)SelectObject(hdc, hYellowBrush);
                break;
            case MovePointState::PATH:
                hOldBrush = (HBRUSH)SelectObject(hdc, hPinkBrush);
                break;
            default:
                hOldBrush = (HBRUSH)SelectObject(hdc, hNullBrush);
                break;
            }
            Rectangle(hdc, movePoint[i][j].rect.left, movePoint[i][j].rect.top, movePoint[i][j].rect.right, movePoint[i][j].rect.bottom);
            SelectObject(hdc, hOldBrush);
            if (movePoint[i][j].state == MovePointState::WALL) continue;
            else if (movePoint[i][j].state == MovePointState::DESTINATION) continue;
            TCHAR buf[20]; 
            RECT textBox = { movePoint[i][j].rect.left + 5,movePoint[i][j].rect.top + 5, movePoint[i][j].rect.right - 5, movePoint[i][j].rect.bottom+10};
            wsprintf(buf, L"%d", movePoint[i][j].startCost); 
            DrawText(hdc, buf, wcslen(buf), &textBox, DT_SINGLELINE | DT_LEFT);
            wsprintf(buf, L"%d", movePoint[i][j].endCost);
            DrawText(hdc, buf, wcslen(buf), &textBox, DT_SINGLELINE | DT_RIGHT);
            wsprintf(buf, L"%d", movePoint[i][j].total);
            DrawText(hdc, buf, wcslen(buf), &textBox, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
        }
    }


    wsprintf(mousePos, TEXT("Mouse Position : (%04d, %04d_)"), ptMouse.x, ptMouse.y);
    TextOut(hdc, 1550, 10, mousePos, lstrlen(mousePos));
}


void CreateDoubbleBuffering(HWND hWnd)
{
    hdc = BeginPaint(hWnd, &ps);

    GetClientRect(hWnd, &windowSize);
    hMemDC = CreateCompatibleDC(hdc);
    hBitmap = CreateCompatibleBitmap(hdc, windowSize.right, windowSize.bottom);
    hOldBitmap = (HBITMAP)SelectObject(hMemDC, hBitmap);
    PatBlt(hMemDC, 0, 0, windowSize.right, windowSize.bottom, WHITENESS);
    tempDC = hdc;
    hdc = hMemDC;
    hMemDC = tempDC;
}

void EndDoubleBuffering(HWND hWnd)
{
    tempDC = hdc;
    hdc = hMemDC;
    hMemDC = tempDC;
    GetClientRect(hWnd, &windowSize);
    BitBlt(hdc, 0, 0, windowSize.right, windowSize.bottom, hMemDC, 0, 0, SRCCOPY);
    SelectObject(hMemDC, hOldBitmap);
    DeleteObject(hBitmap);
    DeleteDC(hMemDC);
    EndPaint(hWnd, &ps);
}

void MapClear()
{
    openList.clear();
    for (int i = 0; i < MAX_Y_POINT; i++)
    {
        for (int j = 0; j < MAX_X_POINT; j++)
        {
            int left = j * MOVE_POINT_SIZE;
            int top = i * MOVE_POINT_SIZE + BTN_SPACE_SIZE;
            movePoint[i][j].rect = { left, top, left + MOVE_POINT_SIZE, top + MOVE_POINT_SIZE };
            movePoint[i][j].total = 0;
            movePoint[i][j].endCost = 0;
            movePoint[i][j].startCost = 0;
            movePoint[i][j].state = MovePointState::DEFAULT;
        }
    }
}

void CreateMap(MovePointState state)
{
    int x = ptMouse.x / MOVE_POINT_SIZE;
    int y = (ptMouse.y - BTN_SPACE_SIZE) / MOVE_POINT_SIZE ;

     switch (state)
     {
     case MovePointState::START:
         if (ptStart.x != -1)
         {
             movePoint[ptStart.y][ptStart.x].state = MovePointState::DEFAULT;
         }
         ptStart = { x, y };
         curPath = &movePoint[ptStart.y][ptStart.x];
        break;
     case MovePointState::DESTINATION:
         if (ptDestination.x != -1)
         {
             movePoint[ptDestination.y][ptDestination.x].state = MovePointState::DEFAULT;
         }
         ptDestination = { x, y };
        break;
     }

    movePoint[y][x].total = 0;
    movePoint[y][x].startCost = 0;
    movePoint[y][x].endCost = 0;
    movePoint[y][x].state = state;
    movePoint[y][x].parent = nullptr;
}


bool searchAround(MovePoint& selectPoint)
{
    int x = selectPoint.rect.left / MOVE_POINT_SIZE;
    int y = (selectPoint.rect.top - BTN_SPACE_SIZE) / MOVE_POINT_SIZE;

    

    for (int i = -1; i < 2; i++)
    {
        if (y + i < 0 || y + i >= MAX_Y_POINT) continue;
        for (int j = -1; j < 2; j++)
        {
            if (x + j < 0 || x + j >= MAX_X_POINT) continue;
            if (i == 0 && j == 0) continue;

            MovePoint& curPoint = movePoint[y + i][x + j];

            if (curPoint.state == MovePointState::WALL || curPoint.state == MovePointState::CLOSELIST) continue;
            if (curPoint.state == MovePointState::START) continue;
            if (curPoint.state == MovePointState::DESTINATION)
            {
                curPoint.parent = &selectPoint;
                return true;
            }

            if (curPoint.state == MovePointState::DEFAULT)
            {
                if (i == 0 || j == 0)    curPoint.startCost = selectPoint.startCost + WeightUDLR;
                else                     curPoint.startCost = selectPoint.startCost + WeightDiagonal;

                curPoint.endCost = heuristic(x+j, y+i, ptDestination);
                curPoint.state = MovePointState::OPENLIST;
                curPoint.parent = &selectPoint;
                curPoint.total = curPoint.endCost + curPoint.startCost;
                openList.push_back(&curPoint);
            }
            else if (curPoint.state == MovePointState::OPENLIST)
            {
                if (i == 0 || j == 0)
                {
                    if (curPoint.startCost > selectPoint.startCost + WeightUDLR)
                    {
                        curPoint.startCost = selectPoint.startCost + WeightUDLR;
                        curPoint.parent = &selectPoint;
                        curPoint.total = curPoint.endCost + curPoint.startCost;
                    }
                }
                else
                {
                    if (curPoint.startCost > selectPoint.startCost + WeightDiagonal)
                    {
                        curPoint.startCost = selectPoint.startCost + WeightDiagonal;
                        curPoint.parent = &selectPoint;
                        curPoint.total = curPoint.endCost + curPoint.startCost;
                    }
                }
            }
        }
    }


    return false;
}

int heuristic(int x, int y, POINT endPoint)
{
    int diagonalNum = 0;

    while (x != endPoint.x && y != endPoint.y)
    {
        diagonalNum++;
        x += (endPoint.x - x) / abs(endPoint.x - x);
        y += (endPoint.y - y) / abs(endPoint.y - y);
    }

    if (x == endPoint.x) return (abs(endPoint.y - y ) * WeightUDLR + diagonalNum * WeightDiagonal);
    if (y == endPoint.y) return (abs(endPoint.x - x ) * WeightUDLR + diagonalNum * WeightDiagonal);
}


MovePoint* getPath()
{
    MovePoint* minPoint;
    std::list<MovePoint*>::iterator minValueIter = openList.begin();

    for (std::list<MovePoint*>::iterator it = openList.begin(); it != openList.end(); it++)
    {
        if ((*minValueIter)->total > (*it)->total)
        {
            minValueIter = it;
        }
    }

     minPoint = *minValueIter;
     if(minPoint->state != MovePointState::START)
        minPoint->state = MovePointState::CLOSELIST;
    minValueIter = openList.erase(minValueIter);

    return minPoint;
}