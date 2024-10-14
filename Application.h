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

	static void openImage();

	static SaveResult saveImage();
};
