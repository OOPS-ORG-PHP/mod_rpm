#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

/* for PHP */
#include <stdarg.h>
#include "php.h"
#include "SAPI.h"
/* for PHP */

#ifndef va_copy
# ifdef __va_copy
#  define va_copy(DEST,SRC) __va_copy((DEST),(SRC))
# else
#  ifdef HAVE_VA_LIST_AS_ARRAY
#   define va_copy(DEST,SRC) (*(DEST) = *(SRC))
#  else
#   define va_copy(DEST,SRC) ((DEST) = (SRC))
#  endif
# endif
#endif

extern char lastmsg[1024];

int int_num (int x) {
	char s[64] = { 0, };
	sprintf (s, "%d", x);
	return strlen (s);
}

void rpm_last_message_clear (void) {
	memset (lastmsg, 0, 1024);
}

void rpm_last_message (int stream, char *format, ...) {
	char *buf;
	char *p;
	int sum;
	va_list ap, cp;

	if ( stream != 7 ) {
		sum = 0;
		va_start (ap, format);
		va_copy (cp, ap);

		for ( p = format; *p; p++ ) {
			if ( *p == '%' ) {
				p++;
				switch (*p) {
					case 's' :
						sum += strlen (va_arg (cp, char *));
						break;
					case 'd' :
						sum += int_num (va_arg (cp, int));
						break;
					case 'c' :
						(char) va_arg (cp, int);
						sum += 1;
						break;
					default:
						sum += 1;
				}
			} else {
				sum += 1;
			}
		}
		va_end (cp);

		buf = malloc (sizeof (char *) * (sum + 1));
		vsprintf (buf, format, ap);
		va_end (ap);

		if ( sum > 1023 )
			buf[1024] = 0;

		if ( stream == -100 )
			strcat (lastmsg, buf);
		else
			strcpy (lastmsg, buf);

		free (buf);
	}
}

void rpm_printf (int stream, char *format, ...) {
	char *buf;
	char *p;
	int sum;
	va_list ap, cp;

	if ( stream != 7 ) {
		sum = 0;
		va_start (ap, format);
		__va_copy (cp, ap);

		for ( p = format; *p; p++ ) {
			if ( *p == '%' ) {
				p++;
				switch (*p) {
					case 's' :
						sum += strlen (va_arg (cp, char *));
						break;
					case 'd' :
						sum += int_num (va_arg (cp, int));
						break;
					case 'c' :
						(char) va_arg (cp, int);
						sum += 1;
						break;
					default:
						sum += 1;
				}
			} else {
				sum += 1;
			}
		}
		va_end (cp);

		buf = malloc (sizeof (char *) * (sum + 1));
		vsprintf (buf, format, ap);
		va_end (ap);

		php_printf (buf);
		free (buf);
	}
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
