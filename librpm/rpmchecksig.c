/** \ingroup rpmcli
 * \file lib/rpmchecksig.c
 * Verify the signature of a package.
 */

#include "system.h"

#include <rpmcli.h>

#include "rpmlead.h"
#include "signature.h"
#include "misc.h"	/* XXX for makeTempFile() */
#include "debug.h"

/* for PHP */
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include "php.h"
#include "SAPI.h"
#include <defext.h>

void rpm_printf (int, char *, ...);
void rpm_last_message (int, char *, ...);

#ifdef rpmMessage
#undef rpmMessage
#endif

#ifdef rpmError
#undef rpmError
#endif

#define rpmMessage rpm_printf
#define rpmError rpm_last_message
/* for PHP */


/*@access Header@*/		/* XXX compared with NULL */
/*@access FD_t@*/		/* XXX compared with NULL */

static int manageFile(FD_t *fdp, const char **fnp, int flags,
		/*@unused@*/ int rc)
	/*@globals rpmGlobalMacroContext, fileSystem @*/
	/*@modifies *fdp, *fnp, rpmGlobalMacroContext, fileSystem @*/
{
    const char *fn;
    FD_t fd;

    if (fdp == NULL) {	/* programmer error */
	return 1;
    }

    /* close and reset *fdp to NULL */
    if (*fdp && (fnp == NULL || *fnp == NULL)) {
	(void) Fclose(*fdp);
	*fdp = NULL;
	return 0;
    }

    /* open a file and set *fdp */
    if (*fdp == NULL && fnp && *fnp) {
	fd = Fopen(*fnp, ((flags & O_WRONLY) ? "w.ufdio" : "r.ufdio"));
	if (fd == NULL || Ferror(fd)) {
	    rpmError(RPMERR_OPEN, _("%s: open failed: %s\n"), *fnp,
		Fstrerror(fd));
	    return 1;
	}
	*fdp = fd;
	return 0;
    }

    /* open a temp file */
    if (*fdp == NULL && (fnp == NULL || *fnp == NULL)) {
	fn = NULL;
	if (makeTempFile(NULL, (fnp ? &fn : NULL), &fd)) {
	    rpmError(RPMERR_MAKETEMP, _("makeTempFile failed\n"));
	    return 1;
	}
	if (fnp)
	    *fnp = fn;
	*fdp = fdLink(fd, "manageFile return");
	(void) fdFree(fd, "manageFile return");
	return 0;
    }

    /* no operation */
    if (*fdp && fnp && *fnp) {
	return 0;
    }

    /* XXX never reached */
    return 1;
}

static int copyFile(FD_t *sfdp, const char **sfnp,
	FD_t *tfdp, const char **tfnp)
	/*@modifies *sfdp, *sfnp, *tfdp, *tfnp, fileSystem @*/
{
    unsigned char buffer[BUFSIZ];
    ssize_t count;
    int rc = 1;

    if (manageFile(sfdp, sfnp, O_RDONLY, 0))
	goto exit;
    if (manageFile(tfdp, tfnp, O_WRONLY|O_CREAT|O_TRUNC, 0))
	goto exit;

    while ((count = Fread(buffer, sizeof(buffer[0]), sizeof(buffer), *sfdp)) > 0) {
	if (Fwrite(buffer, sizeof(buffer[0]), count, *tfdp) != count) {
	    rpmError(RPMERR_FWRITE, _("%s: Fwrite failed: %s\n"), *tfnp,
		Fstrerror(*tfdp));
	    goto exit;
	}
    }
    if (count < 0) {
	rpmError(RPMERR_FREAD, _("%s: Fread failed: %s\n"), *sfnp, Fstrerror(*sfdp));
	goto exit;
    }

    rc = 0;

exit:
    if (*sfdp)	(void) manageFile(sfdp, NULL, 0, rc);
    if (*tfdp)	(void) manageFile(tfdp, NULL, 0, rc);
    return rc;
}

