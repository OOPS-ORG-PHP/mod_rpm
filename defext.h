#ifndef DEPERROR_H
#define DEPERROR_H

#ifndef RPM_SUCCESS
#define RPM_SUCCESS 0
#endif

#ifndef RPM_FAILURE
#define RPM_FAILURE  1
#endif

#define RPM_WRONGOPT 1001
#define RPM_NOPKG    1002

#define SAFE_FREE(p) { if(p) { efree((p)); (p)=NULL; } }


#endif	/* DEPERROR_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
