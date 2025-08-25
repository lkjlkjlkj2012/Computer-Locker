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

HHOOK keyboardHook,mouseHook;

extern const string unlockKeyboardKey;
extern const vector<WPARAM> unlockMouseKey;

vector<int> keyboardKeyPoses = {-1};
vector<int> mouseKeyPoses = {-1};
bool lockKeyboard = true, lockMouse = true;
bool preLockKeyboard = true, preLockMouse = true;
WPARAM lastMouseAction = 0;

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    lockKeyboard = preLockKeyboard;
    
    if (nCode == HC_ACTION && wParam == WM_KEYDOWN && unlockKeyboardKey.length()>0) {
        KBDLLHOOKSTRUCT* pKey = (KBDLLHOOKSTRUCT*)lParam;
        char pressedKey = MapVirtualKey(pKey->vkCode, MAPVK_VK_TO_CHAR);

        vector<int> tmp;
        bool flag=false;
        for(int keyboardKeyPos:keyboardKeyPoses)
        	if(tolower(pressedKey) == tolower(unlockKeyboardKey[keyboardKeyPos+1])) {
        		if(keyboardKeyPos+1==unlockKeyboardKey.length()-1) {
        			flag=true;
        			preLockKeyboard=!preLockKeyboard;
				} else {
					tmp.push_back(keyboardKeyPos+1);
				}
			}
		if(flag)
			tmp.clear();
		tmp.push_back(-1);
		keyboardKeyPoses=tmp;
    }
    
    if(lockKeyboard)
        return 1;  // À¹½ØËùÓÐ¼üÅÌÊäÈë
    return CallNextHookEx(keyboardHook, nCode, wParam, lParam);
}

LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
    lockMouse = preLockMouse;
    
    if (nCode == HC_ACTION) {
        if (unlockMouseKey.size() > 0 &&
            find(unlockMouseKey.begin(), unlockMouseKey.end(), wParam) !=
            unlockMouseKey.end() && !(lastMouseAction == wParam &&
            (wParam == MV || wParam == WH))) {
            #ifdef DEBUG
            cout<<"wParam="<<wParam<<endl;
            #endif
            
            vector<int> tmp;
            bool flag=false;
            lastMouseAction = wParam;
            for(int mouseKeyPos:mouseKeyPoses)
	            if (wParam == unlockMouseKey[mouseKeyPos+1]) {
	                if (mouseKeyPos+1 == unlockMouseKey.size()-1) {
	                    flag=true;
	                    preLockMouse = !preLockMouse;
	                } else {
	                	tmp.push_back(mouseKeyPos+1);
					}
	            }
	        if(flag)
	        	tmp.clear();
	        tmp.push_back(-1);
	        mouseKeyPoses=tmp;
	        #ifdef DEBUG
	        cout<<"mouseKeyPoses={";
	        for(int i=0;i<mouseKeyPoses.size();i++)
	        	cout<<mouseKeyPoses[i]<<(i==mouseKeyPoses.size()-1?"":" ");
	        cout<<"}"<<endl;
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

const string info = "Computer Locker 1.0\n"
                    "Make by lkjlkjlkj2012\n"
                    "\n"
                    "Use it to lock your computer to avoid JC and unauthorized use.\n"
                    "Change key.cpp to set your own key.\n"
                    "Use \"locker\" to confirm then lock your computer.\n"
                    "Use \"locker don't lock\" to start without confirm and not lock your computer.\n"
                    "Use \"locker skip confirm\" to skip confirm and lock computer.\n"
                    "Other use will pop this info.\n"
                    "\n"
                    "Warning: Never start two or more locker, it may cause your computer\n"
                    "         lock forever and you need to close your computer!\n"
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
            "Warning: This program will lock your computer by block all input!\n"
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