int rpmCheckSig_p (rpmCheckSigFlags flags, const char ** argv)
{
    FD_t fd = NULL;
    FD_t ofd = NULL;
    int res2, res3;
    struct rpmlead lead, *l = &lead;
    const char *rpm = NULL;
    char result[8*BUFSIZ];
    const char * sigtarget = NULL;
    unsigned char buffer[8192];
    unsigned char missingKeys[7164];
    unsigned char untrustedKeys[7164];
    Header sig;
    HeaderIterator hi;
    int_32 tag, type, count;
    const void * ptr;
    int res = 0;
    rpmRC rc;

    if (argv)
    while ((rpm = *argv++) != NULL) {

	if (manageFile(&fd, &rpm, O_RDONLY, 0)) {
	    res++;
	    goto bottom;
	}

	memset(l, 0, sizeof(*l));
	if (readLead(fd, l)) {
	    rpmError(RPMERR_READLEAD, _("%s: readLead failed\n"), rpm);
	    res++;
	    goto bottom;
	}
	switch (l->major) {
	case 1:
	    rpmError(RPMERR_BADSIGTYPE, _("%s: No signature available (v1.0 RPM)\n"), rpm);
	    res++;
	    goto bottom;
	    /*@notreached@*/ break;
	default:
	    break;
	}

	rc = rpmReadSignature(fd, &sig, l->signature_type);
	if (!(rc == RPMRC_OK || rc == RPMRC_BADSIZE)) {
	    rpmError(RPMERR_SIGGEN, _("%s: rpmReadSignature failed\n"), rpm);
	    res++;
	    goto bottom;
	}
	if (sig == NULL) {
	    rpmError(RPMERR_SIGGEN, _("%s: No signature available\n"), rpm);
	    res++;
	    goto bottom;
	}
	/* Write the header and archive to a temp file */
	/* ASSERT: ofd == NULL && sigtarget == NULL */
	if (copyFile(&fd, &rpm, &ofd, &sigtarget)) {
	    res++;
	    goto bottom;
	}
	/* Both fd and ofd are now closed. sigtarget contains tempfile name. */
	/* ASSERT: fd == NULL && ofd == NULL */

	res2 = 0;
	missingKeys[0] = '\0';
	untrustedKeys[0] = '\0';
	sprintf(buffer, "%s:%c", rpm, (rpmIsVerbose() ? '\n' : ' ') );

	for (hi = headerInitIterator(sig);
	    headerNextIterator(hi, &tag, &type, &ptr, &count);
	    ptr = headerFreeData(ptr, type))
	{
	    switch (tag) {
	    case RPMSIGTAG_PGP5:	/* XXX legacy */
	    case RPMSIGTAG_PGP:
		if (!(flags & CHECKSIG_PGP)) 
		     continue;
		break;
	    case RPMSIGTAG_GPG:
		if (!(flags & CHECKSIG_GPG)) 
		     continue;
		break;
	    case RPMSIGTAG_LEMD5_2:
	    case RPMSIGTAG_LEMD5_1:
	    case RPMSIGTAG_MD5:
		if (!(flags & CHECKSIG_MD5)) 
		     continue;
		break;
	    default:
		continue;
		/*@notreached@*/ break;
	    }
	    if (ptr == NULL) continue;	/* XXX can't happen */

	    if ((res3 = rpmVerifySignature(sigtarget, tag, ptr, count, 
					   result))) {
		if (rpmIsVerbose()) {
		    strcat(buffer, result);
		    res2 = 1;
		} else {
		    char *tempKey;
		    switch (tag) {
		      case RPMSIGTAG_SIZE:
			strcat(buffer, "SIZE ");
			res2 = 1;
			break;
		      case RPMSIGTAG_LEMD5_2:
		      case RPMSIGTAG_LEMD5_1:
		      case RPMSIGTAG_MD5:
			strcat(buffer, "MD5 ");
			res2 = 1;
			break;
		      case RPMSIGTAG_PGP5:	/* XXX legacy */
		      case RPMSIGTAG_PGP:
			switch (res3) {
			case RPMSIG_NOKEY:
			    res2 = 1;
			    /*@fallthrough@*/
			case RPMSIG_NOTTRUSTED:
			{   int offset = 7;
			    strcat(buffer, "(PGP) ");
			    tempKey = strstr(result, "Key ID");
			    if (tempKey == NULL) {
			        tempKey = strstr(result, "keyid:");
				offset = 9;
			    }
			    if (tempKey) {
			      if (res3 == RPMSIG_NOKEY) {
				strcat(missingKeys, " PGP#");
				/*@-compdef@*/
				strncat(missingKeys, tempKey + offset, 8);
				/*@=compdef@*/
			      } else {
			        strcat(untrustedKeys, " PGP#");
				/*@-compdef@*/
				strncat(untrustedKeys, tempKey + offset, 8);
				/*@=compdef@*/
			      }
			    }
			}   break;
			default:
			    strcat(buffer, "PGP ");
			    res2 = 1;
			    break;
			}
			break;
		      case RPMSIGTAG_GPG:
			/* Do not consider this a failure */
			switch (res3) {
			case RPMSIG_NOKEY:
			    strcat(buffer, "(GPG) ");
			    strcat(missingKeys, " GPG#");
			    tempKey = strstr(result, "key ID");
			    if (tempKey)
				/*@-compdef@*/
				strncat(missingKeys, tempKey+7, 8);
				/*@=compdef@*/
			    res2 = 1;
			    break;
			default:
			    strcat(buffer, "GPG ");
			    res2 = 1;
			    break;
			}
			break;
		      default:
			strcat(buffer, "?UnknownSignatureType? ");
			res2 = 1;
			break;
		    }
		}
	    } else {
		if (rpmIsVerbose()) {
		    strcat(buffer, result);
		} else {
		    switch (tag) {
		    case RPMSIGTAG_SIZE:
			strcat(buffer, "size ");
			break;
		    case RPMSIGTAG_LEMD5_2:
		    case RPMSIGTAG_LEMD5_1:
		    case RPMSIGTAG_MD5:
			strcat(buffer, "md5 ");
			break;
		    case RPMSIGTAG_PGP5:	/* XXX legacy */
		    case RPMSIGTAG_PGP:
			strcat(buffer, "pgp ");
			break;
		    case RPMSIGTAG_GPG:
			strcat(buffer, "gpg ");
			break;
		    default:
			strcat(buffer, "??? ");
			break;
		    }
		}
	    }
	}
	hi = headerFreeIterator(hi);
	res += res2;
	(void) unlink(sigtarget);
	sigtarget = _free(sigtarget);

	if (res2) {
	    if (rpmIsVerbose()) {
		rpmMessage(RPMERR_SIGVFY, "%s", (char *)buffer);
	    } else {
		rpmMessage(RPMERR_SIGVFY, "%s%s%s%s%s%s%s%s\n", (char *)buffer,
			_("NOT OK"),
			(missingKeys[0] != '\0') ? _(" (MISSING KEYS:") : "",
			(char *)missingKeys,
			(missingKeys[0] != '\0') ? _(") ") : "",
			(untrustedKeys[0] != '\0') ? _(" (UNTRUSTED KEYS:") : "",
			(char *)untrustedKeys,
			(untrustedKeys[0] != '\0') ? _(")") : "");

	    }
	} else {
	    if (rpmIsVerbose()) {
		rpmMessage(RPMERR_SIGVFY, "%s", (char *)buffer);
	    } else {
		rpmMessage(RPMERR_SIGVFY, "%s%s%s%s%s%s%s%s\n", (char *)buffer,
			_("OK"),
			(missingKeys[0] != '\0') ? _(" (MISSING KEYS:") : "",
			(char *)missingKeys,
			(missingKeys[0] != '\0') ? _(") ") : "",
			(untrustedKeys[0] != '\0') ? _(" (UNTRUSTED KEYS:") : "",
			(char *)untrustedKeys,
			(untrustedKeys[0] != '\0') ? _(")") : "");
	    }
	}

    bottom:
	if (fd)		(void) manageFile(&fd, NULL, 0, 0);
	if (ofd)	(void) manageFile(&ofd, NULL, 0, 0);
	if (sigtarget) {
	    (void) unlink(sigtarget);
	    sigtarget = _free(sigtarget);
	}
    }

    return res;
}
