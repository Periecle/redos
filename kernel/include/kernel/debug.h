#ifndef _KERNEL_DEBUG_H
#define _KERNEL_DEBUG_H

#include <stddef.h>
#include <stdarg.h>

/* Debug level definitions */
#define DEBUG_LEVEL_NONE    0
#define DEBUG_LEVEL_ERROR   1
#define DEBUG_LEVEL_WARNING 2
#define DEBUG_LEVEL_INFO    3
#define DEBUG_LEVEL_DEBUG   4
#define DEBUG_LEVEL_TRACE   5

/* Output target flags - can be combined with bitwise OR */
#define DEBUG_TARGET_NONE   0x00
#define DEBUG_TARGET_VGA    0x01
#define DEBUG_TARGET_SERIAL 0x02
#define DEBUG_TARGET_ALL    0xFF

/* Initialize the debug subsystem */
void debug_init(void);

/* Set the current debug level */
void debug_set_level(int level);

/* Get the current debug level */
int debug_get_level(void);

/* Set the debug output target */
void debug_set_target(int target);

/* Get the current debug output target */
int debug_get_target(void);

/* Debug output functions for different levels */
void debug_error(const char* format, ...);
void debug_warning(const char* format, ...);
void debug_info(const char* format, ...);
void debug_debug(const char* format, ...);
void debug_trace(const char* format, ...);

/* Generic debug output function with specified level */
void debug_log(int level, const char* format, ...);

/* Raw debug output function (bypasses level checks) */
void debug_print(const char* format, ...);
void debug_vprint(const char* format, va_list args);

/* Function to output a buffer as hex bytes (for memory dumps) */
void debug_hex_dump(const void* data, size_t size);

/* Custom implementations of snprintf and vsnprintf for minimal dependencies */
int vsnprintf(char* str, size_t size, const char* format, va_list args);
int snprintf(char* str, size_t size, const char* format, ...);

#endif /* _KERNEL_DEBUG_H */
