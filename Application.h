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

	static void saveImage();


	static const char* getSaveResultString(SaveResult r);

private:

	static SaveResult _saveImage();
};
