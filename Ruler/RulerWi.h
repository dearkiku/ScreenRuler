#pragma once
#include <windows.h>

class RulerWi {
public:
	RulerWi(int canvasWidth, int canvasHeight,
		double physicalWidth, double physicalHeight,
		double unitInterval = 5.0); // 默认间隔单位为5
	~RulerWi();
	void CreateScaleWindow(HINSTANCE hInstance, HWND parent);

private:
	int canvasWidth;               // 标尺窗口的宽度
	int canvasHeight;              // 标尺窗口的高度
	double physicalWidth;          // 物理宽度
	double physicalHeight;         // 物理高度
	double unitInterval;           // 单位间隔
	HWND hWndScaleWindow;		   // 标尺窗口句柄
	COLORREF rulerColor;           // 标尺颜色
	COLORREF lineColor;            // 刻度线颜色
	double horizontalUnitPerPixel; // 横向每像素对应的单位
	double verticalUnitPerPixel;   // 纵向每像素对应的单位

	void RegisterRulerClass(HINSTANCE hInstance);
	double CalculateUnitPerPixel(int screenSize, double physicalSize);
	// 绘制标尺
	void DrawRuler(HWND hWnd, HDC hdc, bool isHorizontal);

	static LRESULT CALLBACK RulerWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};
