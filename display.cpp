#include "display.h"
#pragma comment(lib, "dxva2.lib")
#pragma comment(lib, "Powrprof.lib")

namespace DisplayCtrl {

    int g_MonitorState = 1; // Default to On
    HPOWERNOTIFY hPowerNotify = NULL;

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

        if (increase) {
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
        return;

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
	// ID 1: Next Track, ID 2: Prev Track, ID 3: Stop
    bool Initialize(HWND hWnd) {
        hPowerNotify = RegisterPowerSettingNotification(hWnd, &GUID_CONSOLE_DISPLAY_STATE, DEVICE_NOTIFY_WINDOW_HANDLE);
        return RegisterHotKey(hWnd, 1, 0, VK_MEDIA_NEXT_TRACK) &&
               RegisterHotKey(hWnd, 2, 0, VK_MEDIA_PREV_TRACK) &&
               RegisterHotKey(hWnd, 3, 0, VK_MEDIA_STOP);
    }

    void Cleanup() {
        UnregisterHotKey(NULL, 1);
        UnregisterHotKey(NULL, 2);
        UnregisterHotKey(NULL, 3);
        if (hPowerNotify) {
            UnregisterPowerSettingNotification(hPowerNotify);
        }
	}

    void ToggleMonitor() {
        if (g_MonitorState != 0) {
            PostMessage(HWND_BROADCAST, WM_SYSCOMMAND, SC_MONITORPOWER, 2);
        }
        else {
            PostMessage(HWND_BROADCAST, WM_SYSCOMMAND, SC_MONITORPOWER, -1);
        }
    }

    LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
        switch (message) {
			case WM_POWERBROADCAST:
				if (wParam == PBT_POWERSETTINGCHANGE) {
					POWERBROADCAST_SETTING* setting = (POWERBROADCAST_SETTING*)lParam;
					if (setting->PowerSetting == GUID_CONSOLE_DISPLAY_STATE) {
						g_MonitorState = *(int*)setting->Data; // Update our source of truth
					}
				}
				break;

			case WM_HOTKEY:
				switch(wParam) {
					case 1: // Next Track
						AdjustPowerSchemeBrightness(true);
						break;
					case 2: // Prev Track
						AdjustPowerSchemeBrightness(false);
						break;
					case 3: // Stop
						ToggleMonitor();
						break;
					default:
						break;
				}
				break;

			case WM_DESTROY:
                Cleanup();
                PostQuitMessage(0);
                break;

			default:
				return DefWindowProc(hWnd, message, wParam, lParam);
        }
        return 0;
    }
}
