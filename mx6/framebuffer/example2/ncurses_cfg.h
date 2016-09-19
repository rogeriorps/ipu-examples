/* ncurses_cfg.h.  Generated automatically by configure.  */
/****************************************************************************
 * Copyright (c) 1998 Free Software Foundation, Inc.                        *
 *                                                                          *
 * Permission is hereby granted, free of charge, to any person obtaining a  *
 * copy of this software and associated documentation files (the            *
 * "Software"), to deal in the Software without restriction, including      *
 * without limitation the rights to use, copy, modify, merge, publish,      *
 * distribute, distribute with modifications, sublicense, and/or sell       *
 * copies of the Software, and to permit persons to whom the Software is    *
 * furnished to do so, subject to the following conditions:                 *
 *                                                                          *
 * The above copyright notice and this permission notice shall be included  *
 * in all copies or substantial portions of the Software.                   *
 *                                                                          *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS  *
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF               *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.   *
 * IN NO EVENT SHALL THE ABOVE COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,   *
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR    *
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR    *
 * THE USE OR OTHER DEALINGS IN THE SOFTWARE.                               *
 *                                                                          *
 * Except as contained in this notice, the name(s) of the above copyright   *
 * holders shall not be used in advertising or otherwise to promote the     *
 * sale, use or other dealings in this Software without prior written       *
 * authorization.                                                           *
 ****************************************************************************/

/****************************************************************************
 *  Author: Thomas E. Dickey <dickey@clark.net> 1998                        *
 ****************************************************************************/
/*
 * $Id: ncurses_tst.hin,v 1.2 1998/02/11 12:14:05 tom Exp $
 *
 * This is a template-file used to generate the "ncurses_cfg.h" file.
 *
 * Rather than list every definition, the configuration script substitutes
 * the definitions that it finds using 'sed'.  You need a patch (971222)
 * to autoconf 2.12 to do this.
 */
#ifndef NC_CONFIG_H
#define NC_CONFIG_H

#define SYSTEM_NAME "linux-gnu"
#define MIXEDCASE_FILENAMES 1
#define GCC_SCANF 1
#define GCC_SCANFLIKE(fmt,var) __attribute__((format(scanf,fmt,var)))
#define GCC_PRINTF 1
#define GCC_PRINTFLIKE(fmt,var) __attribute__((format(printf,fmt,var)))
#define GCC_UNUSED __attribute__((unused))
#define GCC_NORETURN __attribute__((noreturn))
#define SIG_ATOMIC_T volatile sig_atomic_t
#define HAVE_NCURSES_H 1
#define HAVE_TERM_H 1
#define NCURSES 1
#define HAVE_LIBPANEL 1
#define HAVE_LIBMENU 1
#define HAVE_LIBFORM 1
#define HAVE_FORM_H 1
#define HAVE_MENU_H 1
#define HAVE_PANEL_H 1
#define HAVE_TERM_ENTRY_H 1
#define STDC_HEADERS 1
#define TIME_WITH_SYS_TIME 1
#define HAVE_GETOPT_H 1
#define HAVE_LOCALE_H 1
#define HAVE_MATH_H 1
#define HAVE_STDARG_H 1
#define HAVE_SYS_IOCTL_H 1
#define HAVE_SYS_SELECT_H 1
#define HAVE_SYS_TIME_H 1
#define HAVE_TERMIOS_H 1
#define HAVE_UNISTD_H 1
#define HAVE_GETOPT_H 1
#define HAVE_GETOPT_HEADER 1
#define HAVE_GETOPT 1
#define HAVE_GETTIMEOFDAY 1
#define HAVE_MBLEN 1
#define HAVE_MBRLEN 1
#define HAVE_MBRTOWC 1
#define HAVE_MBSRTOWCS 1
#define HAVE_MBSTOWCS 1
#define HAVE_MBTOWC 1
#define HAVE_WCSRTOMBS 1
#define HAVE_WCSTOMBS 1
#define HAVE_TERM_H 1
#define HAVE_UNCTRL_H 1
#define HAVE_ASSUME_DEFAULT_COLORS 1
#define HAVE_CHGAT 1
#define HAVE_COLOR_SET 1
#define HAVE_FILTER 1
#define HAVE_GETBEGX 1
#define HAVE_GETCURX 1
#define HAVE_GETMAXX 1
#define HAVE_GETNSTR 1
#define HAVE_GETPARX 1
#define HAVE_GETWIN 1
#define HAVE_MVVLINE 1
#define HAVE_MVWVLINE 1
#define HAVE_NAPMS 1
#define HAVE_PUTWIN 1
#define HAVE_RESIZE_TERM 1
#define HAVE_RESIZETERM 1
#define HAVE_RIPOFFLINE 1
#define HAVE_SCR_DUMP 1
#define HAVE_SETUPTERM 1
#define HAVE_SLK_COLOR 1
#define HAVE_SLK_INIT 1
#define HAVE_TERMATTRS 1
#define HAVE_TGETENT 1
#define HAVE_TIGETNUM 1
#define HAVE_TIGETSTR 1
#define HAVE_TYPEAHEAD 1
#define HAVE_USE_DEFAULT_COLORS 1
#define HAVE_USE_ENV 1
#define HAVE_USE_EXTENDED_NAMES 1
#define HAVE_USE_SCREEN 1
#define HAVE_USE_WINDOW 1
#define HAVE_VIDPUTS 1
#define HAVE_VSSCANF 1
#define HAVE_VW_PRINTW 1
#define HAVE_WCHGAT 1
#define HAVE_WINSSTR 1
#define HAVE_WRESIZE 1
#define HAVE_WSYNCDOWN 1
#define HAVE_TPUTS 1
#define TPUTS_ARG               int
#define TPUTS_PROTO(func,value) int func(TPUTS_ARG value)
#define TPUTS_RETURN(value)     return value
#define USE_WIDEC_SUPPORT 0
#define HAVE_SYS_TIME_SELECT 1
#define HAVE_CURSES_VERSION 1
#define CURSES_ACS_ARRAY acs_map
#define HAVE_TYPE_ATTR_T 1
#define NEED_WCHAR_H 1
#define DECL_CURSES_DATA_OSPEED 1
#define HAVE_CURSES_DATA_BOOLNAMES 1
#define HAVE_CURSES_DATA_BOOLFNAMES 1

	/* The C compiler may not treat these properly but C++ has to */
#ifdef __cplusplus
#undef const
#undef inline
#else
#if defined(lint) || defined(TRACE)
#undef inline
#define inline /* nothing */
#endif
#endif

#endif /* NC_CONFIG_H */
