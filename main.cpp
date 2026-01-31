#include <windows.h>
#include <powrprof.h>

#pragma comment(lib, "Powrprof.lib")

const DWORD DELTA = 25;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    GUID* activeGuid = nullptr;

    // Get the active scheme
    if (PowerGetActiveScheme(NULL, &activeGuid) != ERROR_SUCCESS) {
        return 0;
    }

    const GUID subgroup = GUID_VIDEO_SUBGROUP;
    const GUID setting = GUID_DEVICE_POWER_POLICY_VIDEO_BRIGHTNESS;
    DWORD dcVal, acVal;

    // Direct Read
    PowerReadDCValueIndex(NULL, activeGuid, &subgroup, &setting, &dcVal);
    PowerReadACValueIndex(NULL, activeGuid, &subgroup, &setting, &acVal);

    //For decreasing
    dcVal = (dcVal < DELTA) ? 0 : dcVal - DELTA;
    acVal = (acVal < DELTA) ? 0 : acVal - DELTA;
	//for increasing
    //dcVal = (dcVal + DELTA > 100) ? 100 : dcVal + DELTA;
    //acVal = (acVal + DELTA > 100) ? 100 : acVal + DELTA;


    // Direct Write
    PowerWriteDCValueIndex(NULL, activeGuid, &subgroup, &setting, dcVal);
    PowerWriteACValueIndex(NULL, activeGuid, &subgroup, &setting, acVal);

    // Apply changes immediately
    PowerSetActiveScheme(NULL, activeGuid);

    LocalFree(activeGuid);
    return 0;
}