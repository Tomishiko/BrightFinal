#pragma once
#include <windows.h>
#include <powrprof.h>
#include <highlevelmonitorconfigurationapi.h>
#include <physicalmonitorenumerationapi.h>
#include <vector>
namespace DisplayCtrl
{
	//Delta for adjusting brightness, in percentage
	const DWORD DELTA = 10;
	extern int g_MonitorState;
	extern HPOWERNOTIFY hPowerNotify;

	//Brightness adjustment function for built in displays
	void AdjustPowerSchemeBrightness(bool);
	//Brightness adjustment function for external monitors
	void AdjustMonitorBrightness(bool);
	//Toggle monitor on/off state
	void ToggleMonitor();

	bool Initialize(HWND hWnd);
	LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	void Cleanup();

}

