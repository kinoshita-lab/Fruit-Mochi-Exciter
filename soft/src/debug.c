
#include "debug.h"
#include <stdint.h>
#define DEBUG_DATA0_ADDRESS ((volatile uint32_t *)0xE00000F4)
#define DEBUG_DATA1_ADDRESS ((volatile uint32_t *)0xE00000F8)
/*********************************************************************
 * @fn      SDI_Printf_Enable
 *
 * @brief   Initializes the SDI printf Function.
 *
 * @param   None
 *
 * @return  None
 */
void SDI_Printf_Enable(void) { *(DEBUG_DATA0_ADDRESS) = 0; }

/*********************************************************************
 * @fn      _write
 *
 * @brief   Support Printf Function
 *
 * @param   *buf - UART send Data.
 *          size - Data length.
 *
 * @return  size - Data length
 */
__attribute__((used)) int _write(int fd, char *buf, int size) {
  int i = 0;
  int writeSize = size;
  do {

    /**
     * data0  data1 8 bytes
     * data0 The lowest byte storage length, the maximum is 7
     *
     */
    while ((*(DEBUG_DATA0_ADDRESS) != 0u)) {
    }

    if (writeSize > 7) {
      *(DEBUG_DATA1_ADDRESS) = (*(buf + i + 3)) | (*(buf + i + 4) << 8) |
                               (*(buf + i + 5) << 16) | (*(buf + i + 6) << 24);
      *(DEBUG_DATA0_ADDRESS) = (7u) | (*(buf + i) << 8) |
                               (*(buf + i + 1) << 16) | (*(buf + i + 2) << 24);

      i += 7;
      writeSize -= 7;
    } else {
      *(DEBUG_DATA1_ADDRESS) = (*(buf + i + 3)) | (*(buf + i + 4) << 8) |
                               (*(buf + i + 5) << 16) | (*(buf + i + 6) << 24);
      *(DEBUG_DATA0_ADDRESS) = (writeSize) | (*(buf + i) << 8) |
                               (*(buf + i + 1) << 16) | (*(buf + i + 2) << 24);

      writeSize = 0;
    }

  } while (writeSize);

  return writeSize;
}
  