#ifndef LINE_H
#define LINE_H

#include <windows.h>
#include <vector>
#define LINE_WIDTH      1
class Line
{
public:
	Line(int location, int length, bool isVertical);
	~Line();

	void CreateLine();

	HWND GetWnd() const { return hwndLine; }
	void MoveLine(int location);

	static void UpdateLines(HWND hwndParent);

private:
	HWND hwndLine;      // 线段控件句柄
	int location;       // 线段的位置
	int length;         // 线长
	bool isVertical;    // 是否是竖线
	COLORREF lineColor; // 线段颜色
	static std::vector<HWND> lines;  // 存储所有线段控件句柄
};

// 子类化窗口过程
LRESULT CALLBACK LineWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

#endif
