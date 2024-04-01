
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <string>
#include <fstream>
#include <cmath>
#include <iostream>
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
	: currentProfile(0)
	, x_offset(0.0f)
	, y_offset(0.0f)
	, w_offset(0.0f)
{
	/*
	ifstream fin("validRefreshRates.ini");
	if (fin.is_open())
	{
		while (!fin.eof())
		{
			string line;
			getline(fin, line);
			//validRefreshRates.push_back(atoi(line.c_str()));
			validRefreshRates.push_back(stof(line));
		}
		fin.close();
	}
	else { validRefreshRates.push_back(99.999f); }
	*/

	ifstream fin("MonitorTimings.ini");
	if (fin.is_open())
	{
		while (!fin.eof())
		{
			string line;
			getline(fin, line);

			string searchstr = "Monitor:";
			size_t found = line.find(searchstr);
			if (found != std::string::npos)
				MonitorID.push_back(line.substr(found + searchstr.length()));

			searchstr = "EDID_ID:";
			found = line.find(searchstr);
			if (found != std::string::npos)
				EDID_ID.push_back(line.substr(found + searchstr.length()));
			
			searchstr = "RefreshRateHz:";
			found = line.find(searchstr);
			if (found != std::string::npos) 
			validRefreshRates.push_back( stof( line.substr(found+searchstr.length()) ) );

			searchstr = "X_us:";
			found = line.find(searchstr);
			if (found != std::string::npos)
				valid_x_us.push_back(stof(line.substr(found + searchstr.length())));

			searchstr = "Y_us:";
			found = line.find(searchstr);
			if (found != std::string::npos)
				valid_y_us.push_back(stof(line.substr(found + searchstr.length())));

			searchstr = "Z_us:";
			found = line.find(searchstr);
			if (found != std::string::npos)
				valid_z_us.push_back(stof(line.substr(found + searchstr.length())));

			searchstr = "W_us:";
			found = line.find(searchstr);
			if (found != std::string::npos)
				valid_w_us.push_back(stof(line.substr(found + searchstr.length())));
		}
		fin.close();
	}
	else { 
		MonitorID.push_back("No MonitorTimings.ini !!!");
		EDID_ID.push_back("DummyID");
		validRefreshRates.push_back(120.0f);
		valid_x_us.push_back(1.0f);
		valid_y_us.push_back(7333.0f);
		valid_z_us.push_back(8333.34f);
		valid_w_us.push_back(4735.0f);
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
	//int rate = validRefreshRates[currentRefreshRate];

	float rate = validRefreshRates[currentProfile];
	float x_us = valid_x_us[currentProfile];	//us
	float y_us = valid_y_us[currentProfile];	//us(activeTime)
	float z_us = valid_z_us[currentProfile];	//us(frameTime) (Can be calculated from 1000ms/rate ???)
	float w_us = valid_w_us[currentProfile];	//us
	
	//Asus PG248Q (AUS24B1) Values from NvTimingsEd 120Hz
	//float rate = 119.983 ;
	//float x_us =    0.50 ;	//us
	//float y_us = 7334.00 ;	//us(activeTime)
	//float z_us = 8334.50 ;	//us(frameTime) (Can be calculated from 1000ms/rate ???)
	//float w_us = 4735.58 ;	//us

	// prevent negative timing values
	if ((x_us + x_offset) < 0.0) { x_offset = 0 - x_us; }
	if ((y_us + y_offset) < 0.0) { y_offset = 0 - y_us; }
	if ((w_us + w_offset) < 0.0) { w_offset = 0 - w_us; }

	x_us += x_offset;	
	y_us += y_offset;	
	w_us += w_offset;

	//int NVSTUSB_CLOCK = 48000000; // CPU clock of IR emitter
	//int NVSTUSB_T0_CLOCK = NVSTUSB_CLOCK / 12 / 1000000; // T0 runs at  4MHz
	//int NVSTUSB_T2_CLOCK = NVSTUSB_CLOCK /  4 / 1000000; // T2 runs at 12MHz
	int x = (int)(-x_us * 4 + 1); // T0 runs at  4MHz
	int y = (int)(-y_us * 4 + 1); // T0 runs at  4MHz
	int z = (int)(-z_us *12 + 1); // T2 runs at 12MHz
	int w = (int)(-w_us *12 + 1); // T2 runs at 12MHz
	int timeout = (int)(rate * 4); // idle timeout(number of frames)

	//int a = (int)(0.1748910*(rate*rate*rate) - 54.5533*(rate*rate) + 6300.40*(rate) - 319395.0);
	//int b = (int)(0.0582808*(rate*rate*rate) - 18.1804*(rate*rate) + 2099.82*(rate) - 101257.0);
	//int c = (int)(0.3495840*(rate*rate*rate) - 109.060*(rate*rate) + 12597.3*(rate) - 638705.0);

	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), { (SHORT)0, (SHORT)13 });
	cout << "Monitor: " << MonitorID[currentProfile] << "       \n"
		 << "EDID ID: " << EDID_ID[currentProfile] << "       \n"
		 << "ScreenRefresh: " << rate << " Hz      \n"
		 << "x: " << x_us << "us                   " << x << "       \n"
		 << "y: " << y_us << "us                   " << y << "       \n"
		 << "z: " << z_us << "us                   " << z << "       \n"
		 << "w: " << w_us << "us                   " << w << "       \n"
		 << "                                        \n"
		 << "                                        \n";
	//cout << "a: " << a << "       \n";
	//cout << "b: " << b << "       \n";
	//cout << "c: " << c << "       \n";
	/*
	int sequence[] = {	0x00031842,
						0x00180001, a, b, 0xfffff830, 0x22302824, 0x040a0805, c,
						0x00021c01, 0x00000002,	//note only 6 bytes are actually sent here
						0x00021e01, rate*2,		//note only 6 bytes are actually sent here
						0x00011b01, 0x00000007,	//note only 5 bytes are actually sent here
						0x00031840	};
	*/
	int sequence[] = {	0x00031842,
						0x00180001, w, x, y, 0x22242830, 0x0405080a, z,
						0x00021c01, 0x00000002,	//note only 6 bytes are actually sent here
						0x00021e01, timeout,	//note only 6 bytes are actually sent here
						0x00011b01, 0x00000007,	//note only 5 bytes are actually sent here
						0x00031840 };

	HANDLE readPipe = openUsbDeviceFile("PIPE03");
	char readBuffer[7];

	writeToPipe(pipe0, sequence, 4);    // 42 18 03 00
	// Here start the problems with the sleep mode of the IR emitter !!!
	// readFromPipe fails after sleep mode
	readFromPipe(readPipe, readBuffer, 7);
	writeToPipe(pipe0, sequence+1, 28); // 01 00 18 00,ww ww ww ww,xx xx xx xx,yy yy yy yy,22 24 28 30,04 05 08 0a,zz zz zz zz
	writeToPipe(pipe0, sequence+8, 6);  // 01 1c 02 00,02 00
	writeToPipe(pipe0, sequence+10, 6); // 01 1e 02 00,timeout
	writeToPipe(pipe0, sequence+12, 5); // 01 1b 01 00,07
	writeToPipe(pipe0, sequence+13, 4); // 40 18 03 00

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

void NvidiaShutterGlasses::nextProfile()
{
	currentProfile++;
	if (currentProfile >= validRefreshRates.size())
		currentProfile = 0;
	x_offset = 0.0f;
	y_offset = 0.0f;
	w_offset = 0.0f;
	refresh();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
