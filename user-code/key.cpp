#include <string>
#include <vector>
#include <windows.h>
using namespace std;
#define LB WM_LBUTTONDOWN
#define RB WM_RBUTTONDOWN
#define MB WM_MBUTTONDOWN
#define MV WM_MOUSEMOVE
#define WH WM_MOUSEWHEEL
// extern the variables so the compiler can find them when linking
extern const string unlockKeyboardKey;
extern const vector<WPARAM> unlockMouseKey;

// define your own key here
// Warning: If you make the key empty, that means you can never unlock your
//          computer!
const string unlockKeyboardKey = "abc";
const vector<WPARAM> unlockMouseKey = {MB, LB, RB};

// Info: We only check the mouse action in your unlock key.
//       For example, if you don't contain MV, we wouldn't check the mouse move
//       action so it won't cause you to re-enter the key.
// Info: We only check the first mouse move or scroll action in a line,
//       because it makes many move or scroll actions when you do it,
//       so never make your key like this:
//       {..., MV, MV, ...} or {..., WH, WH, ...}!
// Info: We can only check the letter and the symbol that can be  input in one hit,
//       so the charset is: 1234567890qwertyuiopasdfghjklzxcvbnm
//       QWERTYUIOPASDFGHJKLZXCVBNM`-=[]\;',./


