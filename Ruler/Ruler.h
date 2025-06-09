#pragma once

// #include <unordered_map>
// 菜单命令ID

// 退出
#define IDM_EXIT							1001
// 鼠标穿透
#define IDM_MOUSE_TRANSPARENT_LINE			1002
#define IDM_MOUSE_TRANSPARENT_SCALE			1003
// 截图隐藏
#define IDM_SCREENSHOT_TRANSPARENT_LINE		1004
#define IDM_SCREENSHOT_TRANSPARENT_SCALE	1005
#define IDM_HIDE_SCALE						1006
#define IDM_HIDE_LINE						1007

#define IDM_CLEAN_LINE						1008
#define IDM_LOCK_LINE						1009
#define IDM_CTRL_MOVE_LINE					1010
// 标线数据结构
struct LineInfo {
	bool      horizontal;   // 横/竖
	int       pos;          // 位置
	COLORREF  color;        // 颜色
	bool      dragging;     // 拖拽中
};

struct MonitorInfo {
    HMONITOR hMonitor;
    RECT     rcMonitor;  // 显示器工作区（排除任务栏）
    RECT     rcWork;     // 显示器完整区域
};