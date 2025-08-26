#include <windows.h>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <tlhelp32.h>
#include <tchar.h>

//#define DEBUG
#ifdef DEBUG
#include <iostream>
#endif
using namespace std;

#define LB WM_LBUTTONDOWN
#define RB WM_RBUTTONDOWN
#define MB WM_MBUTTONDOWN
#define MV WM_MOUSEMOVE
#define WH WM_MOUSEWHEEL

HHOOK keyboardHook, mouseHook;

extern const string unlockKeyboardKey;
extern const vector<WPARAM> unlockMouseKey;

// KMP状态机结构
struct KMPState {
    vector<int> prefixTable;
    int currentState;

    KMPState(const vector<WPARAM>& pattern) {
        computePrefix(pattern);
        currentState = 0;
    }

    KMPState(const string& pattern) {
        vector<WPARAM> intPattern(pattern.begin(), pattern.end());
        computePrefix(intPattern);
        currentState = 0;
    }

    void computePrefix(const vector<WPARAM>& pattern) {
        prefixTable.resize(pattern.size());
        int j = 0;
        for (int i = 1; i < pattern.size(); i++) {
            while (j > 0 && pattern[i] != pattern[j])
                j = prefixTable[j-1];
            if (pattern[i] == pattern[j]) j++;
            prefixTable[i] = j;
        }
    }

    bool processInput(WPARAM input, const vector<WPARAM>& pattern) {
        while (currentState > 0 && input != pattern[currentState])
            currentState = prefixTable[currentState-1];

        if (input == pattern[currentState])
            currentState++;

        if (currentState == pattern.size()) {
            currentState = 0;
            return true;
        }
        return false;
    }

    bool processInput(char input, const string& pattern) {
        while (currentState > 0 && tolower(input) != tolower(pattern[currentState]))
            currentState = prefixTable[currentState-1];

        if (tolower(input) == tolower(pattern[currentState]))
            currentState++;

        if (currentState == pattern.size()) {
            currentState = 0;
            return true;
        }
        return false;
    }
};

KMPState keyboardKMP(unlockKeyboardKey);
KMPState mouseKMP(unlockMouseKey);

bool lockKeyboard = true, lockMouse = true;
bool preLockKeyboard = true, preLockMouse = true;
WPARAM lastMouseAction = 0;

HWND hwnd;

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    lockKeyboard = preLockKeyboard;

    if (nCode == HC_ACTION && wParam == WM_KEYDOWN && !unlockKeyboardKey.empty()) {
        KBDLLHOOKSTRUCT* pKey = (KBDLLHOOKSTRUCT*)lParam;
        char pressedKey = MapVirtualKey(pKey->vkCode, MAPVK_VK_TO_CHAR);

        if (keyboardKMP.processInput(pressedKey, unlockKeyboardKey)) {
            preLockKeyboard = !preLockKeyboard;
        }
    }

    if(lockKeyboard)
        return 1;  // 拦截所有键盘输入
    return CallNextHookEx(keyboardHook, nCode, wParam, lParam);
}

LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
    lockMouse = preLockMouse;

    if (nCode == HC_ACTION && !unlockMouseKey.empty()) {
        if (find(unlockMouseKey.begin(), unlockMouseKey.end(), wParam) != unlockMouseKey.end() &&
            !(lastMouseAction == wParam && (wParam == MV || wParam == WH))) {

            lastMouseAction = wParam;

            if (mouseKMP.processInput(wParam, unlockMouseKey)) {
                preLockMouse = !preLockMouse;
            }

            #ifdef DEBUG
            cout << "Mouse state: " << mouseKMP.currentState << endl;
            #endif
        }
    }

    if(lockMouse)
        return 1;
    return CallNextHookEx(mouseHook, nCode, wParam, lParam);
}

bool IsProcessAlreadyRunning(const std::string& processName) {
    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    DWORD currentPid = GetCurrentProcessId();

    if (Process32First(hSnapshot, &pe)) {
        do {
            if (pe.th32ProcessID != currentPid &&
                _stricmp(pe.szExeFile, processName.c_str()) == 0) {
                CloseHandle(hSnapshot);
                return true;
            }
        } while (Process32Next(hSnapshot, &pe));
    }
    CloseHandle(hSnapshot);
    return false;
}

