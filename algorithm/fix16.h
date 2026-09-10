#ifndef FIX16_H
#define FIX16_H

#include <stdint.h>

/* Q16.16: value = raw / 65536 */
typedef int32_t fix16_t;

#define FIX16_ONE  65536
#define FIX16_MAX  INT32_MAX
#define FIX16_MIN  INT32_MIN

/* Compile-time constants ONLY. Never pass a variable. */
#define F16(x) ((fix16_t)((x) * 65536.0))

fix16_t fix16_add(fix16_t a, fix16_t b);
fix16_t fix16_sub(fix16_t a, fix16_t b);

#endif
