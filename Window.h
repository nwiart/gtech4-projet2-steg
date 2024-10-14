#pragma once

// Forward declaration.
typedef struct HWND__* HWND;


class Window
{
public:

	Window();

	void init(const char* title);

	void run();


private:

	HWND m_hwnd;
};
