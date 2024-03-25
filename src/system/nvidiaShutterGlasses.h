#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <vector>
#include "shutterGlasses.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class NvidiaShutterGlasses : public ShutterGlasses
{
public:
	NvidiaShutterGlasses();
	~NvidiaShutterGlasses();

	void toggleEyes(int offset);
	void nextRefreshRate();

private:
	void refresh();

	std::vector<int> validRefreshRates;
	int currentRefreshRate;
	HANDLE pipe0, pipe1;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
