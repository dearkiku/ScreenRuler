#include "Scale.h"
#include "gloab.h"
// #include "Line.h"

// #define RULERWINDOW_CLASS			L"RulerClass"
#define RULERWINDOW_CLASS			L"ScaleClass"
// 全局变量

// 标线背景色 标线线段色
static COLORREF rulerColor = RGB(31, 31, 31), lineColor = RGB(241, 241, 241);
static bool dragging = false;
static bool create = false;
// 是否正在创建线段
static bool isCreatingLine = false;
// 当前正在创建的线段
// static Line* currentLine = nullptr;
// 是否垂直
static bool isVertical = false;
// 是否按住
static bool hold = false;
// 此代码模块中包含的函数的前向声明:
static ATOM                MyRegisterClass(HINSTANCE hInst);
static LRESULT CALLBACK    RulerWndProc(HWND, UINT, WPARAM, LPARAM);

// 计算每个像素对应的单位
static double CalculateUnitPerPixel(int screenSize, double physicalSize)
{
	if (physicalSize <= 0.0) return 1.0; // 默认每像素 1 个单位
	return physicalSize / screenSize;
}

static ATOM MyRegisterClass(HINSTANCE hInst)
{
	WNDCLASSEX wcex = {};
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = RulerWndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = sizeof(LONG_PTR);
	// wcex.hInst = hInst;
	//wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hCursor = LoadCursor(nullptr, IDC_PIN);
	//wcex.hCursor = LoadCursor(nullptr, IDC_CROSS);
	wcex.hbrBackground = CreateSolidBrush(rulerColor);  // 背景色
	wcex.lpszClassName = RULERWINDOW_CLASS;
	return RegisterClassEx(&wcex);
}

// 消息处理函数
static LRESULT CALLBACK RulerWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

	switch (message)
	{
	case WM_CREATE:
	{

	}
	break;
	case WM_PAINT:
	{
		// 不要删除绘图区注释

		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		// TODO: 在此处添加使用 hdc 的任何绘图代码...

		// 获取客户区矩形
		RECT rc;
		GetClientRect(hWnd, &rc);
		// 创建一个内存设备上下文（双缓冲）
		HDC hdcMem = CreateCompatibleDC(hdc);
		HBITMAP hBitmap = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
		HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMem, hBitmap);

		// 绘制背景
		FillRect(hdcMem, &ps.rcPaint, CreateSolidBrush(rulerColor));

		// 创建画笔
		HPEN hPen = CreatePen(PS_SOLID, 1, lineColor);
		SelectObject(hdcMem, hPen);

		int bSpacing = RULER_SIZE * 0.8; // 长刻度线
		int mSpacing = RULER_SIZE * 0.4; // 短刻度线
		// 遍历绘制水平刻度线
		for (int i = RULER_SIZE; i < rc.right; i += 5)
		{
			// 每5个像素绘制一个长刻度线
			int lineSize = (i % RULER_SIZE == 0) ? bSpacing : mSpacing;
			// 绘制水平刻度线，从上向下
			MoveToEx(hdcMem, i, 0, nullptr);
			LineTo(hdcMem, i, lineSize);
			// 绘制水平刻度线，从下向上
			// MoveToEx(hdcMem, i, rc.bottom - lineSize, nullptr);
			// LineTo(hdcMem, i, rc.bottom);
		}
		// 遍历绘制垂直刻度线
		for (int i = RULER_SIZE; i < rc.bottom; i += 5)
		{
			// 每5个像素绘制一个长刻度线
			int lineSize = (i % RULER_SIZE == 0) ? bSpacing : mSpacing;  // 长刻度和短刻度的区别
			// 绘制垂直刻度线，从左边向右
			MoveToEx(hdcMem, 0, i, nullptr);
			LineTo(hdcMem, lineSize, i);
			// 绘制垂直刻度线，从右向左
			// MoveToEx(hdcMem, rc.right - lineSize, i, nullptr);
			// LineTo(hdcMem, rc.right, i);
		}
		// 将内存设备上下文的内容复制到屏幕
		BitBlt(hdc, 0, 0, rc.right, rc.bottom, hdcMem, 0, 0, SRCCOPY);
		// 清理资源
		DeleteObject(hPen);
		SelectObject(hdcMem, hOldBitmap);
		DeleteObject(hBitmap);
		DeleteDC(hdcMem);
		EndPaint(hWnd, &ps);
	}
	break;
	//case WM_ERASEBKGND:
	//	// 阻止背景擦除，避免窗口背景不正确
	//	return 1;
	case WM_LBUTTONDOWN:
	{
		POINT pt = { LOWORD(lParam), HIWORD(lParam) };
		if (pt.x < mainRect.x + RULER_SIZE && pt.y < mainRect.y + RULER_SIZE)
		{
			return 0;
		}
		dragging = true;
		hold = (pt.y <= RULER_SIZE); // 顶部区域
		SetCapture(hWnd);
		return 0;
	}
	// 处理鼠标点击事件
	break;
	case WM_MOUSEMOVE:
	{
		// 鼠标移动时拖拽为真
		if (dragging) {
			POINT pt = { LOWORD(lParam), HIWORD(lParam) };
			if (!create)
			{
				// 判断鼠标是否还在标尺区域中
				if (pt.x < RULER_SIZE + 1 || pt.y < RULER_SIZE + 1)
				{
					return 0;
				}
				SendMessage(canvashWnd, WM_RULER_DRAG_START, hold, hold ? pt.y : pt.x);
				create = true;
			}
			// 通知主窗口正在移动线
			SendMessage(canvashWnd, WM_RULER_DRAG_MOVE, 0, hold ? pt.y : pt.x);
		}
		return 0;
	}
	break;
	case WM_LBUTTONUP:
	{

		create = false;
		// 释放鼠标捕获
		ReleaseCapture();
		// 释放鼠标时如果拖拽为真
		if (dragging) {
			// 正在拖拽为假
			dragging = false;

			// 通知主窗口结束拖拽
			SendMessage(canvashWnd, WM_RULER_DRAG_END, 0, 0);
		}
		return 0;

		//// 释放鼠标捕捉
		//ReleaseCapture();
		//isCreatingLine = false;
		//currentLine = nullptr;

		//if (isCreatingLine)
		//{
		//	// 松开鼠标时，停止创建线段
		//	isCreatingLine = false;
		//}
		//if (currentLine != nullptr)
		//{
		//	// 重置当前线段
		//	currentLine = nullptr;
		//}
	}
	break;
	case WM_RBUTTONUP:
	{
		// 发送托盘消息让主窗口弹出菜单
		SendMessage(canvashWnd, WM_TRAYICON, 0, WM_RBUTTONUP);
	}
	break;
	//case WM_RBUTTONDOWN:
	//	// 右键点击事件处理
	//	break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}


