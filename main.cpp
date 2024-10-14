#include <Windows.h>

#include "Window.h"


#define SHOW_CONSOLE

#ifdef SHOW_CONSOLE
int main()
#else
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
#endif
{
	Window& window = Window::getInstance();

	window.init("NoPeek for Windows");
	window.run();

	return 0;
}
