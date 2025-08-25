#include <windows.h>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <tlhelp32.h>

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

const string info = "Computer Locker v1.0\n"
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

int main(int argc, char *argv[]) {
    string curExe = "locker.exe";
    if (argc == 3 && (string)argv[1] == "skip" && (string)argv[2] == "confirm") {
        if (IsProcessAlreadyRunning(curExe)) return 0;
    } else if (argc == 3 && (string)argv[1] == "don't" && (string)argv[2] == "lock") {
        if (IsProcessAlreadyRunning(curExe)) return 0;
        preLockKeyboard = preLockMouse = false;
    } else if (argc == 1) {
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

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWindowsHookEx(keyboardHook);
    UnhookWindowsHookEx(mouseHook);
    return 0;
}
