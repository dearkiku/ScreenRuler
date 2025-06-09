#include "Scale.h"
#include "gloab.h"
// #include "Line.h"

// #define RULERWINDOW_CLASS			L"RulerClass"
#define RULERWINDOW_CLASS			L"ScaleClass"
// ȫ�ֱ���

// ���߱���ɫ �����߶�ɫ
static COLORREF rulerColor = RGB(31, 31, 31), lineColor = RGB(241, 241, 241);
static bool dragging = false;
static bool create = false;
// �Ƿ����ڴ����߶�
static bool isCreatingLine = false;
// ��ǰ���ڴ������߶�
// static Line* currentLine = nullptr;
// �Ƿ�ֱ
static bool isVertical = false;
// �Ƿ�ס
static bool hold = false;
// �˴���ģ���а����ĺ�����ǰ������:
static ATOM                MyRegisterClass(HINSTANCE hInst);
static LRESULT CALLBACK    RulerWndProc(HWND, UINT, WPARAM, LPARAM);

// ����ÿ�����ض�Ӧ�ĵ�λ
static double CalculateUnitPerPixel(int screenSize, double physicalSize)
{
	if (physicalSize <= 0.0) return 1.0; // Ĭ��ÿ���� 1 ����λ
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
	wcex.hbrBackground = CreateSolidBrush(rulerColor);  // ����ɫ
	wcex.lpszClassName = RULERWINDOW_CLASS;
	return RegisterClassEx(&wcex);
}

// ��Ϣ������
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
		// ��Ҫɾ����ͼ��ע��

		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		// TODO: �ڴ˴����ʹ�� hdc ���κλ�ͼ����...

		// ��ȡ�ͻ�������
		RECT rc;
		GetClientRect(hWnd, &rc);
		// ����һ���ڴ��豸�����ģ�˫���壩
		HDC hdcMem = CreateCompatibleDC(hdc);
		HBITMAP hBitmap = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
		HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMem, hBitmap);

		// ���Ʊ���
		FillRect(hdcMem, &ps.rcPaint, CreateSolidBrush(rulerColor));

		// ��������
		HPEN hPen = CreatePen(PS_SOLID, 1, lineColor);
		SelectObject(hdcMem, hPen);

		int bSpacing = RULER_SIZE * 0.8; // ���̶���
		int mSpacing = RULER_SIZE * 0.4; // �̶̿���
		// ��������ˮƽ�̶���
		for (int i = RULER_SIZE; i < rc.right; i += 5)
		{
			// ÿ5�����ػ���һ�����̶���
			int lineSize = (i % RULER_SIZE == 0) ? bSpacing : mSpacing;
			// ����ˮƽ�̶��ߣ���������
			MoveToEx(hdcMem, i, 0, nullptr);
			LineTo(hdcMem, i, lineSize);
			// ����ˮƽ�̶��ߣ���������
			// MoveToEx(hdcMem, i, rc.bottom - lineSize, nullptr);
			// LineTo(hdcMem, i, rc.bottom);
		}
		// �������ƴ�ֱ�̶���
		for (int i = RULER_SIZE; i < rc.bottom; i += 5)
		{
			// ÿ5�����ػ���һ�����̶���
			int lineSize = (i % RULER_SIZE == 0) ? bSpacing : mSpacing;  // ���̶ȺͶ̶̿ȵ�����
			// ���ƴ�ֱ�̶��ߣ����������
			MoveToEx(hdcMem, 0, i, nullptr);
			LineTo(hdcMem, lineSize, i);
			// ���ƴ�ֱ�̶��ߣ���������
			// MoveToEx(hdcMem, rc.right - lineSize, i, nullptr);
			// LineTo(hdcMem, rc.right, i);
		}
		// ���ڴ��豸�����ĵ����ݸ��Ƶ���Ļ
		BitBlt(hdc, 0, 0, rc.right, rc.bottom, hdcMem, 0, 0, SRCCOPY);
		// ������Դ
		DeleteObject(hPen);
		SelectObject(hdcMem, hOldBitmap);
		DeleteObject(hBitmap);
		DeleteDC(hdcMem);
		EndPaint(hWnd, &ps);
	}
	break;
	//case WM_ERASEBKGND:
	//	// ��ֹ�������������ⴰ�ڱ�������ȷ
	//	return 1;
	case WM_LBUTTONDOWN:
	{
		POINT pt = { LOWORD(lParam), HIWORD(lParam) };
		if (pt.x < mainRect.x + RULER_SIZE && pt.y < mainRect.y + RULER_SIZE)
		{
			return 0;
		}
		dragging = true;
		hold = (pt.y <= RULER_SIZE); // ��������
		SetCapture(hWnd);
		return 0;
	}
	// ����������¼�
	break;
	case WM_MOUSEMOVE:
	{
		// ����ƶ�ʱ��קΪ��
		if (dragging) {
			POINT pt = { LOWORD(lParam), HIWORD(lParam) };
			if (!create)
			{
				// �ж�����Ƿ��ڱ��������
				if (pt.x < RULER_SIZE + 1 || pt.y < RULER_SIZE + 1)
				{
					return 0;
				}
				SendMessage(canvashWnd, WM_RULER_DRAG_START, hold, hold ? pt.y : pt.x);
				create = true;
			}
			// ֪ͨ�����������ƶ���
			SendMessage(canvashWnd, WM_RULER_DRAG_MOVE, 0, hold ? pt.y : pt.x);
		}
		return 0;
	}
	break;
	case WM_LBUTTONUP:
	{

		create = false;
		// �ͷ���겶��
		ReleaseCapture();
		// �ͷ����ʱ�����קΪ��
		if (dragging) {
			// ������קΪ��
			dragging = false;

			// ֪ͨ�����ڽ�����ק
			SendMessage(canvashWnd, WM_RULER_DRAG_END, 0, 0);
		}
		return 0;

		//// �ͷ���겶׽
		//ReleaseCapture();
		//isCreatingLine = false;
		//currentLine = nullptr;

		//if (isCreatingLine)
		//{
		//	// �ɿ����ʱ��ֹͣ�����߶�
		//	isCreatingLine = false;
		//}
		//if (currentLine != nullptr)
		//{
		//	// ���õ�ǰ�߶�
		//	currentLine = nullptr;
		//}
	}
	break;
	case WM_RBUTTONUP:
	{
		// ����������Ϣ�������ڵ����˵�
		SendMessage(canvashWnd, WM_TRAYICON, 0, WM_RBUTTONUP);
	}
	break;
	//case WM_RBUTTONDOWN:
	//	// �Ҽ�����¼�����
	//	break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}


