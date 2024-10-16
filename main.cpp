#include <Windows.h>

#include "Window.h"
#include "ResAnalysis.h"


#define SHOW_CONSOLE

#ifdef SHOW_CONSOLE
int main()
#else
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
#endif
{
	ResAnalysis::GetInstance().GenerateImages();

	Window& window = Window::getInstance();

	window.init("NoPeek for Windows");
	window.run();

	return 0;
}
