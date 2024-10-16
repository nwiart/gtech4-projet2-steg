#pragma once
#include <windows.h>

#include <gdiplus.h>
#include <vector>
#include <string>

#pragma comment (lib, "Gdiplus.lib")

class ResAnalysis
{
public:
	static ResAnalysis& GetInstance();

	ResAnalysis();
	~ResAnalysis();

	static bool AddNoise(double mean, double stddev);
	static bool AddCompression(ULONG quality);


	void GenerateImages();
private:
	ULONG_PTR gdiplusToken;
};

