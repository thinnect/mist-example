#ifndef FSRECORD_H
#define FSRECORD_H

#define FS_RECORD_FILE_ERROR -1
#define NO_WAIT 0

/*****************************************************************************
 * Callback function for read/write done event
 * @param len - Read/write data length in bytes
 * @param p_user - User pointer
 ****************************************************************************/
typedef void (*fs_rw_done_f) (int32_t len, void* p_user);

/*****************************************************************************
 * Function pointer definition to read from filesystem
 * @param file_sys_nr - File system number 0..2
 * @param p_file_name - Pointer to the file name
 * @param p_value - Pointer to the data record
 * @param len - Data record length in bytes
 * @param wait - When wait = 0 function returns immediately, even when putting fails,
 *                otherwise waits until put succeeds (and blocks calling thread)
 * @param callback_func - User callback function
 * @param p_user - User pointer
 * @return Returns number of bytes to write on success, 0 otherwise
 ****************************************************************************/
typedef int32_t (*fs_read_record_f) (int file_sys_nr,
                                     const char* p_file_name,
                                     void* p_value,
                                     int32_t len,
                                     uint32_t wait,
                                     fs_rw_done_f callback_func,
                                     void* p_user);

/*****************************************************************************
 * Function pointer definition to write to filesystem
 * @param file_sys_nr - File system number 0..2
 * @param p_file_name - Pointer to the file name
 * @param p_value - Pointer to the data record
 * @param len - Data record length in bytes
 * @param wait - When wait = 0 function returns immediately, even when putting fails,
 *                otherwise waits until put succeeds (and blocks calling thread)
 * @param callback_func - User callback function
 * @param p_user - User pointer
 * @return Returns number of bytes to write on success, 0 otherwise
 ****************************************************************************/
typedef int32_t (*fs_write_record_f) (int file_sys_nr,
                                     const char* p_file_name,
                                     const void* p_value,
                                     int32_t len,
                                     uint32_t wait,
                                     fs_rw_done_f callback_func,
                                     void* p_user);

#endif  // FSRECORD_H
