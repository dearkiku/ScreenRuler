#include "Line.h"
#include <windows.h>
#include <vector>
#include "gloab.h"
#include "CommCtrl.h"
std::vector<HWND> Line::lines;  // ��ʼ���洢�߶οؼ��ľ������
// �ߵĳ��ȹ̶�Ϊlength ����Ĭ��Ϊ���Ե ����ȷ��������֮��ֻ��Ҫ�����������ɲ���
Line::Line(int location, int length, bool isVertical)
	: location(location), length(length), isVertical(isVertical)
{
	CreateLine();
}

Line::~Line()
{
	if (hwndLine)
	{
		DestroyWindow(hwndLine);  // �����߶οؼ�
	}
}

void Line::CreateLine()
{
	// ����STATIC�ؼ������໯��
	if (isVertical)
	{
		hwndLine = CreateWindowEx(
			0,
			WC_STATIC, L"",
			WS_CHILD | WS_VISIBLE  | SS_ETCHEDVERT,  // �ӿؼ����ܽ�����Ϣ
			location, 0, LINE_WIDTH, length,
			canvashWnd, nullptr, hInst, nullptr
		);
	}
	else
	{
		hwndLine = CreateWindowEx(
			0,
			WC_STATIC, L"",
			WS_CHILD | WS_VISIBLE | SS_ETCHEDHORZ,  // �ӿؼ����ܽ�����Ϣ | SS_NOTIFY |
			0, location, length, LINE_WIDTH,
			canvashWnd, nullptr, hInst, nullptr
		);
	}

	if (hwndLine)
	{
		// ������洢��������
		lines.push_back(hwndLine);
		// �������໯����
		SetWindowLongPtr(hwndLine, GWLP_WNDPROC, (LONG_PTR)LineWndProc);
		// SetWindowLongPtr(hwndLine, GWLP_USERDATA, (LONG_PTR)this);
		SetWindowLongPtr(hwndLine, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
	}
	else
	{
		OutputDebugString(L"Line����ʧ����");
	}
}

void Line::MoveLine(int location)
{
	this->location = location;
	// ��ֱ�߶���Ϊ0
	if (isVertical) {

		SetWindowPos(hwndLine, HWND_TOP, location, 0, LINE_WIDTH, length, SWP_NOZORDER | SWP_NOSIZE);
		return;
	}
	// ˮƽ�����Ϊ0
	SetWindowPos(hwndLine, HWND_TOP, 0, location, length, LINE_WIDTH, SWP_NOZORDER | SWP_NOSIZE);
}

void Line::UpdateLines(HWND hwndParent)
{
	for (auto hwnd : lines)
	{
		// ÿ�θ���ʱ���ػ��߶�
		InvalidateRect(hwnd, nullptr, TRUE);
	}
}

// ��̬�����໯���ڹ���
LRESULT CALLBACK LineWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	Line* pThis = reinterpret_cast<Line*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

	switch (msg)
	{
	case WM_CREATE:
		//// �ڴ���ʱ�� Line �����봰�ڹ���
		//pThis = new Line(100, mainRect.h, true);  // Ĭ������
		//SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));

		// OutputDebugString(L"���Ѿ�������\n");
		break;

	case WM_PAINT:
	{
		if (pThis)
		{
			RECT rect;
			GetWindowRect(hWnd, &rect);
			// OutputDebugString(L"�� WM_PAINT\n");
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
			// SetBkColor(hdc, RGB(255, 200, 200));
			// ʹ�� FillRect ��䱳��
			// FillRect(hdc, &rect, (HBRUSH)GetStockObject(SOLID_BRUSH));
			FillRect(hdc, &rect, (HBRUSH)GetStockObject(WHITE_BRUSH));  // ��䱳��
			EndPaint(hWnd, &ps);
		}
	}
	break;
	case WM_CTLCOLORSTATIC:  // ���ƿؼ��ı�����ɫ
	{
		//if (pThis)
		//{
		//	HDC hdcStatic = (HDC)wParam;
		//	// SetBkMode(hdcStatic, TRANSPARENT);  // ͸������
		//	SetBkColor(hdcStatic, RGB(255, 0, 0));
		//	// SetTextColor(hdcStatic, LINE_COLOR);  // �����ı���ɫ��������ɫ
		//	return (LRESULT)CreateSolidBrush(RGB(255, 0, 0));  // �����߶���ɫ�Ļ�ˢ
		//}

		HDC hdcStatic = (HDC)wParam;
		SetBkColor(hdcStatic, RGB(255, 0, 0));  // ���ñ�����ɫΪ��ɫ
		return (LRESULT)GetStockObject(NULL_BRUSH);  // ����͸����ˢ
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
