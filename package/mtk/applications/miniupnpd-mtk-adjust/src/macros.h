/* $Id: macros.h,v 1.7 2022/10/16 06:03:56 nanard Exp $ */
/* MiniUPnP project
 * http://miniupnp.free.fr/ or https://miniupnp.tuxfamily.org/
 * (c) 2012-2022 Thomas Bernard
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */

#ifndef MACROS_H_INCLUDED
#define MACROS_H_INCLUDED

#define UNUSED(arg)	(void)(arg)

#if defined(__GNUC__) && (__GNUC__ >= 7)
#define FALL_THROUGH __attribute__((fallthrough))
#else
#define FALL_THROUGH
#endif

/* Macro to print errors during initialization.
 * Print them on both stderr and syslog.
 * if debug_flag is on, syslog already print on console */
#define INIT_PRINT_ERR(...) do { if (!debug_flag) fprintf(stderr, __VA_ARGS__); syslog(LOG_ERR, __VA_ARGS__); } while(0)

#endif /* MACROS_H_INCLUDED */
