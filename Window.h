#pragma once

// Forward declaration.
typedef struct HWND__* HWND;
typedef struct HDC__* HDC;


class Window
{
public:

	static Window& getInstance();

public:

	Window();

	void init(const char* title);

	void run();

	inline HWND getHwnd() const { return m_hwnd; }


private:

	HWND m_hwnd;
};
