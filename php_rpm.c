/*
  +----------------------------------------------------------------------+
  | PHP Version 4                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2002 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 2.02 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available at through the world-wide-web at                           |
  | http://www.php.net/license/2_02.txt.                                 |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: JoungKyun Kim <http://www.oops.org>                          |
  +----------------------------------------------------------------------+

  $Id: php_rpm.c,v 1.1.1.1 2005-03-28 12:10:34 oops Exp $
*/

/*
 * PHP4 Korean String modue "rpm" - only rpm
 */

#ifndef PHP_RPM_C
#define PHP_RPM_C
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include "php.h"
#include "php_ini.h"
#include "SAPI.h"
#include "ext/standard/info.h"

#include "php_rpm.h"

/* If you declare any globals in php_rpm.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(rpm)
*/

/* True global resources - no need for thread safety here */
static int le_rpm;

/* {{{ rpm_functions[]
 *
 * Every user visible function must have an entry in rpm_functions[].
 */
function_entry rpm_functions[] = {
	PHP_FE(rpmversion,		NULL)
	PHP_FE(rpmbuildno,		NULL)
	/*
	PHP_FE(rpmexec,			NULL)
	PHP_FE(rpmquery,		NULL)
	*/
	PHP_FE(rpmqi,			NULL)
	PHP_FE(rpmdep,			NULL)
	PHP_FE(rpmfind,			NULL)
	PHP_FE(rpmlist,			NULL)
	PHP_FE(rpmplist,		NULL)
	PHP_FE(rpmverify,		NULL)
	PHP_FE(rpmsig,			NULL)
	PHP_FE(rpminstall,		NULL)
	PHP_FE(rpmerase,		NULL)
	PHP_FE(rpmlastmsg,		NULL)
	{NULL, NULL, NULL}
};
/* }}} */

/* {{{ rpm_module_entry
 */
zend_module_entry rpm_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"rpm",
	rpm_functions,
	PHP_MINIT(rpm),
	PHP_MSHUTDOWN(rpm),
	NULL,
	NULL,
	PHP_MINFO(rpm),
#if ZEND_MODULE_API_NO >= 20010901
	BUILDVER, /* Replace with version number for your extension */
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_RPM
ZEND_GET_MODULE(rpm)
#endif

