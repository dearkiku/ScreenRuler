#include "Line.h"
#include <windows.h>
#include <vector>
#include "gloab.h"
#include "CommCtrl.h"
std::vector<HWND> Line::lines;  // 初始化存储线段控件的句柄容器
// 线的长度固定为length 对齐默认为最边缘 所以确定横竖线之后只需要两个参数即可操作
Line::Line(int location, int length, bool isVertical)
	: location(location), length(length), isVertical(isVertical)
{
	CreateLine();
}

Line::~Line()
{
	if (hwndLine)
	{
		DestroyWindow(hwndLine);  // 销毁线段控件
	}
}

void Line::CreateLine()
{
	// 创建STATIC控件并子类化它
	if (isVertical)
	{
		hwndLine = CreateWindowEx(
			0,
			WC_STATIC, L"",
			WS_CHILD | WS_VISIBLE  | SS_ETCHEDVERT,  // 子控件，能接收消息
			location, 0, LINE_WIDTH, length,
			canvashWnd, nullptr, hInst, nullptr
		);
	}
	else
	{
		hwndLine = CreateWindowEx(
			0,
			WC_STATIC, L"",
			WS_CHILD | WS_VISIBLE | SS_ETCHEDHORZ,  // 子控件，能接收消息 | SS_NOTIFY |
			0, location, length, LINE_WIDTH,
			canvashWnd, nullptr, hInst, nullptr
		);
	}

	if (hwndLine)
	{
		// 将句柄存储在容器中
		lines.push_back(hwndLine);
		// 设置子类化过程
		SetWindowLongPtr(hwndLine, GWLP_WNDPROC, (LONG_PTR)LineWndProc);
		// SetWindowLongPtr(hwndLine, GWLP_USERDATA, (LONG_PTR)this);
		SetWindowLongPtr(hwndLine, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
	}
	else
	{
		OutputDebugString(L"Line创建失败了");
	}
}

void Line::MoveLine(int location)
{
	this->location = location;
	// 垂直线顶边为0
	if (isVertical) {

		SetWindowPos(hwndLine, HWND_TOP, location, 0, LINE_WIDTH, length, SWP_NOZORDER | SWP_NOSIZE);
		return;
	}
	// 水平线左边为0
	SetWindowPos(hwndLine, HWND_TOP, 0, location, length, LINE_WIDTH, SWP_NOZORDER | SWP_NOSIZE);
}

void Line::UpdateLines(HWND hwndParent)
{
	for (auto hwnd : lines)
	{
		// 每次更新时，重绘线段
		InvalidateRect(hwnd, nullptr, TRUE);
	}
}

// 静态的子类化窗口过程
LRESULT CALLBACK LineWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	Line* pThis = reinterpret_cast<Line*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

	switch (msg)
	{
	case WM_CREATE:
		//// 在创建时将 Line 对象与窗口关联
		//pThis = new Line(100, mainRect.h, true);  // 默认竖线
		//SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));

		// OutputDebugString(L"线已经被创建\n");
		break;

	case WM_PAINT:
	{
		if (pThis)
		{
			RECT rect;
			GetWindowRect(hWnd, &rect);
			// OutputDebugString(L"线 WM_PAINT\n");
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
			// SetBkColor(hdc, RGB(255, 200, 200));
			// 使用 FillRect 填充背景
			// FillRect(hdc, &rect, (HBRUSH)GetStockObject(SOLID_BRUSH));
			FillRect(hdc, &rect, (HBRUSH)GetStockObject(WHITE_BRUSH));  // 填充背景
			EndPaint(hWnd, &ps);
		}
	}
	break;
	case WM_CTLCOLORSTATIC:  // 控制控件的背景颜色
	{
		//if (pThis)
		//{
		//	HDC hdcStatic = (HDC)wParam;
		//	// SetBkMode(hdcStatic, TRANSPARENT);  // 透明背景
		//	SetBkColor(hdcStatic, RGB(255, 0, 0));
		//	// SetTextColor(hdcStatic, LINE_COLOR);  // 设置文本颜色，线条颜色
		//	return (LRESULT)CreateSolidBrush(RGB(255, 0, 0));  // 返回线段颜色的画刷
		//}

		HDC hdcStatic = (HDC)wParam;
		SetBkColor(hdcStatic, RGB(255, 0, 0));  // 设置背景颜色为红色
		return (LRESULT)GetStockObject(NULL_BRUSH);  // 返回透明画刷
	}
	break;
	case WM_DESTROY:
		delete pThis;
		break;
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}

	return 0;
}