// ʵ����� �����ھ�� �������� ��������
void CreateScaleWindow(int canvasX, int canvasY, float physicalW, float physicalH) {
	MyRegisterClass(hInst);
	hWndScaleWindow = CreateWindowEx(
		WS_EX_LAYERED | WS_EX_COMPOSITED, // ʹ�÷ֲ㴰����ʽ | ˫����
		RULERWINDOW_CLASS, L"",
		WS_CHILD | WS_VISIBLE | WS_POPUP, // ʹ��POPUP��ʽ�����ޱ߿򴰿�
		canvasX, canvasY, (int)physicalW, (int)physicalH,
		canvashWnd, nullptr, hInst, nullptr);
	if (!hWndScaleWindow)
	{
		return;
	}
	// ������������������HRGN�����ֱ��Ӧ�����������
	HRGN hRgnx = CreateRectRgn(0, 0, physicalW, RULER_SIZE); // ˮƽ��������
	HRGN hRgny = CreateRectRgn(0, 0, RULER_SIZE, physicalH); // ��ֱ��������
	// �����ϲ��������
	HRGN hRgnCombined = CreateRectRgn(0, 0, 0, 0);
	CombineRgn(hRgnCombined, hRgnx, hRgny, RGN_OR); // �ϲ��������������
	// ���úϲ�������򵽴��ڣ��������ֻ������Щ��������Ӧ����¼�
	SetWindowRgn(hWndScaleWindow, hRgnCombined, TRUE);
	DeleteObject(hRgnx);
	DeleteObject(hRgny);
	// ���ô��ڵ�͸����
	SetLayeredWindowAttributes(hWndScaleWindow, 0, 128, LWA_ALPHA);
	// ��ʾ����
	ShowWindow(hWndScaleWindow, SW_SHOW);
	UpdateWindow(hWndScaleWindow);
	return;// (int)msg.wParam;
}