/* {{{ PHP_MINIT_FUNCTOIN */
PHP_MINIT_FUNCTION(rpm)
{
	/* If you have INI entries, uncomment these lines
	ZEND_INIT_MODULE_GLOBALS(rpm, php_rpm_init_globals, NULL);
	REGISTER_INI_ENTRIES();
	*/

	REGISTER_LONG_CONSTANT("RPM_SUCCESS", RPM_SUCCESS, CONST_PERSISTENT | CONST_CS);
	REGISTER_LONG_CONSTANT("RPM_FAILURE", RPM_FAILURE, CONST_PERSISTENT | CONST_CS);

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(rpm)
{
	/* uncomment this line if you have INI entries
	 *	UNREGISTER_INI_ENTRIES();
	 */
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(rpm)
{
	php_info_print_table_start();
	php_info_print_table_colspan_header(2, "RPM extension support");
	php_info_print_table_row(2, "RPM versoin", RPMVER);
	php_info_print_table_row(2, "Build version", BUILDVER);
	php_info_print_table_row(2, "URL", "http://devel.oops.org/");
	php_info_print_table_end();
}
/* }}} */

/* {{{ PHPAPI static int _rpmexec (INTERNAL_FUNCTION_PARAMETERS, zend_bool is_query)
PHPAPI static int _rpmexec (INTERNAL_FUNCTION_PARAMETERS, zend_bool is_query)
{
	zval	** __package, ** __args, ** __verbose;
	int		   argc, oargc, pargc, i;
	int		   dynamic_start;
	int		   dynamic_end;
	char	** argv;
	char	 * package;
	int		   res;
	int		   rpmk = 0;

	argc = ZEND_NUM_ARGS();
	switch ( argc ) {
		case 2:
			if ( zend_get_parameters_ex (argc, &__package, &__args) == FAILURE )
				WRONG_PARAM_COUNT;
			break;
		case 1:
			if ( zend_get_parameters_ex (argc, &__package) == FAILURE )
				WRONG_PARAM_COUNT;
			break;
		default:
				WRONG_PARAM_COUNT;
	}

	if ( Z_TYPE_PP (__package) == IS_ARRAY || Z_TYPE_PP (__package) == IS_OBJECT ) {
		pargc = zend_hash_num_elements (Z_ARRVAL_PP(__package));
		if ( pargc < 1 )
			php_error (E_WARNING, "first argument don't have any array value");

		convert_to_array_ex (__package);
	} else {
		if ( Z_STRLEN_PP (__package) < 1 )
			php_error (E_WARNING, "first argument don't hava any values");

		pargc = 1;
		convert_to_string_ex (__package);
	}

	if ( Z_TYPE_PP (__args) == IS_ARRAY || Z_TYPE_PP (__args) == IS_OBJECT ) {
		oargc = zend_hash_num_elements (Z_ARRVAL_PP(__args));
		if ( oargc < 1 )
			php_error (E_WARNING, "second argument don't have any array value");

		convert_to_array_ex (__args);
	} else {
		if ( Z_STRLEN_PP (__args) < 1 ) {
			php_error (E_WARNING, "second argument don't hava any values");
		} else {
			oargc = 1;
			convert_to_string_ex (__args);
		}
	}

	argc = oargc + pargc + 1;
	argv = (char **) emalloc (argc * sizeof (char *));
	argv[0] = ( is_query ) ? "rpmq" : "rpm";

	dynamic_start = 1;

	//
	// option parsing
	//
	if ( Z_TYPE_PP (__args) == IS_ARRAY || Z_TYPE_PP (__args) == IS_OBJECT ) {
		HashTable		* _array;
		HashPosition	pos;

		_array = Z_ARRVAL_PP (__args);
		zend_hash_internal_pointer_reset_ex (_array, &pos);

		for ( i=dynamic_start; i<oargc+dynamic_start; i++ ) {
			zval ** dataptr;

			if ( zend_hash_get_current_data_ex (_array, (void **) &dataptr, &pos) == FAILURE )
				continue;

			if ( Z_TYPE_PP (dataptr) != IS_STRING )
				convert_to_string_ex (dataptr);

			argv[i] = estrdup (Z_STRVAL_PP(dataptr));

			if ( ! strcmp (argv[i], "-K") )
				rpmk++;

			zend_hash_move_forward_ex (_array, &pos);
		}

		dynamic_end = i;
	} else {
		argv[dynamic_start] = estrdup (Z_STRVAL_PP (__args));

		if ( ! strcmp (argv[dynamic_start], "-K") )
			rpmk++;

		dynamic_end = dynamic_start + 1;
	}

	if ( oargc == 1 && rpmk )
		argv[0] = "rpmk";
	else rpmk = 0;

	//
	// package argument parseing
	//
	if ( Z_TYPE_PP (__package) == IS_ARRAY || Z_TYPE_PP (__package) == IS_OBJECT ) {
		HashTable		* _p_array;
		HashPosition	  p_pos;
		int				  dynamic_pend;

		convert_to_array_ex (__package);
		_p_array = Z_ARRVAL_PP (__package);
		zend_hash_internal_pointer_reset_ex (_p_array, &p_pos);

		dynamic_pend = dynamic_end;
		for ( i=dynamic_pend; i<pargc+dynamic_pend; i++ ) {
			zval ** dataptr;

			if ( zend_hash_get_current_data_ex (_p_array, (void **) &dataptr, &p_pos) == FAILURE )
				continue;

			if ( Z_TYPE_PP (dataptr) != IS_STRING )
				convert_to_string_ex (dataptr);

			argv[i] = estrdup (Z_STRVAL_PP (dataptr));
			zend_hash_move_forward_ex (_p_array, &p_pos);
		}

		dynamic_end = i;
	} else {
		argv[dynamic_end] = estrdup (Z_STRVAL_PP (__package));
		dynamic_end++;
	}

	// argv debuging
	// for ( i=0; i<dynamic_end; i++)
	// 	printf ("%-2d. %s\n", i, argv[i]);

	rpm_last_message_clear ();
	if ( is_query )
		res = __rpmquery (argc, argv);
	else
		res = __rpmexec (argc, argv);

	RETVAL_LONG (res);

	for ( i=1; i<dynamic_end; i++ )
		SAFE_FREE (argv[i]);

	SAFE_FREE (argv);
}
}}} */

/* {{{ proto unsigned char rpmversion(void)
 *  print rpm extension build number */
PHP_FUNCTION(rpmversion)
{
	if ( ZEND_NUM_ARGS() > 0 )
		WRONG_PARAM_COUNT;

	RETURN_STRING (RPMVER,1);
}
/* }}} */

/* {{{ proto unsigned char rpmbuildno(void)
 *  print rpm extension version */
PHP_FUNCTION(rpmbuildno)
{
	if ( ZEND_NUM_ARGS() > 0 )
		WRONG_PARAM_COUNT;

	RETURN_STRING (BUILDVER,1);
}
/* }}} */

/* {{{ proto int rpmexec (string package, array option, int vervose)
 *  print rpm extension version
PHP_FUNCTION(rpmexec)
{
	_rpmexec (INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
}
}}} */

/* {{{ proto int rpmquery (string package, array option, int verbose)
 *  print rpm query extension version
PHP_FUNCTION(rpmquery)
{
	_rpmexec (INTERNAL_FUNCTION_PARAM_PASSTHRU, 1);
}
}}} */

/* {{{ proto mixed rpmqi (string package, string queryformat = "") */
PHP_FUNCTION(rpmqi)
{
	zval	** __package, ** __queryformat;
	zval	 * return_output;
	int		   argc, lpkg, i, qident;
	int		   res;
	char	 * package;
	char	 * qf;
	char	** argv;

	res		= 0;
	argc	= 0;
	lpkg	= 0;
	qf		= NULL;
	qident	= 0;

	argc = ZEND_NUM_ARGS();
	switch ( argc ) {
		case 2:
			if ( zend_get_parameters_ex (argc, &__package, &__queryformat) == FAILURE )
				WRONG_PARAM_COUNT;

			if ( Z_STRLEN_PP (__queryformat) > 1 ) {
				convert_to_string_ex (__queryformat);
				qf = parse_queryformat (Z_STRVAL_PP (__queryformat));
			}
			break;
		case 1:
			if ( zend_get_parameters_ex (argc, &__package) == FAILURE )
				WRONG_PARAM_COUNT;
			break;
		default:
			WRONG_PARAM_COUNT;
	}

	if ( Z_STRLEN_PP (__package) < 1 ) {
		memset (lastmsg, 0, 1024);
		strcpy (lastmsg, "first argument don't hasv any value");
		RETURN_LONG (RPM_FAILURE);
	}

	convert_to_string_ex (__package);
	package = Z_STRVAL_PP (__package);
	lpkg = check_localpackage (package);

	if ( qf == NULL )
		qf = parse_queryformat (NULL);
	else {
		int qlen = strlen (qf);

		for ( i=0; i<qlen; i++ )
			if ( qf[i] == '\1' )
				qident++;
	}

	argv = emalloc (sizeof (char *) * 6);
	argv[0] = estrdup ("rpmq");
	argv[1] = lpkg ? estrdup ("-qp") : estrdup ("-q");
	argv[2] = estrdup ("--queryformat");
	argv[3] = estrdup (qf);
	argv[4] = estrdup (package);
	argv[5] = NULL;

	SAFE_FREE (qf);
	argc = 5;

	/* for debugging
	for (i=0; i<argc; i++)
		php_printf ("%-2d. %s\n", i, argv[i]);
	*/

	rpm_last_message_clear ();
	php_start_ob_buffer (NULL, 0, 1 TSRMLS_CC);
	if ( res = __rpmquery (argc, argv) != RPM_SUCCESS ) {
		RETVAL_LONG (RPM_FAILURE);
		goto end;
	}

	ALLOC_INIT_ZVAL (return_output);
	php_ob_get_buffer (return_output TSRMLS_CC);
	php_end_ob_buffer (0, 0 TSRMLS_CC);

	if ( qident == 1 ) {
		char * return_string;
		char * get_string;
		get_string = Z_STRVAL_PP (&return_output);
		return_string = strchr (get_string, ':');

		RETVAL_STRING (return_string != NULL ? return_string + 1 : get_string, 1);
	} else
		parse_return_array (return_value, return_output);

	FREE_ZVAL (return_output);

end:
	for ( i=0; i<argc; i++ )
		SAFE_FREE (argv[i]);

	SAFE_FREE (argv);
}
/* }}} */

/* {{{ proto mixed rpmdep (string package) */
PHP_FUNCTION(rpmdep)
{
	zval	** __package;
	zval	 * return_output;
	int		   argc, lpkg, i;
	int		   res;
	char	 * package;
	char	** argv;

	res		= 0;
	argc	= 0;
	lpkg	= 0;

	argc = ZEND_NUM_ARGS();
	switch ( argc ) {
		case 1:
			if ( zend_get_parameters_ex (argc, &__package) == FAILURE )
				WRONG_PARAM_COUNT;
			break;
		default:
			WRONG_PARAM_COUNT;
	}

	if ( Z_STRLEN_PP (__package) < 1 ) {
		memset (lastmsg, 0, 1024);
		strcpy (lastmsg, "first argument don't hasv any value");
		RETURN_LONG (RPM_FAILURE);
	}

	convert_to_string_ex (__package);
	package = Z_STRVAL_PP (__package);
	lpkg = check_localpackage (package);

	argv = emalloc (sizeof (char *) * 4);
	argv[0] = estrdup ("rpmq");
	argv[1] = lpkg ? estrdup ("-Rp") : estrdup ("-R");
	argv[2] = estrdup (package);
	argv[3] = NULL;

	argc = 3;

	/* for debugging
	for (i=0; i<argc; i++)
		php_printf ("%-2d. %s\n", i, argv[i]);
	*/

	rpm_last_message_clear ();
	php_start_ob_buffer (NULL, 0, 1 TSRMLS_CC);
	if ( res = __rpmquery (argc, argv) != RPM_SUCCESS ) {
		RETVAL_LONG (RPM_FAILURE);
		goto end;
	}

	ALLOC_INIT_ZVAL (return_output);
	php_ob_get_buffer (return_output TSRMLS_CC);
	php_end_ob_buffer (0, 0 TSRMLS_CC);

	{
		char * token, * btoken;
		char * get_output;
		char delimiters[] = "\n";

		array_init (return_value);

		get_output = Z_STRVAL_PP (&return_output);
		token = php_strtok_r (get_output, delimiters, &btoken);
		while ( token ) {
			add_next_index_string (return_value, token, 1);
			token = php_strtok_r (NULL, delimiters, &btoken);
		}
	}

	FREE_ZVAL (return_output);

end:
	for ( i=0; i<argc; i++ )
		SAFE_FREE (argv[i]);

	SAFE_FREE (argv);
}
/* }}} */

/* {{{ proto string rpmfind (string file) */
PHP_FUNCTION(rpmfind)
{
	zval	** __package;
	int		   argc, i;
	int		   res;
	char	 * package;
	char	** argv;

	res		= 0;
	argc	= 0;

	argc = ZEND_NUM_ARGS();
	switch ( argc ) {
		case 1:
			if ( zend_get_parameters_ex (argc, &__package) == FAILURE )
				WRONG_PARAM_COUNT;
			break;
		default:
			WRONG_PARAM_COUNT;
	}

	if ( Z_STRLEN_PP (__package) < 1 ) {
		memset (lastmsg, 0, 1024);
		strcpy (lastmsg, "first argument don't hasv any value");
		RETURN_LONG (RPM_FAILURE);
	}

	convert_to_string_ex (__package);
	package = Z_STRVAL_PP (__package);

	argv = emalloc (sizeof (char *) * 4);
	argv[0] = estrdup ("rpmq");
	argv[1] = estrdup ("-f");
	argv[2] = estrdup (package);
	argv[3] = NULL;

	argc = 3;

	/* for debugging
	for (i=0; i<argc; i++)
		php_printf ("%-2d. %s\n", i, argv[i]);
	*/

	rpm_last_message_clear ();
	php_start_ob_buffer (NULL, 0, 1 TSRMLS_CC);
	if ( res = __rpmquery (argc, argv) != RPM_SUCCESS ) {
		RETVAL_LONG (RPM_FAILURE);
		goto end;
	}

	php_ob_get_buffer (return_value TSRMLS_CC);
	php_end_ob_buffer (0, 0 TSRMLS_CC);

end:
	for ( i=0; i<argc; i++ )
		SAFE_FREE (argv[i]);

	SAFE_FREE (argv);
}
/* }}} */

/* {{{ proto string rpmlist (string package) */
PHP_FUNCTION(rpmlist)
{
	zval	** __package;
	zval	 * return_output;
	int		   argc, i;
	int		   res;
	char	 * package;
	char	 option[4];
	char	** argv;

	res		= 0;
	argc	= 0;
	memset (option, 0, 4);

	argc = ZEND_NUM_ARGS();
	switch ( argc ) {
		case 1:
			if ( zend_get_parameters_ex (argc, &__package) == FAILURE )
				WRONG_PARAM_COUNT;
			break;
		default:
			WRONG_PARAM_COUNT;
	}

	if ( Z_STRLEN_PP (__package) > 0 ) {
		argc = 3;
		strcpy (option, "-q");
		convert_to_string_ex (__package);
		package = Z_STRVAL_PP (__package);
	} else {
		argc = 2;
		strcpy (option, "-qa");
	}

	argv = emalloc (sizeof (char *) * 4);
	argv[0] = estrdup ("rpmq");
	argv[1] = estrdup (option);

	if ( argc == 3 ) {
		argv[2] = estrdup (package);
		argv[3] = NULL;
	} else {
		argv[2] = NULL;
	}

	/* for debugging
	for (i=0; i<argc; i++)
		php_printf ("%-2d. %s\n", i, argv[i]);
	*/

	rpm_last_message_clear ();
	php_start_ob_buffer (NULL, 0, 1 TSRMLS_CC);
	if ( res = __rpmquery (argc, argv) != RPM_SUCCESS ) {
		RETVAL_LONG (RPM_FAILURE);
		goto end;
	}

	ALLOC_INIT_ZVAL (return_output);
	php_ob_get_buffer (return_output TSRMLS_CC);
	php_end_ob_buffer (0, 0 TSRMLS_CC);

	if ( argc == 3 ) {
		RETVAL_LONG (RPM_SUCCESS);
	} else {
		char * token, * btoken;
		char * get_output;
		char delimiters[] = "\n";

		array_init (return_value);

		get_output = Z_STRVAL_PP (&return_output);
		token = php_strtok_r (get_output, delimiters, &btoken);
		while ( token ) {
			add_next_index_string (return_value, token, 1);
			token = php_strtok_r (NULL, delimiters, &btoken);
		}
	}

end:
	for ( i=0; i<argc; i++ )
		SAFE_FREE (argv[i]);

	SAFE_FREE (argv);
}
/* }}} */

/* {{{ proto array rpmplist (string package, string option) */
PHP_FUNCTION(rpmplist)
{
	zval	** __package, ** __option;
	zval	 * return_output;
	int		   argc, lpkg, i;
	int		   res;
	char	 * package;
	char	 * option;
	char	 opt[32];
	char	** argv;

	res		= 0;
	argc	= 0;
	lpkg	= 0;
	option	= NULL;
	memset (opt, 0, 32);

	argc = ZEND_NUM_ARGS();
	switch ( argc ) {
		case 2:
			if ( zend_get_parameters_ex (argc, &__package, &__option) == FAILURE )
				WRONG_PARAM_COUNT;

			if ( Z_STRLEN_PP (__option) > 1 ) {
				convert_to_string_ex (__option);
				option = Z_STRVAL_PP (__option);

				if ( ! strcmp (option, "conf") )
					option = "c";
				else if ( ! strcmp (option, "doc") )
					option = "d";
				else {
					memset (lastmsg, 0, 1024);
					sprintf (lastmsg, "second argument \"%s\" is not recognized. "
									  "set please \"doc\" or \"conf\" or blanked.",
							 option);
					RETURN_LONG (RPM_FAILURE);
				}
			}
			break;
		case 1:
			if ( zend_get_parameters_ex (argc, &__package) == FAILURE )
				WRONG_PARAM_COUNT;
			break;
		default:
			WRONG_PARAM_COUNT;
	}

	if ( Z_STRLEN_PP (__package) < 1 ) {
		memset (lastmsg, 0, 1024);
		strcpy (lastmsg, "first argument don't hasv any value");
		RETURN_LONG (RPM_FAILURE);
	}

	convert_to_string_ex (__package);
	package = Z_STRVAL_PP (__package);
	lpkg = check_localpackage (package);

	sprintf (opt, "-lv%s%s", lpkg ? "p" : "", option ? option : "");

	argv = emalloc (sizeof (char *) * 4);
	argv[0] = estrdup ("rpmq");
	argv[1] = estrdup (opt);
	argv[2] = estrdup (package);
	argv[3] = NULL;

	argc = 3;

	/* for debugging
	for (i=0; i<argc; i++)
		php_printf ("%-2d. %s\n", i, argv[i]);
	*/

	rpm_last_message_clear ();
	php_start_ob_buffer (NULL, 0, 1 TSRMLS_CC);
	if ( res = __rpmquery (argc, argv) != RPM_SUCCESS ) {
		RETVAL_LONG (RPM_FAILURE);
		goto end;
	}

	ALLOC_INIT_ZVAL (return_output);
	php_ob_get_buffer (return_output TSRMLS_CC);
	php_end_ob_buffer (0, 0 TSRMLS_CC);

	{
		char * token, * btoken;
		char * get_output;
		char delimiters[] = "\n";
		int  key_num = 0;

		array_init (return_value);

		get_output = Z_STRVAL_PP (&return_output);
		token = php_strtok_r (get_output, delimiters, &btoken);
		while ( token ) {
			char * tok, * btok;
			char   deli[] = " ";
			zval * obj;

			MAKE_STD_ZVAL (obj);
			object_init (obj);

			//add_next_index_string (return_value, token, 1);

			tok = php_strtok_r (token, deli, &btok);
			i = 0;
			while ( tok ) {
				char month[16] = { 0, };
				char days[4] = { 0, };
				char ext[16] = { 0, };
				char strs[64] = { 0, };

				switch (i++) {
					case 0 :
						add_property_string (obj, "perm", tok, 1);
						//printf ("### perm  : %s\n", tok);
						break;
					case 2 :
						add_property_string (obj, "owner", tok, 1);
						//printf ("### owner : %s\n", tok);
						break;
					case 3 :
						add_property_string (obj, "group", tok, 1);
						//printf ("### group : %s\n", tok);
						break;
					case 4 :
						add_property_string (obj, "size", tok, 1);
						//printf ("### size  : %s\n", tok);
						break;
					case 5 :
						strcpy (month, tok);
						i++;

						tok = php_strtok_r (NULL, deli, &btok);
						strcpy (days, tok);
						i++;

						tok = php_strtok_r (NULL, deli, &btok);
						strcpy (ext, tok);

						sprintf (strs, "%s %s %s", month, days, ext);
						add_property_string (obj, "date", strs, 1);
						//printf ("### date  : %s\n", tok);
						break;
					case 8 :
						add_property_string (obj, "name", tok, 1);
						//printf ("### name  : %s\n", tok);
						break;
				}
				tok = php_strtok_r (NULL, deli, &btok);
			}
			zend_hash_index_update (return_value->value.ht, key_num++, (void *) &obj, sizeof (zval *), NULL);

			token = php_strtok_r (NULL, delimiters, &btoken);
		}
	}

	FREE_ZVAL (return_output);

end:
	for ( i=0; i<argc; i++ )
		SAFE_FREE (argv[i]);

	SAFE_FREE (argv);
}
/* }}} */

/* {{{ proto array rpmverfy (string package) */
PHP_FUNCTION(rpmverify)
{
	zval	** __package;
	zval	 * return_output;
	int		   argc, i;
	int		   res;
	char	 * package;
	char	** argv;

	res		= 0;
	argc	= 0;

	argc = ZEND_NUM_ARGS();
	switch ( argc ) {
		case 1:
			if ( zend_get_parameters_ex (argc, &__package) == FAILURE )
				WRONG_PARAM_COUNT;
			break;
		default:
			WRONG_PARAM_COUNT;
	}

	if ( Z_STRLEN_PP (__package) < 1 ) {
		memset (lastmsg, 0, 1024);
		strcpy (lastmsg, "first argument don't hasv any value");
		RETURN_LONG (RPM_FAILURE);
	}

	convert_to_string_ex (__package);
	package = Z_STRVAL_PP (__package);

	argv = emalloc (sizeof (char *) * 4);
	argv[0] = estrdup ("rpmq");
	argv[1] = estrdup ("-V");
	argv[2] = estrdup (package);
	argv[3] = NULL;

	argc = 3;

	/* for debugging
	for (i=0; i<argc; i++)
		php_printf ("%-2d. %s\n", i, argv[i]);
	*/

	rpm_last_message_clear ();
	php_start_ob_buffer (NULL, 0, 1 TSRMLS_CC);
	res = __rpmquery (argc, argv);

	if ( res == RPM_SUCCESS ) {
		RETVAL_LONG (RPM_SUCCESS);
		goto end;
	} else if ( res == RPM_FAILURE ) {
		RETVAL_LONG (RPM_FAILURE);
		goto end;
	}

	ALLOC_INIT_ZVAL (return_output);
	php_ob_get_buffer (return_output TSRMLS_CC);
	php_end_ob_buffer (0, 0 TSRMLS_CC);

	{
		char	* token, * btoken;
		char	* get_output;
		char	  delimiters[] = "\n";
		int		  key_num = 0;
		int		  j;
		char	* key_name[8] = { "size", "mode", "md5", "rdev",
								  "link", "user", "group", "mtime" };

		array_init (return_value);

		get_output = Z_STRVAL_PP (&return_output);
		token = php_strtok_r (get_output, delimiters, &btoken);
		while ( token ) {
			char * tok, * btok;
			char   deli[] = " ";
			zval * obj;

			MAKE_STD_ZVAL (obj);
			object_init (obj);

			tok = php_strtok_r (token, deli, &btok);
			i = 0;
			while ( tok ) {
				switch (i++) {
					case 0 :
						if ( strcmp (tok, "missing") ) {
							for ( j=0; j<8; j++ ) {
								if ( tok[j] != '.' )
									add_property_long (obj, key_name[j], 1);
								else
									add_property_long (obj, key_name[j], 0);
							}
							add_property_long (obj, "missing", 0);
						} else {
							for ( j=0; j<8; j++ )
								add_property_long (obj, key_name[j], 0);
							add_property_long (obj, "missing", 1);
						}
						break;
					case 1 :
						switch ( tok[0] ) {
							case 'c' :
								add_property_string (obj, "type", "conf", 1);
								break;
							case 'd' :
								add_property_string (obj, "type", "doc", 1);
								break;
							case 'g' :
								add_property_string (obj, "type", "ghost", 1);
								break;
							case 'l' :
								add_property_string (obj, "type", "license", 1);
								break;
							case 'r' :
								add_property_string (obj, "type", "readme", 1);
								break;
							default:
								add_property_string (obj, "type", "general", 1);

						}
						break;
					case 2 :
						add_property_string (obj, "name", tok, 1);
						break;
				}
				tok = php_strtok_r (NULL, deli, &btok);
			}
			zend_hash_index_update (return_value->value.ht, key_num++, (void *) &obj, sizeof (zval *), NULL);

			token = php_strtok_r (NULL, delimiters, &btoken);
		}
	}

	FREE_ZVAL (return_output);

end:
	for ( i=0; i<argc; i++ )
		SAFE_FREE (argv[i]);

	SAFE_FREE (argv);
}
/* }}} */

/* {{{ proto mixed rpmsig (string package) */
PHP_FUNCTION(rpmsig)
{
	zval	** __package;
	zval	 * return_output;
	int		   argc, res, i;
	char	 * package;
	char	** argv;
	char	 * get_output;

	res		= 0;

	argc = ZEND_NUM_ARGS();
	switch ( argc ) {
		case 1:
			if ( zend_get_parameters_ex (argc, &__package) == FAILURE )
				WRONG_PARAM_COUNT;
			break;
		default:
			WRONG_PARAM_COUNT;
	}

	if ( Z_STRLEN_PP (__package) < 1 ) {
		memset (lastmsg, 0, 1024);
		strcpy (lastmsg, "first argument don't hasv any value");
		RETURN_LONG (RPM_FAILURE);
	}

	convert_to_string_ex (__package);
	package = Z_STRVAL_PP (__package);

	argv = emalloc (sizeof (char *) * 4);
	argv[0] = estrdup ("rpmk");
	argv[1] = estrdup ("-K");
	argv[2] = estrdup (package);
	argv[3] = NULL;

	argc = 3;

	/* for debugging
	for (i=0; i<6; i++)
		php_printf ("%-2d. %s\n", i, argv[i]);
	*/

	rpm_last_message_clear ();
	php_start_ob_buffer (NULL, 0, 1 TSRMLS_CC);
	if ( ( res = __rpmquery (argc, argv) ) != RPM_SUCCESS ) {
		RETVAL_LONG (RPM_FAILURE);
	}

	ALLOC_INIT_ZVAL (return_output);
	php_ob_get_buffer (return_output TSRMLS_CC);
	php_end_ob_buffer (0, 0 TSRMLS_CC);

	if ( res == RPM_FAILURE ) {
		if ( Z_STRLEN_PP (&return_output) ) {
			get_output = Z_STRVAL_PP (&return_output);
			memset (lastmsg, 0, 1024);
			strcpy (lastmsg, get_output);
		}
	} else {
		char * filepath, * filename;
		char * get_value;
		char * checkpt;
		char * token, *btoken;
		char delimiters[] = " ";

		get_output = Z_STRVAL_PP (&return_output);

		filepath = estrdup (get_output);
		checkpt = strchr (filepath, ':');

		if ( checkpt == NULL ) {
			memset (lastmsg, 0, 1024);
			sprintf (lastmsg, "Return value is follown\n"
							  "-----------------\n"
							  "%s\n"
							  "-----------------\n"
							  "This value is malformed", get_output);

			RETVAL_LONG (RPM_FAILURE);
			goto end;
		}

		get_value = estrdup (checkpt + 1);
		*checkpt = '\0';

		filename = estrdup (filepath);
		checkpt = strrchr (filename, '/');

		array_init (return_value);
		add_assoc_string (return_value, "name", checkpt != NULL ? checkpt + 1 : filename, 1);
		add_assoc_long (return_value, "md5", 0);
		add_assoc_long (return_value, "gpg", 0);

		SAFE_FREE (filepath);
		SAFE_FREE (filename);

		token = php_strtok_r (get_output, delimiters, &btoken);
		while ( token ) {
			if ( ! strcmp (token, "md5") ) {
				//add_assoc_unset (return_value, "md5");
				add_assoc_long (return_value, "md5", 1);
			} else if ( ! strcmp (token, "gpg") ) {
				//add_assoc_unset (return_value, "gpg");
				add_assoc_long (return_value, "gpg", 1);
			}

			token = php_strtok_r (NULL, delimiters, &btoken);
		}
	}

end:
	for ( i=0; i<5; i++ )
		SAFE_FREE (argv[i]);

	SAFE_FREE (argv);
}
/* }}} */

/* {{{ proto int rpminstall (string package, string mode, string or array option) */
PHP_FUNCTION(rpminstall)
{
	zval	** __package, ** __mode, ** __option;
	char	** argv;
	char	 * package, * mode, modebuf[2] = { 0, };
	int		   argc, oargc, i, res;

	mode	= NULL;
	oargc	= 0;

	argc = ZEND_NUM_ARGS();
	switch ( argc ) {
		case 3:
			if ( zend_get_parameters_ex (argc, &__package, &__mode, &__option) == FAILURE )
				WRONG_PARAM_COUNT;
			break;
		case 2:
			if ( zend_get_parameters_ex (argc, &__package, &__mode) == FAILURE )
				WRONG_PARAM_COUNT;
			break;
		case 1:
			if ( zend_get_parameters_ex (argc, &__package) == FAILURE )
				WRONG_PARAM_COUNT;
			break;
		default:
			WRONG_PARAM_COUNT;
	}

	if ( Z_STRLEN_PP (__package) < 1 ) {
		memset (lastmsg, 0, 1024);
		strcpy (lastmsg, "first argument don't have any value");
		RETURN_LONG (RPM_FAILURE);
	}

	convert_to_string_ex (__package);
	package = Z_STRVAL_PP (__package);

	if ( argc > 1 ) {
		if ( Z_STRLEN_PP (__mode) > 0 ) {
			convert_to_string_ex (__mode);
			mode = Z_STRVAL_PP (__mode);

			if ( strcmp (mode, "i") && strcmp (mode, "F") && strcmp (mode, "U") ) {
				memset (lastmsg, 0, 1024);
				strcpy (lastmsg, "second arguement must be one of i or e or u");
				RETURN_LONG (RPM_FAILURE);
			}
		}
	}

	if ( mode == NULL )
		mode = "i";

	if ( Z_TYPE_PP (__option) == IS_ARRAY ) {
		oargc = zend_hash_num_elements (Z_ARRVAL_PP (__option));
		if ( oargc > 0 ) {
			convert_to_array_ex (__option);
		}
	} else {
		if ( Z_STRLEN_PP (__option) > 1 ) {
			convert_to_string_ex (__option);
			oargc = 1;
		}
	}


	argc = oargc + 3;
	argv = (char **) emalloc (sizeof (char *) * argc);

	if ( ! strcmp (mode, "i") )
		argv[0] = estrdup ("rpmi");
	else
		argv[0] = estrdup ("rpmu");

	sprintf (modebuf, "-%s", mode);
	argv[1] = estrdup (modebuf);

	if ( oargc > 0 ) {
		if ( Z_TYPE_PP (__option) == IS_ARRAY ) {
			HashTable		* _array;
			HashPosition	  pos;
			int				  no;

			_array = Z_ARRVAL_PP (__option);
			zend_hash_internal_pointer_reset_ex (_array, &pos);

			no = 0;
			for ( i=2; i<oargc+2; i++ ) {
				zval ** dataptr;

				if ( zend_hash_get_current_data_ex (_array, (void **) &dataptr, &pos) == FAILURE ) {
					zend_hash_move_forward_ex (_array, &pos);
					continue;
				}

				if ( Z_TYPE_PP (dataptr) != IS_STRING )
					convert_to_string_ex (dataptr);

				argv[i] = estrdup (Z_STRVAL_PP (dataptr));
				zend_hash_move_forward_ex (_array, &pos);
				no++;
			}
		} else {
			argv[2] = estrdup (Z_STRVAL_PP (__option));
		}
	}
	argv[argc-1] = estrdup (package);

	/* for debugging
	for ( i=0; i<argc; i++ )
		php_printf ("## argv[%d] : %s\n", i, argv[i]);
	*/

	rpm_last_message_clear ();
	if ( (res = __rpmquery (argc, argv)) != RPM_SUCCESS) {
		RETVAL_LONG (RPM_FAILURE);
	} else {
		RETVAL_LONG (RPM_SUCCESS);
	}

	for ( i=0; i<argc; i++ )
		SAFE_FREE (argv[i]);

	SAFE_FREE (argv);
}
/* }}} */

/* {{{ proto int rpmerase (string package, strint or array option) */
PHP_FUNCTION(rpmerase)
{
	zval	** __package, ** __option;
	char	** argv;
	char	 * package;
	int		   argc, oargc, i, res;

	oargc	= 0;

	argc = ZEND_NUM_ARGS();
	switch ( argc ) {
		case 2:
			if ( zend_get_parameters_ex (argc, &__package, &__option) == FAILURE )
				WRONG_PARAM_COUNT;
			break;
		case 1:
			if ( zend_get_parameters_ex (argc, &__package) == FAILURE )
				WRONG_PARAM_COUNT;
			break;
		default:
			WRONG_PARAM_COUNT;
	}

	if ( Z_STRLEN_PP (__package) < 1 ) {
		memset (lastmsg, 0, 1024);
		strcpy (lastmsg, "first argument don't have any value");
		RETURN_LONG (RPM_FAILURE);
	}

	convert_to_string_ex (__package);
	package = Z_STRVAL_PP (__package);

	if ( Z_TYPE_PP (__option) == IS_ARRAY ) {
		oargc = zend_hash_num_elements (Z_ARRVAL_PP (__option));
		if ( oargc > 0 ) {
			convert_to_array_ex (__option);
		}
	} else {
		if ( Z_STRLEN_PP (__option) > 1 ) {
			convert_to_string_ex (__option);
			oargc = 1;
		}
	}

	argc = oargc + 3;
	argv = (char **) emalloc (sizeof (char *) * argc);

	argv[0] = estrdup ("rpme");
	argv[1] = estrdup ("-e");

	if ( oargc > 0 ) {
		if ( Z_TYPE_PP (__option) == IS_ARRAY ) {
			HashTable		* _array;
			HashPosition	  pos;
			int				  no;

			_array = Z_ARRVAL_PP (__option);
			zend_hash_internal_pointer_reset_ex (_array, &pos);

			no = 0;
			for ( i=2; i<oargc+2; i++ ) {
				zval ** dataptr;

				if ( zend_hash_get_current_data_ex (_array, (void **) &dataptr, &pos) == FAILURE ) {
					zend_hash_move_forward_ex (_array, &pos);
					continue;
				}

				if ( Z_TYPE_PP (dataptr) != IS_STRING )
					convert_to_string_ex (dataptr);

				argv[i] = estrdup (Z_STRVAL_PP (dataptr));
				zend_hash_move_forward_ex (_array, &pos);
				no++;
			}
		} else {
			argv[2] = estrdup (Z_STRVAL_PP (__option));
		}
	}
	argv[argc-1] = estrdup (package);

	/* for debugging */
	for ( i=0; i<argc; i++ )
		php_printf ("## argv[%d] : %s\n", i, argv[i]);
	/**/

	rpm_last_message_clear ();
	if ( (res = __rpmquery (argc, argv)) != RPM_SUCCESS) {
		RETVAL_LONG (RPM_FAILURE);
	} else {
		RETVAL_LONG (RPM_SUCCESS);
	}

	for ( i=0; i<argc; i++ )
		SAFE_FREE (argv[i]);

	SAFE_FREE (argv);
}
/* }}} */

/* {{{ proto int rpmlastmsg (void)
 *  print rpm last message */
PHP_FUNCTION(rpmlastmsg)
{
	int len, i;
	int endmark = 0;

	if ( ZEND_NUM_ARGS() > 0 )
		WRONG_PARAM_COUNT;

	len = strlen (lastmsg) - 1;

	for ( i=len; i>-1; i-- ) {
		if ( lastmsg[i] == '\n' || lastmsg[i] == ' ' )
			endmark = i;
		else
			break;
	}

	if ( endmark )
		lastmsg[endmark] = 0;

	RETVAL_STRING (lastmsg, 1);
	memset (lastmsg, 0, 1024);
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
