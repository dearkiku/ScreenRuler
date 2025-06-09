#pragma once
#include <Windows.h>
// ����͸��ɫ
#define LAY_RGB		RGB(0, 128, 0)
// ������ɫ
#define LINE_RGB	RGB(67, 206, 245)// 239  / 37
#define LINE_RGB_CLICK	RGB(6, 82, 121)// 239  / 37
// �̶ȳ߳ߴ�
#define RULER_SIZE			20
// ��������
#define RULE_THRESH			4
// ���Ա�ߣ�����/��ʼ��ק����
#define WM_RULER_DRAG_START (WM_USER + 200)
// ���Ա�ߣ���ק�У�����λ��
#define WM_RULER_DRAG_MOVE  (WM_USER + 201)
// ���Ա�ߣ�������ק��wParam=0 ��ʾ���߽��������������ʾ existing index
#define WM_RULER_DRAG_END   (WM_USER + 202)
// �ߴ�ṹ
struct COORDINATE {
	int w;  // ���
	int h;	// �߶�
	int x;  // ���
	int y;  // ����
};

// ���ӻ�������λ�úͳߴ�
inline struct COORDINATE mainRect;
// �������ھ��
inline HWND canvashWnd;
// ��ߴ��ھ��
inline HWND hWndScaleWindow;
// ��ǰʵ��
inline HINSTANCE hInst;
// ������Ϣ
#define WM_TRAYICON (WM_USER + 300)
// ���̲˵�
inline static HMENU hTrayMenu = nullptr;
/**
 * @brief �����Ƿ��ھ����ڣ������߽磩
 * @param point Ҫ���ĵ�
 * @param rect Ŀ�����
 * @return true ������ھ����ڻ�߽��ϣ����򷵻� false
 */
bool ptInRect(const POINT& point, const RECT& rect);