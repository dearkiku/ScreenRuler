// Ruler.cpp : 定义应用程序的入口点。
//
#include "framework.h"
#include "resource.h"
// ===================
#include "gloab.h"
#include "Ruler.h"
#include "Scale.h"
#include "uiaccess.h"
// ===================
#include <tchar.h>
#include <vector>
#include <string>
#include <shellapi.h>
#include <combaseapi.h>
//#include <CommCtrl.h>
#define MAX_LOADSTRING 100
// 互斥体句柄
static HANDLE hMutex;
// 互斥体类名
constexpr auto APP_MUTEX_NAME = L"_COM_APP_RULER_MUTEX_LPNAME";
// 鼠标钩子句柄
HHOOK g_mouseHook = nullptr;
// 是否启用CTRL拖拽
static bool g_ctrlDrag = true;
// 禁止移动标线
static bool g_disableMove = false;
// 是否可见
static bool g_LineHide = false;
static bool g_ScaleHide = false;
// 鼠标穿透状态
static bool g_bMouseTransparentLine = false;
static bool g_bMouseTransparentScale = false;
// 截图透明状态
static bool g_bScreenshotTransparentLine = false;
static bool g_bScreenshotTransparentScale = false;
// 全部标线容器
static std::vector<LineInfo> g_lines;
// 显示器容器
std::vector<MonitorInfo> g_monitors;
// 正在拖拽的线索引
static int g_dragIndex = -1;
// 当前悬停索引
static int g_hoverIndex = -1;
// 是否处于Ctrl拖动模式
static bool g_bCtrlDragging = false;
// 当前通过 Ctrl 拖动的线条索引
static int g_ctrlDragIndex = -1;
// 托盘图标
static NOTIFYICONDATA g_nid = { 0 };
// 全局变量:
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名
RECT ruleSizeRc = {};
// 此代码模块中包含的函数的前向声明:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: 在此处放置代码。
	DWORD dwErr;
	// ERROR_NO_SITENAME
	dbgstart();
	// 初始化单元线程对象并发的线程 | 禁用 DDE 以支持 OLE1
	HRESULT hres = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	if (hres == S_OK)
	{
		dbg("COM 库在此线程上已成功初始化\n");
	}
	else if (hres == S_FALSE)
	{
		dbg("COM 库已在此线程上初始化\n");

	}
	else if (hres == RPC_E_CHANGED_MODE)
	{
		dbg("先前对 CoInitializeEx 的调用 为此线程指定了不兼容的并发模型。 这也可能表明发生了从中线单元到单线程单元的更改\n");

	}
	dwErr = PrepareForUIAccess();
	if (dwErr != ERROR_SUCCESS)
	{
		dbg("UIAccess error: 0x%08X\n", dwErr);
	}
	//else
	//{
	//	dbg("UIAccess success\n");
	//}
	dbg("GetCommandLine: %S\n", GetCommandLine());


	hMutex = CreateMutex(NULL, TRUE, APP_MUTEX_NAME);
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		MessageBox(0, L"应用程序已经在运行中，无需重复启动！", L"提示", MB_OK | MB_ICONWARNING);
		return 1;
	}
	// 初始化全局字符串
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_RULER, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// 执行应用程序初始化:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_RULER));

	MSG msg;

	// 主消息循环:
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	dbgend();
	return (int)msg.wParam;
}

//
//  函数: MyRegisterClass()
//
//  目标: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex{};

	wcex.cbSize = sizeof(WNDCLASSEX);

	// wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.style = CS_OWNDC;// 为 类中的每个窗口分配唯一的设备上下文
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_RULER));
	// wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	//wcex.hCursor = LoadCursor(nullptr, IDC_CROSS);
	wcex.hCursor = LoadCursor(hInstance, MAKEINTRESOURCE(IDC_CURSOR_CROSS));
	// wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.hbrBackground = CreateSolidBrush(LAY_RGB);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_RULER);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

// 回调函数：枚举显示器
BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData) {
	MONITORINFOEX info{};
	info.cbSize = sizeof(MONITORINFOEX);
	GetMonitorInfo(hMonitor, &info);

	MonitorInfo mi{};
	mi.hMonitor = hMonitor;
	mi.rcMonitor = info.rcMonitor;
	mi.rcWork = info.rcWork;
	g_monitors.push_back(mi);

	return TRUE; // 继续枚举
}

