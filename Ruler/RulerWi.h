#pragma once
#include <windows.h>

class RulerWi {
public:
	RulerWi(int canvasWidth, int canvasHeight,
		double physicalWidth, double physicalHeight,
		double unitInterval = 5.0); // Ĭ�ϼ����λΪ5
	~RulerWi();
	void CreateScaleWindow(HINSTANCE hInstance, HWND parent);

private:
	int canvasWidth;               // ��ߴ��ڵĿ��
	int canvasHeight;              // ��ߴ��ڵĸ߶�
	double physicalWidth;          // ������
	double physicalHeight;         // ����߶�
	double unitInterval;           // ��λ���
	HWND hWndScaleWindow;		   // ��ߴ��ھ��
	COLORREF rulerColor;           // �����ɫ
	COLORREF lineColor;            // �̶�����ɫ
	double horizontalUnitPerPixel; // ����ÿ���ض�Ӧ�ĵ�λ
	double verticalUnitPerPixel;   // ����ÿ���ض�Ӧ�ĵ�λ

	void RegisterRulerClass(HINSTANCE hInstance);
	double CalculateUnitPerPixel(int screenSize, double physicalSize);
	// ���Ʊ��
	void DrawRuler(HWND hWnd, HDC hdc, bool isHorizontal);

	static LRESULT CALLBACK RulerWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};
