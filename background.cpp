#include <windows.h>
#include <powrprof.h>
#include <highlevelmonitorconfigurationapi.h>
#include <physicalmonitorenumerationapi.h>
#include <vector>

#pragma comment(lib, "dxva2.lib")
#pragma comment(lib, "Powrprof.lib")

const DWORD DELTA = 10;

void AdjustPowerSchemeBrightness(bool increase) {
     GUID* activeGuid = nullptr;
 
     // Get the active scheme
     if (PowerGetActiveScheme(NULL, &activeGuid) != ERROR_SUCCESS) {
         return;
     }
 
     const GUID subgroup = GUID_VIDEO_SUBGROUP;
     const GUID setting = GUID_DEVICE_POWER_POLICY_VIDEO_BRIGHTNESS;
     DWORD dcVal = 0, acVal = 0;
 
     // Direct Read
     if (PowerReadDCValueIndex(NULL, activeGuid, &subgroup, &setting, &dcVal) != ERROR_SUCCESS ||
         PowerReadACValueIndex(NULL, activeGuid, &subgroup, &setting, &acVal) != ERROR_SUCCESS) {
         LocalFree(activeGuid);
         return;
     }
 
     if(increase) {
         dcVal = (dcVal + DELTA > 100) ? 100 : dcVal + DELTA;
         acVal = (acVal + DELTA > 100) ? 100 : acVal + DELTA;
     }
     else {
         dcVal = (dcVal < DELTA) ? 0 : dcVal - DELTA;
         acVal = (acVal < DELTA) ? 0 : acVal - DELTA;
     }
 
     // Direct Write
     PowerWriteDCValueIndex(NULL, activeGuid, &subgroup, &setting, dcVal);
     PowerWriteACValueIndex(NULL, activeGuid, &subgroup, &setting, acVal);
 
     // Apply changes immediately
     PowerSetActiveScheme(NULL, activeGuid);
 
     LocalFree(activeGuid);
     return ;

}

void AdjustMonitorBrightness(bool increase) {
    // Get the handle to the primary monitor
    HMONITOR hMonitor = MonitorFromWindow(GetDesktopWindow(), MONITOR_DEFAULTTOPRIMARY);

    // Get the number of physical monitors
    DWORD numPhysicalMonitors = 0;
    if (!GetNumberOfPhysicalMonitorsFromHMONITOR(hMonitor, &numPhysicalMonitors)) return;

    // Get the physical monitor array
    std::vector<PHYSICAL_MONITOR> pPhysicalMonitors(numPhysicalMonitors);

    if (GetPhysicalMonitorsFromHMONITOR(hMonitor, numPhysicalMonitors, pPhysicalMonitors.data())) {

        for (DWORD i = 0; i < numPhysicalMonitors; i++) {
            DWORD minBrightness, curBrightness, maxBrightness;

            if (GetMonitorBrightness(pPhysicalMonitors[i].hPhysicalMonitor, &minBrightness, &curBrightness, &maxBrightness)) {

                DWORD newBrightness = curBrightness;

                if (increase) {
                    newBrightness = (curBrightness + DELTA > maxBrightness) ? maxBrightness : curBrightness + DELTA;
                }
                else {
                    newBrightness = (curBrightness < DELTA) ? minBrightness : curBrightness - DELTA;
                }

                SetMonitorBrightness(pPhysicalMonitors[i].hPhysicalMonitor, newBrightness);
            }
        }

        //Clean up handles
        DestroyPhysicalMonitors(numPhysicalMonitors, pPhysicalMonitors.data());
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // ID 1: Next Track, ID 2: Prev Track
    if (!RegisterHotKey(NULL, 1, 0, VK_MEDIA_NEXT_TRACK) ||
        !RegisterHotKey(NULL, 2, 0, VK_MEDIA_PREV_TRACK)) {
        MessageBox(NULL, L"Failed to register hotkeys. Another app might be using them.", L"Error", MB_ICONERROR);
        return 1;
    }

    MSG msg = { 0 };
    // GetMessage will still retrieve thread-level messages
    while (GetMessage(&msg, NULL, 0, 0)) {
        if (msg.message == WM_HOTKEY) {
            if (msg.wParam == 1) {
				AdjustPowerSchemeBrightness(true);
            }
            else if (msg.wParam == 2) {
				AdjustPowerSchemeBrightness(false);
            }
        }
    }
    UnregisterHotKey(NULL, 1);
    UnregisterHotKey(NULL, 2);
    return 0;
}