// 将窗口移动到鼠标所在的屏幕
void MoveWindowToMouseScreen(HWND hWnd) {
	POINT pt;
	GetCursorPos(&pt); // 获取鼠标位置

	for (const auto& monitor : g_monitors) {
		if (PtInRect(&monitor.rcMonitor, pt)) {
			// MoveWindowToScreen(hWnd, &monitor.rcMonitor);
			break;
		}
	}
}

// 将窗口移动到第 nScreenIndex 个屏幕（从0开始）
void MoveWindowToScreen(HWND hWnd, int nScreenIndex) {
	if (nScreenIndex < 0 || nScreenIndex >= (int)g_monitors.size()) {
		return; // 无效索引
	}

	RECT rc = g_monitors[nScreenIndex].rcWork;
	int width = rc.right - rc.left;
	int height = rc.bottom - rc.top;

	// 居中窗口（或自定义位置）
	SetWindowPos(
		hWnd, NULL,
		rc.left + (width - mainRect.w) / 2,  // X 坐标
		rc.top + (height - mainRect.h) / 2,  // Y 坐标
		mainRect.w, mainRect.h,              // 宽度和高度
		SWP_NOZORDER | SWP_NOACTIVATE
	);
}
// 初始化时调用
void InitMonitors() {
	EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, 0);
}
// 获取屏幕物理大小
void showScreen()
{
	int nScreenWidth, nScreenHeight;
	HDC hdcScreen = GetDC(NULL);   //获取屏幕的HDC
	nScreenWidth = GetDeviceCaps(hdcScreen, HORZSIZE);
	nScreenHeight = GetDeviceCaps(hdcScreen, VERTSIZE);


	const double MILLIMETRE_TO_INCH = 0.03937;
	double fDiagonalLen = sqrt(nScreenHeight * nScreenHeight + nScreenWidth * nScreenWidth);
	std::wstring tips = L"屏幕物理宽度：" + std::to_wstring(nScreenWidth) + L"\n屏幕物理高度：" + std::to_wstring(nScreenHeight)
		+ L"\n屏幕对角线长为：" + std::to_wstring(fDiagonalLen) + L"毫米,英寸约为：" + std::to_wstring(fDiagonalLen * MILLIMETRE_TO_INCH);
	MessageBox(canvashWnd, tips.c_str(), L"屏幕尺寸", MB_OK);

	// 由于GetDeviceCaps函数的限制，在Win7系统下该程序检测结果不准确，WinXP系统下基本上可以正确运行
}

/**
 * @brief 初始化托盘&图标&菜单
 */
