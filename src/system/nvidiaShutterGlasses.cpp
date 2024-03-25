
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <string>
#include <fstream>
#include <cmath>
using namespace std;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include "nvidiaShutterGlasses.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <initguid.h>
#include <setupapi.h>
#include <usbdi.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

string getDeviceName(HDEVINFO hardwareDeviceInfo, PSP_DEVICE_INTERFACE_DATA deviceInfoData)
{
    ULONG predictedLength = 0;
	ULONG requiredLength = 0;

    //allocate a function class device data structure to receive the goods about this particular device.
    SetupDiGetDeviceInterfaceDetail(hardwareDeviceInfo, deviceInfoData, NULL, 0, &requiredLength, NULL);
    PSP_DEVICE_INTERFACE_DETAIL_DATA functionClassDeviceData = new SP_DEVICE_INTERFACE_DETAIL_DATA[requiredLength];
	functionClassDeviceData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
    predictedLength = requiredLength;

    if (SetupDiGetDeviceInterfaceDetail(hardwareDeviceInfo, deviceInfoData, functionClassDeviceData, predictedLength, &requiredLength, NULL))
	{
		char name[256];
		//strncpy_s(name, functionClassDeviceData->DevicePath, 256);
		strncpy(name, functionClassDeviceData->DevicePath, 256);
		delete [] functionClassDeviceData;
		return name;
    }

	delete [] functionClassDeviceData;
	return "";
}

string findUsbDevice()
{
	HDEVINFO hardwareDeviceInfo = SetupDiGetClassDevs(&GUID_DEVINTERFACE_USB_DEVICE, 0, 0, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
	if (hardwareDeviceInfo == INVALID_HANDLE_VALUE)
		return "";

	//Enumerate through all devices in Set.
	SP_DEVICE_INTERFACE_DATA deviceInfoData;
	deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

	for (int i = 0; SetupDiEnumDeviceInterfaces(hardwareDeviceInfo, 0, &GUID_DEVINTERFACE_USB_DEVICE, i, &deviceInfoData); ++i)
	{
		string usbName = getDeviceName(hardwareDeviceInfo, &deviceInfoData);
		if (usbName.find("usb#vid_0955&pid_0007") != string::npos)
		{
			SetupDiDestroyDeviceInfoList(hardwareDeviceInfo);
			return usbName;
		}
	}

	SetupDiDestroyDeviceInfoList(hardwareDeviceInfo);
	return "";
}

HANDLE openUsbDeviceFile(const string& filename)
{
	string deviceName = findUsbDevice();
    if (deviceName == "")
        return INVALID_HANDLE_VALUE;

	return	CreateFile(	(deviceName + "\\" + filename).c_str(),
						GENERIC_WRITE | GENERIC_READ,
						FILE_SHARE_WRITE | FILE_SHARE_READ,
						NULL,
						OPEN_EXISTING,
						0,
						NULL);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T>
unsigned long writeToPipe(HANDLE pipe, T buffer, int bytes)
{
	unsigned long bytesWritten;
	WriteFile(pipe, (char*)buffer, bytes, &bytesWritten, NULL);
	return bytesWritten;
}

template <typename T>
unsigned long readFromPipe(HANDLE pipe, T buffer, int bytes)
{
	unsigned long bytesRead;
	ReadFile(pipe, buffer, bytes, &bytesRead, NULL);
	return bytesRead;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

NvidiaShutterGlasses::NvidiaShutterGlasses()
	: currentRefreshRate(0)
{
	ifstream fin("validRefreshRates.ini");

	while (!fin.eof())
	{
		string line;
		getline(fin, line);
		validRefreshRates.push_back(atoi(line.c_str()));
	}

	pipe0 = openUsbDeviceFile("PIPE02");
	pipe1 = openUsbDeviceFile("PIPE00");
	refresh();
}

NvidiaShutterGlasses::~NvidiaShutterGlasses()
{
	CloseHandle(pipe0);
	CloseHandle(pipe1);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//refresh variables and initialize usb device
void NvidiaShutterGlasses::refresh()
{
	int rate = validRefreshRates[currentRefreshRate];

	int a = (int)(0.1748910*(rate*rate*rate) - 54.5533*(rate*rate) + 6300.40*(rate) - 319395.0);
	int b = (int)(0.0582808*(rate*rate*rate) - 18.1804*(rate*rate) + 2099.82*(rate) - 101257.0);
	int c = (int)(0.3495840*(rate*rate*rate) - 109.060*(rate*rate) + 12597.3*(rate) - 638705.0);

	int sequence[] = {	0x00031842,
						0x00180001, a, b, 0xfffff830, 0x22302824, 0x040a0805, c,
						0x00021c01, 0x00000002,	//note only 6 bytes are actually sent here
						0x00021e01, rate*2,		//note only 6 bytes are actually sent here
						0x00011b01, 0x00000007,	//note only 5 bytes are actually sent here
						0x00031840	};

	HANDLE readPipe = openUsbDeviceFile("PIPE03");
	char readBuffer[7];

	writeToPipe(pipe0, sequence, 4);
	readFromPipe(readPipe, readBuffer, 7);
	writeToPipe(pipe0, sequence+1, 28);
	writeToPipe(pipe0, sequence+8, 6);
	writeToPipe(pipe0, sequence+10, 6);
	writeToPipe(pipe0, sequence+12, 5);
	writeToPipe(pipe0, sequence+13, 4);

	CloseHandle(readPipe);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void NvidiaShutterGlasses::toggleEyes(int offset)
{
	int sequence[]  = { isLeftEye() ? 0x0000feaa : 0x0000ffaa, offset };
	writeToPipe(pipe1, sequence, 8);
	ShutterGlasses::toggleEyes();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void NvidiaShutterGlasses::nextRefreshRate()
{
	currentRefreshRate++;
	if (currentRefreshRate >= validRefreshRates.size())
		currentRefreshRate = 0;
	refresh();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
