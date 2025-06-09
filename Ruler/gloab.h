#pragma once
#include <Windows.h>
// 背景透明色
#define LAY_RGB		RGB(0, 128, 0)
// 标线颜色
#define LINE_RGB	RGB(67, 206, 245)// 239  / 37
#define LINE_RGB_CLICK	RGB(6, 82, 121)// 239  / 37
// 刻度尺尺寸
#define RULER_SIZE			20
// 冗余像素
#define RULE_THRESH			4
// 来自标尺：创建/开始拖拽标线
#define WM_RULER_DRAG_START (WM_USER + 200)
// 来自标尺：拖拽中，更新位置
#define WM_RULER_DRAG_MOVE  (WM_USER + 201)
// 来自标尺：结束拖拽，wParam=0 表示新线结束创建，否则表示 existing index
#define WM_RULER_DRAG_END   (WM_USER + 202)
// 尺寸结构
struct COORDINATE {
	int w;  // 宽度
	int h;	// 高度
	int x;  // 左边
	int y;  // 顶边
};

// 尺子画布窗口位置和尺寸
inline struct COORDINATE mainRect;
// 画布窗口句柄
inline HWND canvashWnd;
// 标尺窗口句柄
inline HWND hWndScaleWindow;
// 当前实例
inline HINSTANCE hInst;
// 托盘消息
#define WM_TRAYICON (WM_USER + 300)
// 托盘菜单
inline static HMENU hTrayMenu = nullptr;
/**
 * @brief 检查点是否在矩形内（包含边界）
 * @param point 要检查的点
 * @param rect 目标矩形
 * @return true 如果点在矩形内或边界上，否则返回 false
 */
bool ptInRect(const POINT& point, const RECT& rect);