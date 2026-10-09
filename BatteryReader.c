#include <windows.h>
#include <stdio.h>

int main(void) {
    SYSTEM_POWER_STATUS powerStatus;

    if (GetSystemPowerStatus(&powerStatus)) {
        printf("Battery: %u%%\n", powerStatus.BatteryLifePercent);
//        printf("AC power: %s\n",
        printf("AC power: %s\n",
            powerStatus.ACLineStatus == 1 ? "Connected" : "Disconnected");
    }

    return 0;
}