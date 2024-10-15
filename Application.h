#pragma once


enum class EncodeMethod
{
	LSB,
	MATRIX_EMBEDDING,
	NUM_METHODS
};

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
	static void openMessage();
	static void saveImage();

	static void encode(EncodeMethod m);
	static void decode(EncodeMethod m);


	static const char* getEncodeMethodString(EncodeMethod r);
	static const char* getSaveResultString(SaveResult r);

private:

	static SaveResult _saveImage();
};
