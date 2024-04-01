
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <iostream>
using namespace std;
#include <windows.h>
#include "mainApp.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main()
{
	cout << "3D Vision Activator\n";
	cout << "###################\n";
	cout << "F1 - toggles stereo\n";
	cout << "F2 - swaps eyes\n";
	cout << "F3 - next Monitor profile(or loops to the first)\n";
	//cout << "Shift +F5/+F6 - modifies the convergence\n";
	cout << "F5 / F6 - modifies the separation\n";
	cout << "X / S - modifies the x timing\n";
	cout << "Y / A - modifies the y timing\n";
	cout << "W / Q - modifies the w timing\n";
	cout << "ESC - Close App\n";
	cout << "###################\n";
	//cerr << "Output error\n";

	return WinMain(GetModuleHandle(NULL), NULL, GetCommandLineA(), SW_SHOWNORMAL);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	MainApp app(sf::VideoMode(1200/1, 1020/1, 32), "Stereo Activator");
	app.run();
    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