void checkStatus() {
    if(lockMouse&&lockKeyboard)
    	ShowWindow(hwnd,SW_MAXIMIZE);
    else
    	ShowWindow(hwnd,SW_MINIMIZE);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_PAINT: {
	        PAINTSTRUCT ps;
	        HDC hdc = BeginPaint(hwnd, &ps);
	        
	        // 获取客户区矩形
	        RECT rc;
	        GetClientRect(hwnd, &rc);
	        
	        // 创建字体
	        HFONT hFont = CreateFont(
	            -48, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
	            DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
	            CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
	            VARIABLE_PITCH, _T("Arial"));
	        HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
	        
	        // 设置文本属性
	        SetTextColor(hdc, RGB(255, 255, 255));
	        SetBkMode(hdc, TRANSPARENT);
	        
	        // 计算文本高度
	        LPCTSTR pszText = _T("已锁定\nYour computer has been locked");
	        RECT rcText = rc;
	        DrawText(hdc, pszText, -1, &rcText, DT_CALCRECT | DT_WORDBREAK);
	        
	        // 调整矩形位置实现居中
	        rc.top = (rc.bottom - (rcText.bottom - rcText.top)) / 2;
	        rc.bottom = rc.top + (rcText.bottom - rcText.top);
	        
	        // 绘制文本
	        DrawText(hdc, pszText, -1, &rc, DT_WORDBREAK | DT_CENTER);
	        
	        // 恢复资源
	        SelectObject(hdc, hOldFont);
	        DeleteObject(hFont);
	        EndPaint(hwnd, &ps);
	        break;
        }

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        case WM_TIMER:
        	checkStatus();
        	break;

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

const string info = "Computer Locker v1.1\n"
                    "Make by @lkjlkjlkj2012.\n"
                    "\n"
                    "Use it to lock your computer to avoid unauthorized use.\n"
                    "Change key.cpp to set your own key.\n"
                    "Use \"locker\" to confirm then lock your computer.\n"
                    "Use \"locker don't lock\" to start without confirm and not lock your computer.\n"
                    "Use \"locker skip confirm\" to skip confirm and lock computethe r.\n"
                    "Other parameters will pop this info.\n"
                    "\n"
                    "Warning: Never start two or more lockers, it may cause your computer\n"
                    "         lock forever, and you need to close your computer!\n"
                    "\n";

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    string curExe = "locker.exe";
    if (!strcmp(lpCmdLine, "skip comfirm")) {
        if (IsProcessAlreadyRunning(curExe)) return 0;
    } else if (!strcmp(lpCmdLine, "don't lock")) {
        if (IsProcessAlreadyRunning(curExe)) return 0;
        preLockKeyboard = preLockMouse = false;
    } else if (!strcmp(lpCmdLine, "")) {
        if (IsProcessAlreadyRunning(curExe)) {
            MessageBox(nullptr, "Process already running!", "Error",
                MB_OK|MB_ICONERROR);
            return 0;
        }
        int t = MessageBox(nullptr,
            "Warning: This program will lock your computer by blocking all input!\n"
            "Warning: Remember your unlock key! If you forgot it, you need to\n"
            "         close your computer!\n"
            "\n"
            "Sure to run it?\n",
            "Warning", MB_YESNO|MB_ICONWARNING|MB_DEFBUTTON2);
        if (t == IDNO) return 0;
    } else {
        MessageBox(nullptr, info.c_str(), "Info",
            MB_OK|MB_ICONINFORMATION|MB_DEFBUTTON1);
    }

    keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, NULL, 0);
    mouseHook = SetWindowsHookEx(WH_MOUSE_LL, MouseProc, NULL, 0);

    // 注册窗口类
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszClassName = _T("LockScreenClass");

    RegisterClass(&wc);

    // 获取屏幕尺寸
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    // 创建全屏窗口
    hwnd = CreateWindowEx(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        _T("LockScreenClass"),
        _T("锁定屏幕"),
        WS_POPUP,
        0, 0, screenWidth, screenHeight,
        NULL, NULL, hInstance, NULL);

    // 显示窗口
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    SetTimer(hwnd, 1, 100, NULL);
    
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWindowsHookEx(keyboardHook);
    UnhookWindowsHookEx(mouseHook);
    return (int)msg.wParam;
}
