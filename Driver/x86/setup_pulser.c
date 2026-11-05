#include "v1495_pulser.h"

int
main(void)
{
    vmeOpenDefaultWindows();
    V1495PulserInit(0x800000);

    /* Use the driver */

    vmeCloseDefaultWindows();
    return 0;
}
