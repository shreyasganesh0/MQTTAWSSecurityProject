#ifndef __SERIAL_H_
#define __SERIAL_H_

/**
 * @brief Initilize a serial peripheral
 * @param num The peripheral to initialize
 * @return 0 on success -1 on failure
 */
int serial_init(int num);
/**
 * @brief Initilize a serial peripheral
 * @param num The serial peripheral to use
 * @param buf Pointer to data to receive
 * @param len Length of buffer
 * @param timeout Maximum amount of time to receive
 * @return 0 on success -1 on failure
 */
int serial_read(int num, unsigned char *buf, int len, int timeout);
/**
 * @brief Initilize a serial peripheral
 * @param num The serial peripheral to use
 * @param buf Pointer to data to send
 * @param len Length of buffer
 * @return 0 on success -1 on failure
 */
void serial_write(int num, const unsigned char *buf, int len);
/**
 * @brief Uninstall serial driver
 * @param num The peripheral to uninstall
 * @return 0 on success -1 on failure
 */
int serial_delete(int num);

#endif