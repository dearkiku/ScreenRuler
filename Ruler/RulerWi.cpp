#include "RulerWi.h"
#include <cmath>  // 或 #include <math.h>
#include "gloab.h"

// 构造函数，初始化标尺窗口的相关参数
RulerWi::RulerWi(int canvasWidth, int canvasHeight,
	double physicalWidth, double physicalHeight,
	double unitInterval)
	: canvasWidth(canvasWidth),
	canvasHeight(canvasHeight),
	physicalWidth(physicalWidth),
	physicalHeight(physicalHeight),
	unitInterval(unitInterval),
	hWndScaleWindow(nullptr),
	rulerColor(RGB(31, 31, 31)),
	lineColor(RGB(241, 241, 241)),
	horizontalUnitPerPixel(0.0),
	verticalUnitPerPixel(0.0)
{
	// 根据提供的物理尺寸计算单位像素间距
	horizontalUnitPerPixel = CalculateUnitPerPixel(canvasWidth, physicalWidth);
	verticalUnitPerPixel = CalculateUnitPerPixel(canvasHeight, physicalHeight);
}

// 析构函数
RulerWi::~RulerWi() {}

void RulerWi::RegisterRulerClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex = {};
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = RulerWndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = sizeof(LONG_PTR);
	wcex.hInstance = hInstance;
	wcex.hbrBackground = CreateSolidBrush(rulerColor);  // 背景色
	wcex.lpszClassName = L"RulerClass";
	RegisterClassEx(&wcex);
}

// 创建横向和纵向标尺窗口
void RulerWi::CreateScaleWindow(HINSTANCE hInstance, HWND parent)
{
	RegisterRulerClass(hInstance);
	hWndScaleWindow = CreateWindowEx(
		WS_EX_LAYERED, // 使用分层窗口样式
		L"RulerClass", nullptr,
		WS_CHILD | WS_VISIBLE | WS_POPUP, // 使用POPUP样式来做无边框窗口
		0, 0, canvasWidth, canvasHeight,
		parent, nullptr, hInstance, nullptr);
	// 创建横向和纵向的区域（HRGN），分别对应横向和纵向标尺
	HRGN hRgnx = CreateRectRgn(0, 0, canvasWidth, RULER_SIZE); // 水平长条区域
	HRGN hRgny = CreateRectRgn(0, 0, RULER_SIZE, canvasHeight); // 垂直长条区域
	// 创建合并后的区域
	HRGN hRgnCombined = CreateRectRgn(0, 0, 0, 0); // 合并后的区域
	CombineRgn(hRgnCombined, hRgnx, hRgny, RGN_OR); // 合并横向和纵向区域
	// 设置合并后的区域到窗口，这样标尺只会在这些区域内响应鼠标事件
	SetWindowRgn(hWndScaleWindow, hRgnCombined, TRUE);
	// 设置窗口的透明度
	SetLayeredWindowAttributes(hWndScaleWindow, 0, 255, LWA_ALPHA); // 设置为128半透明
	// 显示窗口
	ShowWindow(hWndScaleWindow, SW_SHOW);
	UpdateWindow(hWndScaleWindow);
}

// 计算每个像素对应的单位
double RulerWi::CalculateUnitPerPixel(int screenSize, double physicalSize)
{
	if (physicalSize <= 0.0) return 1.0; // 默认每像素 1 个单位
	return physicalSize / screenSize;
}
static bool needRepaint = true;
// 消息处理函数
LRESULT CALLBACK RulerWi::RulerWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	RulerWi* pThis = (RulerWi*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

	switch (message)
	{
	case WM_CREATE:
	{
		// 设置窗口用户数据，指向当前对象
		pThis = (RulerWi*)((LPCREATESTRUCT)lParam)->lpCreateParams;
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pThis);
	}
	break;
	case WM_PAINT:
	{
		if (needRepaint && pThis)
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);

			// 获取窗口的尺寸
			RECT rect;
			GetClientRect(hWnd, &rect);  // 获取客户区域尺寸

			// 判断横向或纵向
			bool isHorizontal = (rect.right - rect.left) > (rect.bottom - rect.top);
			pThis->DrawRuler(hWnd, hdc, isHorizontal);

			EndPaint(hWnd, &ps);
			needRepaint = false;  // 完成绘制后，标记为不需要再次绘制
		}
	}
	break;
	case WM_ERASEBKGND:
		// 阻止背景擦除，避免窗口背景不正确
		return 1;
	case WM_MOUSEMOVE:
		// 处理鼠标移动事件
		break;
	case WM_LBUTTONDOWN:
		// 处理鼠标点击事件
		break;
	case WM_RBUTTONDOWN:
		// 右键点击事件处理
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// 绘制标尺
void RulerWi::DrawRuler(HWND hWnd, HDC hdc, bool isHorizontal)
{
	int length = isHorizontal ? canvasWidth : canvasHeight;
	double unitPerPixel = isHorizontal ? horizontalUnitPerPixel : verticalUnitPerPixel;

	int bSpacing = RULER_SIZE * 0.8; // 长刻度线
	int mSpacing = RULER_SIZE * 0.6; // 短刻度线
	// 创建画笔
	HPEN hPen = CreatePen(PS_SOLID, 1, lineColor);
	SelectObject(hdc, hPen);
	// 遍历绘制刻度线
	for (int i = 25; i < length; ++i)
	{
		double physicalUnit = i * unitPerPixel;
		if (fmod(physicalUnit, unitInterval) < unitPerPixel)
		{
			int lineSize = fmod(physicalUnit, unitInterval * 5) < unitPerPixel ? bSpacing : mSpacing;
			if (isHorizontal)
			{
				MoveToEx(hdc, i, 25 - lineSize, nullptr);
				LineTo(hdc, i, 25);
			}
			else
			{
				MoveToEx(hdc, 25 - lineSize, i, nullptr);
				LineTo(hdc, 25, i);
			}
		}
	}
	DeleteObject(hPen);
}
