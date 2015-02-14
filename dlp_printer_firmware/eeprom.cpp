#include "eeprom.h"


namespace eeprom {
  void write(uint8_t& start_idx, const uint8_t* data_arr, uint8_t size) {
    /* Write `size` bytes of data to EEPROM
     * The available address space is indexed between 0 and 255
     *
     * `start_idx` - int between 0 and 255 that addresses the EEPROM
     * `data_arr` - data to save.
     * `size` number of bytes to write
     */
    while (size--) {
      eeprom_write_byte((unsigned char *) start_idx, *data_arr);
      start_idx++;
      data_arr++;
    }
  }

  void read(uint8_t& start_idx, uint8_t* data_arr, uint8_t size) {
    /* Read `size` bytes of data from EEPROM and write to `data_arr`
     *
     * `start_idx` - int between 0 and 255 that addresses the EEPROM
     * `data_arr` - empty array to save data to
     * `size` number of bytes to write
     */
    while (size--) {
      *data_arr = eeprom_read_byte((unsigned char *) start_idx);
      start_idx++;
      data_arr++;
    }
  }
}
