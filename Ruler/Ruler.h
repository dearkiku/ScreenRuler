#pragma once

// #include <unordered_map>
// �˵�����ID

// �˳�
#define IDM_EXIT							1001
// ��괩͸
#define IDM_MOUSE_TRANSPARENT_LINE			1002
#define IDM_MOUSE_TRANSPARENT_SCALE			1003
// ��ͼ����
#define IDM_SCREENSHOT_TRANSPARENT_LINE		1004
#define IDM_SCREENSHOT_TRANSPARENT_SCALE	1005
#define IDM_HIDE_SCALE						1006
#define IDM_HIDE_LINE						1007

#define IDM_CLEAN_LINE						1008
#define IDM_LOCK_LINE						1009
#define IDM_CTRL_MOVE_LINE					1010
// �������ݽṹ
struct LineInfo {
	bool      horizontal;   // ��/��
	int       pos;          // λ��
	COLORREF  color;        // ��ɫ
	bool      dragging;     // ��ק��
};

struct MonitorInfo {
    HMONITOR hMonitor;
    RECT     rcMonitor;  // ��ʾ�����������ų���������
    RECT     rcWork;     // ��ʾ����������
};