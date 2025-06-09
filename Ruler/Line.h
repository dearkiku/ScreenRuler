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
	HWND hwndLine;      // �߶οؼ����
	int location;       // �߶ε�λ��
	int length;         // �߳�
	bool isVertical;    // �Ƿ�������
	COLORREF lineColor; // �߶���ɫ
	static std::vector<HWND> lines;  // �洢�����߶οؼ����
};

// ���໯���ڹ���
LRESULT CALLBACK LineWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

#endif