// 实例句柄 父窗口句柄 画布坐标 物理坐标
void CreateScaleWindow(int canvasX, int canvasY, float physicalW, float physicalH) {
	MyRegisterClass(hInst);
	hWndScaleWindow = CreateWindowEx(
		WS_EX_LAYERED | WS_EX_COMPOSITED, // 使用分层窗口样式 | 双缓冲
		RULERWINDOW_CLASS, L"",
		WS_CHILD | WS_VISIBLE | WS_POPUP, // 使用POPUP样式来做无边框窗口
		canvasX, canvasY, (int)physicalW, (int)physicalH,
		canvashWnd, nullptr, hInst, nullptr);
	if (!hWndScaleWindow)
	{
		return;
	}
	// 创建横向和纵向的区域（HRGN），分别对应横向和纵向标尺
	HRGN hRgnx = CreateRectRgn(0, 0, physicalW, RULER_SIZE); // 水平长条区域
	HRGN hRgny = CreateRectRgn(0, 0, RULER_SIZE, physicalH); // 垂直长条区域
	// 创建合并后的区域
	HRGN hRgnCombined = CreateRectRgn(0, 0, 0, 0);
	CombineRgn(hRgnCombined, hRgnx, hRgny, RGN_OR); // 合并横向和纵向区域
	// 设置合并后的区域到窗口，这样标尺只会在这些区域内响应鼠标事件
	SetWindowRgn(hWndScaleWindow, hRgnCombined, TRUE);
	DeleteObject(hRgnx);
	DeleteObject(hRgny);
	// 设置窗口的透明度
	SetLayeredWindowAttributes(hWndScaleWindow, 0, 128, LWA_ALPHA);
	// 显示窗口
	ShowWindow(hWndScaleWindow, SW_SHOW);
	UpdateWindow(hWndScaleWindow);
	return;// (int)msg.wParam;
}