#pragma once

#include "BinaryBuffer.h"


enum class EncodeMethod
{
	LSB,
	LSB_EXTENDED,
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
	static void saveMessage();

	static void encode(EncodeMethod m);
	static void decode(EncodeMethod m);


	static int getMaximumBytes(EncodeMethod m, int width, int height);

	static const BinaryBuffer& getSelectedMessage();
	static const BinaryBuffer& getDecodedMessage();


	static const char* getEncodeMethodString(EncodeMethod r);
	static const char* getSaveResultString(SaveResult r);

private:

	static SaveResult _saveImage();
	static SaveResult _saveMessage();
};
