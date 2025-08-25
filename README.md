# Computer-Locker
A computer locker designed for Windows OS that can lock the mouse and keyboard, except for Ctrl+Alt+Delete and Win+L. Can't lock some touchscreen and touchpad operations.
When you run it, it will lock your computer. Enter your mouse key to unlock the mouse, re-enter it to lock the mouse, and so on. The same applies to the keyboard.
If you run it without administrator privileges, other people can use Task Manager to close it.
If you run it as an administrator, other people can't close it, except by logging out (closing all applications).

# How to use
1. Open the program without any parameters, best suited for user use.
2. Open the program with the parameters `skip confirm`, start quietly.
3. Open the program with parameters `don't lock`, start quietly, and don't lock your computer.
4. Other parameters will cause an info window.

# How to build some object files for the user
Run the `prepare-build.bat` file.
The object files can be found at [here](https://github.com/lkjlkjlkj2012/Computer-Locker/releases).

# How to set your own key
Change file `user-code/key.cpp`.

# How to build a complete executable file
Run the `user-code/build.bat` file.