static void InitTray()
{
	// 初始化托盘图标
	g_nid.cbSize = sizeof(NOTIFYICONDATA);
	g_nid.hWnd = canvashWnd;
	g_nid.uID = 1;
	g_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	g_nid.uCallbackMessage = WM_TRAYICON;
	// LoadIconMetric(hInst, MAKEINTRESOURCE(IDI_SMALL), LIM_SMALL, &g_nid.hIcon);
	g_nid.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_SMALL));
	wcscpy_s(g_nid.szTip, L"合纵连横");
	Shell_NotifyIcon(NIM_ADD, &g_nid);

	// 创建托盘菜单
	hTrayMenu = CreatePopupMenu();
	// AppendMenu(hTrayMenu, MF_STRING | (g_bMouseTransparentLine ? MF_CHECKED : 0), IDM_MOUSE_TRANSPARENT_LINE, L"鼠标穿透标线");
	// GetWindowBand();

	AppendMenu(hTrayMenu, MF_STRING, IDM_CLEAN_LINE, L"清除所有标线");
	AppendMenu(hTrayMenu, MF_SEPARATOR, 0, NULL);  // 分隔线
	AppendMenu(hTrayMenu, MF_STRING, IDM_LOCK_LINE, L"禁止鼠标拖拽");
	AppendMenu(hTrayMenu, MF_STRING, IDM_CTRL_MOVE_LINE, L"禁止CTRL拖拽");
	AppendMenu(hTrayMenu, MF_SEPARATOR, 0, NULL);  // 分隔线
	AppendMenu(hTrayMenu, MF_STRING, IDM_HIDE_LINE, L"隐藏标线");
	AppendMenu(hTrayMenu, MF_STRING, IDM_MOUSE_TRANSPARENT_LINE, L"穿透标线");
	AppendMenu(hTrayMenu, MF_STRING, IDM_SCREENSHOT_TRANSPARENT_LINE, L"截图隐藏标线");
	AppendMenu(hTrayMenu, MF_SEPARATOR, 0, NULL);  // 分隔线
	AppendMenu(hTrayMenu, MF_STRING, IDM_HIDE_SCALE, L"隐藏标尺");
	AppendMenu(hTrayMenu, MF_STRING, IDM_SCREENSHOT_TRANSPARENT_SCALE, L"截图隐藏标尺");
	AppendMenu(hTrayMenu, MF_STRING, IDM_MOUSE_TRANSPARENT_SCALE, L"穿透标尺");
	AppendMenu(hTrayMenu, MF_SEPARATOR, 0, NULL);  // 分隔线
	AppendMenu(hTrayMenu, MF_STRING, IDM_EXIT, L"退出");
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目标: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // 将实例句柄存储在全局变量中
	// 获取屏幕分辨率
	//RECT screenRect = { 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN) };
	//AdjustWindowRect(&screenRect, WS_POPUP, FALSE);
	mainRect.x = 0;
	mainRect.y = 0;
	mainRect.w = GetSystemMetrics(SM_CXSCREEN);
	mainRect.h = GetSystemMetrics(SM_CYSCREEN);
	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE);

	HWND hWnd = CreateWindowEx(
		// WS_EX_NOREDIRECTIONBITMAP // 始终置顶、不显示在ALT+TAB中 | 支持透明 | WS_EX_TRANSPARENT鼠标穿透
		WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_LAYERED | WS_EX_COMPOSITED /*| WS_EX_TRANSPARENT*/,
		szWindowClass, szTitle,
		//WS_OVERLAPPEDWINDOW 窗口是重叠的窗口
		// 无边框窗口 | 将所有其他重叠的子窗口剪裁出要更新的子窗口的区域
		// WS_EX_TRANSPARENT 在父窗口内进行绘图时，不包括子窗口所占用的区域
		WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		//CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,
		// 暂时全屏显示
		mainRect.x, mainRect.y,
		mainRect.w, mainRect.h,
		nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}
	SetLayeredWindowAttributes(hWnd, LAY_RGB, 255, LWA_COLORKEY);
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	CoUninitialize();

	return TRUE;
}


/**
 * @brief 标线是否被击中
 * @param L 标线信息
 * @param pt 鼠标坐标
 * @return 如果命中则返回 true，否则返回 false
 * @note ±RULE_THRESH 像素内算命中
 */
static bool IsHit(const LineInfo& L, const POINT& pt) {
	return  abs((L.horizontal ? pt.y : pt.x) - L.pos) <= RULE_THRESH;
	// 透明背景无法接受鼠标消息
	if (L.horizontal)
	{
		return abs(pt.y - L.pos) <= RULE_THRESH;
	}
	else
	{
		return abs(pt.x - L.pos) <= RULE_THRESH;
	}
}


/**
 * @brief 获取命中标线的索引
 * @param pt 坐标
 * @return 命中标线的索引，如果没有命中则返回 -1
 */
static int GetHitLineIndex(const POINT& pt)
{
	// 遍历标线列表
	for (int i = 0; i < (int)g_lines.size(); ++i)
	{
		if (IsHit(g_lines[i], pt))
		{
			return i;
		}
	}
	return -1;
}

/**
 * @brief 鼠标钩子回调函数
 * @param nCode 要检查的点
 * @param wParam 鼠标消息的标识符。此参数可以是以下消息之一： WM_LBUTTONDOWN、 WM_LBUTTONUP、 WM_MOUSEMOVE、 WM_MOUSEWHEEL、 WM_RBUTTONDOWN 或 WM_RBUTTONUP。
 * @param lParam 指向 MSLLHOOKSTRUCT 结构的指针
 * @return 如果 nCode 小于零，则挂钩过程必须返回 CallNextHookEx 返回的值
 */
static LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{

	if (!g_ctrlDrag)
	{
		return CallNextHookEx(g_mouseHook, nCode, wParam, lParam);
	}

	if (nCode == HC_ACTION && wParam == WM_MOUSEMOVE)
	{
		PMSLLHOOKSTRUCT p = (PMSLLHOOKSTRUCT)lParam;
		POINT pt = p->pt;

		// 将屏幕坐标转换为主窗口客户坐标
		ScreenToClient(canvashWnd, &pt);

		// 检测 Ctrl 是否按下
		if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
		{
			if (g_ctrlDragIndex < 0)
			{
				int hit = GetHitLineIndex(pt);
				if (hit != -1)
				{
					g_ctrlDragIndex = hit;
					SetCapture(canvashWnd);
				}
			}

			if (g_ctrlDragIndex >= 0 && g_ctrlDragIndex < (int)g_lines.size())
			{
				LineInfo& L = g_lines[g_ctrlDragIndex];
				L.pos = L.horizontal ? pt.y : pt.x;
				InvalidateRect(canvashWnd, nullptr, FALSE);
			}
		}
		else
		{
			if (g_ctrlDragIndex >= 0)
			{
				g_ctrlDragIndex = -1;
				ReleaseCapture();
			}
		}
	}
	return CallNextHookEx(g_mouseHook, nCode, wParam, lParam);
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目标: 处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
	{
		// 保存画布窗口句柄
		canvashWnd = hWnd;
		// 创建托盘图标和菜单
		InitTray();
		// showScreen();
		ruleSizeRc.left = mainRect.x;
		ruleSizeRc.right = mainRect.x + RULER_SIZE;
		ruleSizeRc.top = mainRect.y;
		ruleSizeRc.bottom = mainRect.y + RULER_SIZE;
		// 创建标尺窗口
		CreateScaleWindow(mainRect.x, mainRect.y, mainRect.w, mainRect.h);
		g_mouseHook = SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseProc, nullptr, 0);

	}
	break;
	case WM_TRAYICON:
	{
		//switch (lParam)
		//{
		//case WM_RBUTTONUP:
		//{
		//	POINT pt;
		//	GetCursorPos(&pt);
		//	// 将创建指定窗口的线程引入前台并激活窗口，确保菜单能正确关闭
		//	SetForegroundWindow(hWnd);
		//	// 弹出托盘菜单
		//	TrackPopupMenu(hTrayMenu, TPM_BOTTOMALIGN | TPM_LEFTBUTTON, pt.x, pt.y, 0, hWnd, nullptr);
		//}
		//break;
		//default:
		//	break;
		//}
		if (lParam == WM_RBUTTONUP) {
			POINT pt;
			GetCursorPos(&pt);
			// 将创建指定窗口的线程引入前台并激活窗口，确保菜单能正确关闭
			SetForegroundWindow(hWnd);
			// 弹出托盘菜单
			TrackPopupMenu(hTrayMenu, TPM_BOTTOMALIGN | TPM_LEFTBUTTON, pt.x, pt.y, 0, hWnd, nullptr);
		}
	}
	break;
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// 解析菜单选择:
		switch (wmId)
		{
		case IDM_MOUSE_TRANSPARENT_LINE:
		{
			g_bMouseTransparentLine = !g_bMouseTransparentLine;
			LONG_PTR style = GetWindowLongPtr(hWnd, GWL_EXSTYLE);
			if (g_bMouseTransparentLine) {
				style |= WS_EX_TRANSPARENT;
				CheckMenuItem(hTrayMenu, IDM_MOUSE_TRANSPARENT_LINE, MF_BYCOMMAND | MF_CHECKED);
			}
			else {
				style &= ~WS_EX_TRANSPARENT;
				CheckMenuItem(hTrayMenu, IDM_MOUSE_TRANSPARENT_LINE, MF_BYCOMMAND | MF_UNCHECKED);
			}
			SetWindowLongPtr(hWnd, GWL_EXSTYLE, style);
		}
		break;
		case IDM_MOUSE_TRANSPARENT_SCALE:
		{
			g_bMouseTransparentScale = !g_bMouseTransparentScale;
			LONG_PTR style = GetWindowLongPtr(hWndScaleWindow, GWL_EXSTYLE);
			if (g_bMouseTransparentScale) {
				style |= WS_EX_TRANSPARENT;
				CheckMenuItem(hTrayMenu, IDM_MOUSE_TRANSPARENT_SCALE, MF_BYCOMMAND | MF_CHECKED);
			}
			else {
				style &= ~WS_EX_TRANSPARENT;
				CheckMenuItem(hTrayMenu, IDM_MOUSE_TRANSPARENT_SCALE, MF_BYCOMMAND | MF_UNCHECKED);
			}
			SetWindowLongPtr(hWndScaleWindow, GWL_EXSTYLE, style);
		}
		break;
		case IDM_SCREENSHOT_TRANSPARENT_LINE:
		{
			g_bScreenshotTransparentLine = !g_bScreenshotTransparentLine;
			if (g_bScreenshotTransparentLine)
			{
				SetWindowDisplayAffinity(hWnd, WDA_EXCLUDEFROMCAPTURE);
			}
			else
			{
				SetWindowDisplayAffinity(hWnd, WDA_NONE);
			}
			CheckMenuItem(hTrayMenu, IDM_SCREENSHOT_TRANSPARENT_LINE, MF_BYCOMMAND | (g_bScreenshotTransparentLine ? MF_CHECKED : MF_UNCHECKED));
		}
		break;
		case IDM_SCREENSHOT_TRANSPARENT_SCALE:
		{
			g_bScreenshotTransparentScale = !g_bScreenshotTransparentScale;
			if (g_bScreenshotTransparentScale)
			{
				SetWindowDisplayAffinity(hWndScaleWindow, WDA_EXCLUDEFROMCAPTURE);
				CheckMenuItem(hTrayMenu, IDM_SCREENSHOT_TRANSPARENT_SCALE, MF_BYCOMMAND | MF_CHECKED);
			}
			else
			{
				SetWindowDisplayAffinity(hWndScaleWindow, WDA_NONE);
				CheckMenuItem(hTrayMenu, IDM_SCREENSHOT_TRANSPARENT_SCALE, MF_BYCOMMAND | MF_UNCHECKED);
			}
		}
		break;
		case IDM_HIDE_SCALE:
		{
			g_ScaleHide = !g_ScaleHide;
			ShowWindow(hWndScaleWindow, g_ScaleHide ? SW_HIDE : SW_SHOW);
			CheckMenuItem(hTrayMenu, IDM_HIDE_SCALE, MF_BYCOMMAND | (g_ScaleHide ? MF_CHECKED : MF_UNCHECKED));
		}
		break;
		case IDM_HIDE_LINE:
		{

			g_LineHide = !g_LineHide;
			ShowWindow(hWnd, g_LineHide ? SW_HIDE : SW_SHOW);
			CheckMenuItem(hTrayMenu, IDM_HIDE_LINE, MF_BYCOMMAND | (g_LineHide ? MF_CHECKED : MF_UNCHECKED));
		}
		break;
		case IDM_LOCK_LINE:
		{
			g_disableMove = !g_disableMove;
			CheckMenuItem(hTrayMenu, IDM_LOCK_LINE, MF_BYCOMMAND | (g_disableMove ? MF_CHECKED : MF_UNCHECKED));
		}
		break;
		case IDM_CTRL_MOVE_LINE:
		{
			CheckMenuItem(hTrayMenu, IDM_CTRL_MOVE_LINE, MF_BYCOMMAND | (g_ctrlDrag ? MF_CHECKED : MF_UNCHECKED));
			g_ctrlDrag = !g_ctrlDrag;
		}
		break;
		case IDM_CLEAN_LINE:
		{
			g_lines.clear();
			InvalidateRect(hWnd, nullptr, FALSE);
		}
		break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		// 创建内存设备上下文（双缓冲）
		HDC hdcMem = CreateCompatibleDC(hdc);
		RECT rc;
		GetClientRect(hWnd, &rc);
		HBITMAP hBitmap = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
		HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMem, hBitmap);
		// 填充背景色
		HBRUSH bgBrush = CreateSolidBrush(LAY_RGB);
		FillRect(hdcMem, &rc, bgBrush);
		DeleteObject(bgBrush);

		// 绘制所有标线
		for (auto& L : g_lines) {
			HPEN pen = CreatePen(PS_SOLID, 1, L.color);
			HGDIOBJ oldPen = SelectObject(hdcMem, pen);
			if (L.horizontal) {
				MoveToEx(hdcMem, 0, L.pos, nullptr);
				LineTo(hdcMem, rc.right, L.pos);
			}
			else {
				MoveToEx(hdcMem, L.pos, 0, nullptr);
				LineTo(hdcMem, L.pos, rc.bottom);
			}
			SelectObject(hdcMem, oldPen);
			DeleteObject(pen);
		}

		// 将内存DC内容复制到屏幕
		BitBlt(hdc, 0, 0, rc.right, rc.bottom, hdcMem, 0, 0, SRCCOPY);

		// 清理资源
		SelectObject(hdcMem, hOldBitmap);
		DeleteObject(hBitmap);
		DeleteDC(hdcMem);
		EndPaint(hWnd, &ps);
		return 0;
	}
	break;
	case WM_RULER_DRAG_START:
	{
		LineInfo li = {
			(bool)wParam,	// 横/竖
			(int)lParam,	// 位置
			LINE_RGB,		// 颜色
			true			// 拖拽中
		};
		g_lines.push_back(li);
		g_dragIndex = (int)g_lines.size() - 1;
		InvalidateRect(hWnd, nullptr, FALSE);
		return 0;
	}
	break;
	case WM_RULER_DRAG_MOVE:
	{
		// 正在移动线
		// 当前拖拽的线索引不为0 & 索引不超过线的最大值 & 当前线的拖拽为真
		if (g_dragIndex >= 0 && g_dragIndex < (int)g_lines.size() && g_lines[g_dragIndex].dragging) {
			// 设置当前标线的位置
			g_lines[g_dragIndex].pos = (int)lParam;
			// 重绘窗口
			InvalidateRect(hWnd, nullptr, FALSE);
		}
		return 0;
	}
	break;
	case WM_RULER_DRAG_END:
	{
		// 创建后-拖动结束
		if (g_dragIndex >= 0 && g_dragIndex < (int)g_lines.size())
		{
			LineInfo& L = g_lines[g_dragIndex];
			bool shouldErase = false;
			// 判断标线是否进入标尺区域
			if (L.horizontal)
			{
				shouldErase = (L.pos < mainRect.y + RULER_SIZE + 1);
			}
			else
			{
				shouldErase = (L.pos < mainRect.x + RULER_SIZE + 1);
			}

			if (shouldErase)
			{
				g_lines.erase(g_lines.begin() + g_dragIndex);
			}
			else
			{
				// 正在拖动为假
				g_lines[g_dragIndex].dragging = false;
			}
			// 清除索引
			g_dragIndex = -1;
			InvalidateRect(hWnd, nullptr, FALSE);
			return 0;

			//// 判断标线是否在标尺中
			//if (g_lines[g_dragIndex].horizontal)
			//{
			//	//dbg("水平标尺\n");
			//	// 是否在水平标尺中
			//	if (g_lines[g_dragIndex].pos < mainRect.y + RULER_SIZE + 1)
			//	{
			//		//dbg("位置%d , %d \n", g_lines[g_dragIndex].pos, mainRect.y + RULER_SIZE + 1);

			//		g_lines.erase(g_lines.begin() + g_dragIndex);
			//		g_dragIndex = -1;
			//		InvalidateRect(hWnd, nullptr, FALSE);
			//		return 0;
			//	}
			//}
			//else
			//{
			//	//dbg("垂直标尺\n");
			//	// 是否在垂直标尺中
			//	if (g_lines[g_dragIndex].pos < mainRect.x + RULER_SIZE + 1)
			//	{
			//		//dbg("位置%d , %d \n", g_lines[g_dragIndex].pos, mainRect.y + RULER_SIZE + 1);
			//		g_lines.erase(g_lines.begin() + g_dragIndex);
			//		g_dragIndex = -1;
			//		InvalidateRect(hWnd, nullptr, FALSE);
			//		return 0;
			//	}
			//}




			// 判断鼠标位置是否在ruleSizeRc中
			//POINT pt;
			//GetCursorPos(&pt);			
			//if (ptInRect(pt, ruleSizeRc))
			//{
			//	g_lines.erase(g_lines.begin() + g_dragIndex);
			//	g_dragIndex = -1;
			//	return 0;
			//}



		}
		return 0;
	}
	break;
	case WM_LBUTTONDOWN:
	{
		if (g_disableMove)
		{
			return 0;
		}
		// 终止可能的残留CTRL拖拽状态
		if (g_ctrlDragIndex >= 0) {
			g_ctrlDragIndex = -1;
			ReleaseCapture();
		}
		// 如果收到鼠标左键点击消息 那么 因为背景透明不接收鼠标消息，所以鼠标必定处于某条标线上方
		POINT pt = { LOWORD(lParam), HIWORD(lParam) };
		ScreenToClient(hWnd, &pt);
		// 捕获鼠标
		SetCapture(hWnd);
		// 获取鼠标命中的标线索引
		int selectLine = GetHitLineIndex(pt);
		// dbg("收到点击消息 %d -- %d  \n", selectLine, g_ctrlDragIndex);
		if (selectLine != -1)
		{
			g_dragIndex = selectLine;
			g_lines[selectLine].dragging = true;
			g_lines[selectLine].color = LINE_RGB_CLICK;
			// InvalidateRect(hWnd, nullptr, FALSE);
		}
		return 0;
	}
	break;
	case WM_MOUSEMOVE:
	{
		if (g_disableMove)
		{
			return 0;
		}
		POINT pt = { LOWORD(lParam), HIWORD(lParam) };
		ScreenToClient(hWnd, &pt);
		// 鼠标左键拖拽线条
		// dbg("收到拖拽消息 %d \n", g_dragIndex);
		if (g_dragIndex >= 0 && g_dragIndex < (int)g_lines.size() && g_lines[g_dragIndex].dragging)
		{
			LineInfo& L = g_lines[g_dragIndex];
			L.pos = L.horizontal ? pt.y : pt.x;
			// dbg("设置标线位置 %d \n", L.pos);
			InvalidateRect(hWnd, nullptr, FALSE);
			//dbg("鼠标位置：%d,%d,标线位置L:%d\n", pt.y, pt.x, L.pos);
			return 0;
		}
		return 0;
	}
	break;
	case WM_LBUTTONUP:
	{
		if (g_dragIndex >= 0 && g_dragIndex < (int)g_lines.size()) {
			LineInfo& L = g_lines[g_dragIndex];
			L.color = LINE_RGB;

			//// 是否在角落矩形
			//POINT pt = { LOWORD(lParam), HIWORD(lParam) };
			//ScreenToClient(hWnd, &pt);
			////POINT ptLine = { L.horizontal ? 0 : L.pos, L.horizontal ? L.pos : 0 };
			//ClientToScreen(hWnd, &pt);
			//if (ptInRect(pt, ruleSizeRc))
			//{
			//	g_lines.erase(g_lines.begin() + g_dragIndex);
			//	g_dragIndex = -1;
			//}

			// 判断标线是否在标尺中
			if (g_lines[g_dragIndex].horizontal)
			{
				if (L.pos < mainRect.y + RULER_SIZE + 1)
				{
					g_lines.erase(g_lines.begin() + g_dragIndex);
					g_dragIndex = -1;
				}
			}
			else
			{
				if (L.pos < mainRect.x + RULER_SIZE + 1)
				{
					g_lines.erase(g_lines.begin() + g_dragIndex);
					g_dragIndex = -1;
				}
			}


			g_dragIndex = -1;
			InvalidateRect(hWnd, nullptr, TRUE);
		}
		ReleaseCapture();
		return 0;
	}
	break;
	break;
	case WM_ERASEBKGND:
	{
		// 直接返回，不执行默认背景擦除
		return 1;
	}
	break;
	case WM_DESTROY:
	{
		// 释放鼠标钩子
		if (g_mouseHook)
		{
			UnhookWindowsHookEx(g_mouseHook);
			g_mouseHook = nullptr;
		}
		g_lines.clear();
		// 移除托盘图标
		Shell_NotifyIcon(NIM_DELETE, &g_nid);
		// 移除托盘菜单
		DestroyMenu(hTrayMenu);
		// DeleteObject(hbrBackground);
		PostQuitMessage(0);
	}
	break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}