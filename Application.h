#pragma once


enum class SaveResult
{
	OK,
	NOIMAGE,
	ABORTED,
	INVALIDEXT,
	FAILED,
};


class Application
{
public:

	static void log(const char* msg);

	static void openImage();

	static SaveResult saveImage();
};
