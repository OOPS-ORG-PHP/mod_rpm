/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include "php.h"
#include "php_ini.h"
#include "SAPI.h"
#include "ext/standard/info.h"

#include "php_rpm.h"

/* {{{ PHPAPI int check_localpackage (char *pkg) */
PHPAPI int check_localpackage (char *pkg) {
	int len;
	char *chk;

	len = strlen (pkg);

	if ( len > 0 ) {
		if ( strchr (pkg, '/') )
			return 1;

		chk = strrchr (pkg, '.');

		if ( chk == NULL )
			return 0;

		if ( ! strcmp (chk, ".rpm") )
			return 1;
	}

	return 0;
}
/* }}} */

/* {{{ PHPAPI char * pares_queryformat (char *qf) */
PHPAPI char * parse_queryformat (char *qf) {
	int		len;
	char	delimiters[] = "\n%";
	char	* qfbuf;
	char	* token, *btoken;
	char	* rvalue;

	rvalue = NULL;

	qfbuf = ( qf == NULL ) ? estrdup (QUERYFORM) : estrdup (qf);
	len = strlen (qfbuf);

	rvalue = emalloc (sizeof (char *) * (len * 3));

	token = php_strtok_r (qfbuf, delimiters, &btoken);
	while ( token ) {
		char * tokenbuf;
		char * tmpdirec;
		char directive[64] = { 0, };

		tmpdirec = strchr (token, '{');

		if ( tmpdirec == NULL )
			goto gotoken;

		tokenbuf = estrdup (tmpdirec + 1);
		len = strlen (tokenbuf);


		tmpdirec = strchr (tokenbuf, '}');
		if ( tmpdirec == NULL )
			goto next;

		*tmpdirec = '\0';
		sprintf (directive, "%s:%%{%s}%c", tokenbuf, tokenbuf, '\1');
		strcat (rvalue, directive);

next:
		SAFE_FREE (tokenbuf);
gotoken:
		token = php_strtok_r (NULL, delimiters, &btoken);
	}

	SAFE_FREE (qfbuf);

	return rvalue;
}
/* }}} */

/* {{{ PHPAPI void parse_returnvalue (zval *value) */
PHPAPI void parse_return_array (zval * rvalue, zval *value) {
	char	* _value;
	char	  delimiters[] = "\1";
	char	* token, *btoken;
	int		  _len;

	_len = Z_STRLEN_PP (&value);

	if ( _len > 0 ) {
		int		  _sublen;
		char	* _name;
		char 	* _values;

		array_init (rvalue);

		token = php_strtok_r ( Z_STRVAL_PP (&value), delimiters, &btoken);
		while ( token ) {
			char	* _tmp;

			_sublen = strlen (token);
			if ( token[_sublen-1] == '\n' ) token[_sublen-1] = 0;

			_name = estrdup (token);
			_tmp = strchr (_name, ':');

			if ( _tmp ) {
				_values = estrdup (_tmp + 1);
				* _tmp = 0;

				add_assoc_string (rvalue, _name, _values, 1);

				/*
				php_printf ("==================================\n");
				php_printf ("### %s => %s\n", _name, _values);
				php_printf ("==================================\n");
				*/

				SAFE_FREE (_name);
				SAFE_FREE (_values);
			}
			token = php_strtok_r (NULL, delimiters, &btoken);
		}
	}
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
