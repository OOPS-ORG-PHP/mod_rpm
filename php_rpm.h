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
  | Author: JoungKyun.Kim <http://www.oops.org>                          |
  +----------------------------------------------------------------------+

  $Id$
*/

#ifndef PHP_RPM_H
#define PHP_RPM_H

#ifdef PHP_WIN32
#define PHP_RPM_API __declspec(dllexport)
#else
#define PHP_RPM_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

extern zend_module_entry rpm_module_entry;
#define rpm_module_ptr &rpm_module_entry

PHP_MINIT_FUNCTION(rpm);
PHP_MSHUTDOWN_FUNCTION(rpm);
PHP_RINIT_FUNCTION(rpm);
PHP_RSHUTDOWN_FUNCTION(rpm);
PHP_MINFO_FUNCTION(rpm);

PHP_FUNCTION(rpmversion);
PHP_FUNCTION(rpmbuildno);
PHP_FUNCTION(rpmexec);
PHP_FUNCTION(rpmquery);
PHP_FUNCTION(rpmqi);
PHP_FUNCTION(rpmdep);
PHP_FUNCTION(rpmfind);
PHP_FUNCTION(rpmlist);
PHP_FUNCTION(rpmplist);
PHP_FUNCTION(rpmverify);
PHP_FUNCTION(rpmsig);
PHP_FUNCTION(rpminstall);
PHP_FUNCTION(rpmerase);
PHP_FUNCTION(rpmlastmsg);

/* 
  	Declare any global variables you may need between the BEGIN
	and END macros here:     

ZEND_BEGIN_MODULE_GLOBALS(rpm)
	int   global_value;
	char *global_string;
ZEND_END_MODULE_GLOBALS(rpm)
*/

/* In every utility function you add that needs to use variables 
   in php_rpm_globals, call TSRM_FETCH(); after declaring other 
   variables used by that function, or better yet, pass in TSRMG_CC
   after the last function argument and declare your utility function
   with TSRMG_DC after the last declared argument.  Always refer to
   the globals in your function as RPM_G(variable).  You are 
   encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/

#ifdef ZTS
#define RPM_G(v) TSRMG(rpm_globals_id, zend_rpm_globals *, v)
#else
#define RPM_G(v) (rpm_globals.v)
#endif

#define phpext_rpm_ptr rpm_module_ptr

#define BUILDVER "1.0.0"
#define RPMVER "4.0.4"

#include <defext.h>

#define QUERYINFO "NAME:%{NAME}\1VERSION:%{VERSION}\1" \
				  "RELEASE:%{RELEASE}\1EPOCH:%{EPOCH}\1" \
				  "GROUP:%{GROUP}\1SIZE:%{SIZE}\1" \
                  "SUMMARY:%{SUMMARY}\1DESCRIPTION:%{DESCRIPTION}"

#define QUERYFORM "%{NAME}%{VERSION}%{RELEASE}%{EPOCH}" \
				  "%{GROUP}%{SIZE}%{SUMMARY}%{DESCRIPTION}"

#ifdef PHP_RPM_C
char lastmsg[1024] = { 0, };
#else
extern char lastmsg[1024];
#endif

int __rpmexec (int argc, char ** argv);
int __rpmquery (int argc, char ** argv);
void rpm_last_message_clear (void);

PHPAPI static int _rpmexec (INTERNAL_FUNCTION_PARAMETERS, zend_bool is_query);
PHPAPI void parse_return_array (zval *, zval *);
PHPAPI int check_localpackage (char *);
PHPAPI char * parse_queryformat (char *);

#endif	/* PHP_RPM_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
