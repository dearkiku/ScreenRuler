#include "RulerWi.h"
#include <cmath>  // �� #include <math.h>
#include "gloab.h"

// ���캯������ʼ����ߴ��ڵ���ز���
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
	// �����ṩ������ߴ���㵥λ���ؼ��
	horizontalUnitPerPixel = CalculateUnitPerPixel(canvasWidth, physicalWidth);
	verticalUnitPerPixel = CalculateUnitPerPixel(canvasHeight, physicalHeight);
}

// ��������
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
	wcex.hbrBackground = CreateSolidBrush(rulerColor);  // ����ɫ
	wcex.lpszClassName = L"RulerClass";
	RegisterClassEx(&wcex);
}

// ��������������ߴ���
void RulerWi::CreateScaleWindow(HINSTANCE hInstance, HWND parent)
{
	RegisterRulerClass(hInstance);
	hWndScaleWindow = CreateWindowEx(
		WS_EX_LAYERED, // ʹ�÷ֲ㴰����ʽ
		L"RulerClass", nullptr,
		WS_CHILD | WS_VISIBLE | WS_POPUP, // ʹ��POPUP��ʽ�����ޱ߿򴰿�
		0, 0, canvasWidth, canvasHeight,
		parent, nullptr, hInstance, nullptr);
	// ������������������HRGN�����ֱ��Ӧ�����������
	HRGN hRgnx = CreateRectRgn(0, 0, canvasWidth, RULER_SIZE); // ˮƽ��������
	HRGN hRgny = CreateRectRgn(0, 0, RULER_SIZE, canvasHeight); // ��ֱ��������
	// �����ϲ��������
	HRGN hRgnCombined = CreateRectRgn(0, 0, 0, 0); // �ϲ��������
	CombineRgn(hRgnCombined, hRgnx, hRgny, RGN_OR); // �ϲ��������������
	// ���úϲ�������򵽴��ڣ��������ֻ������Щ��������Ӧ����¼�
	SetWindowRgn(hWndScaleWindow, hRgnCombined, TRUE);
	// ���ô��ڵ�͸����
	SetLayeredWindowAttributes(hWndScaleWindow, 0, 255, LWA_ALPHA); // ����Ϊ128��͸��
	// ��ʾ����
	ShowWindow(hWndScaleWindow, SW_SHOW);
	UpdateWindow(hWndScaleWindow);
}

// ����ÿ�����ض�Ӧ�ĵ�λ
double RulerWi::CalculateUnitPerPixel(int screenSize, double physicalSize)
{
	if (physicalSize <= 0.0) return 1.0; // Ĭ��ÿ���� 1 ����λ
	return physicalSize / screenSize;
}
static bool needRepaint = true;
// ��Ϣ������
LRESULT CALLBACK RulerWi::RulerWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	RulerWi* pThis = (RulerWi*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

	switch (message)
	{
	case WM_CREATE:
	{
		// ���ô����û����ݣ�ָ��ǰ����
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

			// ��ȡ���ڵĳߴ�
			RECT rect;
			GetClientRect(hWnd, &rect);  // ��ȡ�ͻ�����ߴ�

			// �жϺ��������
			bool isHorizontal = (rect.right - rect.left) > (rect.bottom - rect.top);
			pThis->DrawRuler(hWnd, hdc, isHorizontal);

			EndPaint(hWnd, &ps);
			needRepaint = false;  // ��ɻ��ƺ󣬱��Ϊ����Ҫ�ٴλ���
		}
	}
	break;
	case WM_ERASEBKGND:
		// ��ֹ�������������ⴰ�ڱ�������ȷ
		return 1;
	case WM_MOUSEMOVE:
		// ��������ƶ��¼�
		break;
	case WM_LBUTTONDOWN:
		// ����������¼�
		break;
	case WM_RBUTTONDOWN:
		// �Ҽ�����¼�����
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// ���Ʊ��
void RulerWi::DrawRuler(HWND hWnd, HDC hdc, bool isHorizontal)
{
	int length = isHorizontal ? canvasWidth : canvasHeight;
	double unitPerPixel = isHorizontal ? horizontalUnitPerPixel : verticalUnitPerPixel;

	int bSpacing = RULER_SIZE * 0.8; // ���̶���
	int mSpacing = RULER_SIZE * 0.6; // �̶̿���
	// ��������
	HPEN hPen = CreatePen(PS_SOLID, 1, lineColor);
	SelectObject(hdc, hPen);
	// �������ƿ̶���
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
