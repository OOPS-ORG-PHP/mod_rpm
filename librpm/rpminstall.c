/** \ingroup rpmcli
 * \file lib/rpminstall.c
 */

#include "system.h"

#include <rpmcli.h>

#include "manifest.h"
#include "misc.h"	/* XXX for rpmGlob() */
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
void rpmProblemSetPrint_p(FILE *fp, rpmProblemSet probs);
void printDepProblems_p(FILE * fp, const rpmDependencyConflict conflicts, int numConflicts);

#define LASTMSGADD -100

#ifdef rpmMessage
#undef rpmMessage
#endif

#ifdef rpmError
#undef rpmError
#endif

#define rpmMessage rpm_printf
#define rpmError rpm_last_message

/* for PHP */

/*@unchecked@*/
int packagesTotal = 0;

typedef const char * str_t;

struct rpmEIU {
/*@only@*/ rpmTransactionSet ts;
/*@only@*/ /*@null@*/ rpmdb db;
    Header h;
    FD_t fd;
    int numFailed;
    int numPkgs;
/*@only@*/ str_t * pkgURL;
/*@dependent@*/ /*@null@*/ str_t * fnp;
/*@only@*/ char * pkgState;
    int prevx;
    int pkgx;
    int numRPMS;
    int numSRPMS;
/*@only@*/ /*@null@*/ str_t * sourceURL;
    int isSource;
    int argc;
/*@only@*/ /*@null@*/ str_t * argv;
/*@temp@*/ rpmRelocation * relocations;
    rpmRC rpmrc;
};

/** @todo Generalize --freshen policies. */
int rpmInstall_p(const char * rootdir, const char ** fileArgv,
		rpmtransFlags transFlags,
		rpmInstallInterfaceFlags interfaceFlags,
		rpmprobFilterFlags probFilter,
		rpmRelocation * relocations)
{
    struct rpmEIU * eiu = alloca(sizeof(*eiu));
    int notifyFlags = interfaceFlags | (rpmIsVerbose() ? INSTALL_LABEL : 0 );
    /*@only@*/ /*@null@*/ const char * fileURL = NULL;
    int stopInstall = 0;
    const char ** av = NULL;
    int ac = 0;
    int rc;
    int xx;
    int i;

    if (fileArgv == NULL) return 0;

    memset(eiu, 0, sizeof(*eiu));
    eiu->numPkgs = 0;
    eiu->prevx = 0;
    eiu->pkgx = 0;

    if ((eiu->relocations = relocations) != NULL) {
	while (eiu->relocations->oldPath)
	    eiu->relocations++;
	if (eiu->relocations->newPath == NULL)
	    eiu->relocations = NULL;
    }

    /* Build fully globbed list of arguments in argv[argc]. */
    /*@-branchstate@*/
    /*@-temptrans@*/
    for (eiu->fnp = fileArgv; *eiu->fnp != NULL; eiu->fnp++) {
    /*@=temptrans@*/
	av = _free(av);	ac = 0;
	rc = rpmGlob(*eiu->fnp, &ac, &av);
	if (rc || ac == 0) continue;

	eiu->argv = xrealloc(eiu->argv, (eiu->argc+ac+1) * sizeof(*eiu->argv));
	memcpy(eiu->argv+eiu->argc, av, ac * sizeof(*av));
	eiu->argc += ac;
	eiu->argv[eiu->argc] = NULL;
    }
    /*@=branchstate@*/
    av = _free(av);	ac = 0;

restart:
    /* Allocate sufficient storage for next set of args. */
    if (eiu->pkgx >= eiu->numPkgs) {
	eiu->numPkgs = eiu->pkgx + eiu->argc;
	eiu->pkgURL = xrealloc(eiu->pkgURL,
			(eiu->numPkgs + 1) * sizeof(*eiu->pkgURL));
	memset(eiu->pkgURL + eiu->pkgx, 0,
			((eiu->argc + 1) * sizeof(*eiu->pkgURL)));
	eiu->pkgState = xrealloc(eiu->pkgState,
			(eiu->numPkgs + 1) * sizeof(*eiu->pkgState));
	memset(eiu->pkgState + eiu->pkgx, 0,
			((eiu->argc + 1) * sizeof(*eiu->pkgState)));
    }

    /* Retrieve next set of args, cache on local storage. */
    for (i = 0; i < eiu->argc; i++) {
	fileURL = _free(fileURL);
	fileURL = eiu->argv[i];
	eiu->argv[i] = NULL;

	switch (urlIsURL(fileURL)) {
	case URL_IS_FTP:
	case URL_IS_HTTP:
	{   const char *tfn;

	    if (rpmIsVerbose())
		fprintf(stdout, _("Retrieving %s\n"), fileURL);

	    {	char tfnbuf[64];
		strcpy(tfnbuf, "rpm-xfer.XXXXXX");
		(void) mktemp(tfnbuf);
		tfn = rpmGenPath(rootdir, "%{_tmppath}/", tfnbuf);
	    }

	    /* XXX undefined %{name}/%{version}/%{release} here */
	    /* XXX %{_tmpdir} does not exist */
	    rpmMessage(RPMMESS_DEBUG, _(" ... as %s\n"), tfn);
	    rc = urlGetFile(fileURL, tfn);
	    if (rc < 0) {
		//rpmMessage(RPMMESS_ERROR,
		rpmError(LASTMSGADD,
			_("skipping %s - transfer failed - %s\n"),
			fileURL, ftpStrerror(rc));
		eiu->numFailed++;
		eiu->pkgURL[eiu->pkgx] = NULL;
		tfn = _free(tfn);
		/*@switchbreak@*/ break;
	    }
	    eiu->pkgState[eiu->pkgx] = 1;
	    eiu->pkgURL[eiu->pkgx] = tfn;
	    eiu->pkgx++;
	}   /*@switchbreak@*/ break;
	case URL_IS_PATH:
	default:
	    eiu->pkgURL[eiu->pkgx] = fileURL;
	    fileURL = NULL;
	    eiu->pkgx++;
	    /*@switchbreak@*/ break;
	}
    }
    fileURL = _free(fileURL);

    if (eiu->numFailed) goto exit;

    /* Continue processing file arguments, building transaction set. */
    for (eiu->fnp = eiu->pkgURL+eiu->prevx;
	 *eiu->fnp != NULL;
	 eiu->fnp++, eiu->prevx++)
    {
	const char * fileName;

	rpmMessage(RPMMESS_DEBUG, "============== %s\n", *eiu->fnp);
	(void) urlPath(*eiu->fnp, &fileName);

	/* Try to read the header from a package file. */
	eiu->fd = Fopen(*eiu->fnp, "r.ufdio");
	if (eiu->fd == NULL || Ferror(eiu->fd)) {
	    rpmError(LASTMSGADD, _("open of %s failed: %s\n"), *eiu->fnp,
			Fstrerror(eiu->fd));
	    if (eiu->fd) {
		xx = Fclose(eiu->fd);
		eiu->fd = NULL;
	    }
	    eiu->numFailed++; *eiu->fnp = NULL;
	    continue;
	}

	/*@-mustmod@*/	/* LCL: segfault */
	eiu->rpmrc = rpmReadPackageHeader(eiu->fd, &eiu->h,
				&eiu->isSource, NULL, NULL);
	/*@=mustmod@*/
	xx = Fclose(eiu->fd);
	eiu->fd = NULL;

	if (eiu->rpmrc == RPMRC_FAIL || eiu->rpmrc == RPMRC_SHORTREAD) {
	    eiu->numFailed++; *eiu->fnp = NULL;
	    continue;
	}

	if (eiu->isSource &&
		(eiu->rpmrc == RPMRC_OK || eiu->rpmrc == RPMRC_BADSIZE))
	{
	    rpmMessage(RPMMESS_DEBUG, "\tadded source package [%d]\n",
		eiu->numSRPMS);
	    eiu->sourceURL = xrealloc(eiu->sourceURL,
				(eiu->numSRPMS + 2) * sizeof(*eiu->sourceURL));
	    eiu->sourceURL[eiu->numSRPMS] = *eiu->fnp;
	    *eiu->fnp = NULL;
	    eiu->numSRPMS++;
	    eiu->sourceURL[eiu->numSRPMS] = NULL;
	    continue;
	}

	if (eiu->rpmrc == RPMRC_OK || eiu->rpmrc == RPMRC_BADSIZE) {
	    if (eiu->db == NULL) {
		int mode = (transFlags & RPMTRANS_FLAG_TEST)
				? O_RDONLY : (O_RDWR | O_CREAT);

		if (rpmdbOpen(rootdir, &eiu->db, mode, 0644)) {
		    const char *dn;
		    dn = rpmGetPath( (rootdir ? rootdir : ""),
					"%{_dbpath}", NULL);
		    //rpmMessage(RPMMESS_ERROR,
		    rpmError(LASTMSGADD,
				_("cannot open Packages database in %s\n"), dn);
		    dn = _free(dn);
		    eiu->numFailed++; *eiu->fnp = NULL;
		    break;
		}
		/*@-onlytrans@*/
		eiu->ts = rpmtransCreateSet(eiu->db, rootdir);
		/*@=onlytrans@*/
	    }

	    if (eiu->relocations) {
		const char ** paths;
		int pft;
		int c;

		if (headerGetEntry(eiu->h, RPMTAG_PREFIXES, &pft,
				       (void **) &paths, &c) && (c == 1)) {
		    eiu->relocations->oldPath = xstrdup(paths[0]);
		    paths = headerFreeData(paths, pft);
		} else {
		    const char * name;
		    xx = headerNVR(eiu->h, &name, NULL, NULL);
		    //rpmMessage(RPMMESS_ERROR,
		    rpmError(LASTMSGADD,
			       _("package %s is not relocateable\n"), name);
		    eiu->numFailed++;
		    goto exit;
		    /*@notreached@*/
		}
	    }

	    /* On --freshen, verify package is installed and newer */
	    if (interfaceFlags & INSTALL_FRESHEN) {
		rpmdbMatchIterator mi;
		const char * name;
		Header oldH;
		int count;

		xx = headerNVR(eiu->h, &name, NULL, NULL);
		/*@-onlytrans@*/
		mi = rpmdbInitIterator(eiu->db, RPMTAG_NAME, name, 0);
		/*@=onlytrans@*/
		count = rpmdbGetIteratorCount(mi);
		while ((oldH = rpmdbNextIterator(mi)) != NULL) {
		    if (rpmVersionCompare(oldH, eiu->h) < 0)
			/*@innercontinue@*/ continue;
		    /* same or newer package already installed */
		    count = 0;
		    /*@innerbreak@*/ break;
		}
		mi = rpmdbFreeIterator(mi);
		if (count == 0) {
		    eiu->h = headerFree(eiu->h);
		    continue;
		}
		/* Package is newer than those currently installed. */
	    }

	    rc = rpmtransAddPackage(eiu->ts, eiu->h, NULL, fileName,
			       (interfaceFlags & INSTALL_UPGRADE) != 0,
			       relocations);
	    /* XXX reference held by transaction set */
	    eiu->h = headerFree(eiu->h);
	    if (eiu->relocations)
		eiu->relocations->oldPath = _free(eiu->relocations->oldPath);

	    switch(rc) {
	    case 0:
		rpmMessage(RPMMESS_DEBUG, "\tadded binary package [%d]\n",
			eiu->numRPMS);
		/*@switchbreak@*/ break;
	    case 1:
		//rpmMessage(RPMMESS_ERROR,
		rpmError(LASTMSGADD,
			    _("error reading from file %s\n"), *eiu->fnp);
		eiu->numFailed++;
		goto exit;
		/*@notreached@*/ /*@switchbreak@*/ break;
	    case 2:
		//rpmMessage(RPMMESS_ERROR,
		rpmError(LASTMSGADD,
			    _("file %s requires a newer version of RPM\n"),
			    *eiu->fnp);
		eiu->numFailed++;
		goto exit;
		/*@notreached@*/ /*@switchbreak@*/ break;
	    }

	    eiu->numRPMS++;
	    continue;
	}

	if (eiu->rpmrc != RPMRC_BADMAGIC) {
	    //rpmMessage(RPMMESS_ERROR, _("%s cannot be installed\n"), *eiu->fnp);
	    rpmError(LASTMSGADD, _("%s cannot be installed\n"), *eiu->fnp);
	    eiu->numFailed++; *eiu->fnp = NULL;
	    break;
	}

	/* Try to read a package manifest. */
	eiu->fd = Fopen(*eiu->fnp, "r.fpio");
	if (eiu->fd == NULL || Ferror(eiu->fd)) {
	    rpmError(LASTMSGADD, _("open of %s failed: %s\n"), *eiu->fnp,
			Fstrerror(eiu->fd));
	    if (eiu->fd) {
		xx = Fclose(eiu->fd);
		eiu->fd = NULL;
	    }
	    eiu->numFailed++; *eiu->fnp = NULL;
	    break;
	}

	/* Read list of packages from manifest. */
	rc = rpmReadPackageManifest(eiu->fd, &eiu->argc, &eiu->argv);
	if (rc)
	    rpmError(LASTMSGADD, _("%s: read manifest failed: %s\n"),
			*eiu->fnp, Fstrerror(eiu->fd));
	xx = Fclose(eiu->fd);
	eiu->fd = NULL;

	/* If successful, restart the query loop. */
	if (rc == 0) {
	    eiu->prevx++;
	    goto restart;
	}

	eiu->numFailed++; *eiu->fnp = NULL;
	break;
    }

    //rpmMessage(RPMMESS_DEBUG, _("found %d source and %d binary packages\n"),
    rpmError(LASTMSGADD, _("found %d source and %d binary packages\n"),
		eiu->numSRPMS, eiu->numRPMS);

    if (eiu->numFailed) goto exit;

    if (eiu->numRPMS && !(interfaceFlags & INSTALL_NODEPS)) {
	rpmDependencyConflict conflicts;
	int numConflicts;

	if (rpmdepCheck(eiu->ts, &conflicts, &numConflicts)) {
	    eiu->numFailed = eiu->numPkgs;
	    stopInstall = 1;
	}

	/*@-branchstate@*/
	if (!stopInstall && conflicts) {
	    //rpmMessage(RPMMESS_ERROR, _("failed dependencies:\n"));
	    rpmError(LASTMSGADD, _("failed dependencies:\n"));
	    printDepProblems_p(stderr, conflicts, numConflicts);
	    conflicts = rpmdepFreeConflicts(conflicts, numConflicts);
	    eiu->numFailed = eiu->numPkgs;
	    stopInstall = 1;
	}
	/*@=branchstate@*/
    }

    if (eiu->numRPMS && !(interfaceFlags & INSTALL_NOORDER)) {
	if (rpmdepOrder(eiu->ts)) {
	    eiu->numFailed = eiu->numPkgs;
	    stopInstall = 1;
	}
    }

    if (eiu->numRPMS && !stopInstall) {
	rpmProblemSet probs = NULL;

	packagesTotal = eiu->numRPMS + eiu->numSRPMS;

	rpmMessage(RPMMESS_DEBUG, _("installing binary packages\n"));
	rc = rpmRunTransactions(eiu->ts, rpmShowProgress,
			(void *) ((long)notifyFlags),
		 	NULL, &probs, transFlags, probFilter);

	if (rc < 0) {
	    eiu->numFailed += eiu->numRPMS;
	} else if (rc > 0) {
	    eiu->numFailed += rc;
	    rpmProblemSetPrint_p(stderr, probs);
	}

	if (probs != NULL) rpmProblemSetFree(probs);
    }

    if (eiu->numSRPMS && !stopInstall) {
	if (eiu->sourceURL != NULL)
	for (i = 0; i < eiu->numSRPMS; i++) {
	    if (eiu->sourceURL[i] == NULL) continue;
	    eiu->fd = Fopen(eiu->sourceURL[i], "r.ufdio");
	    if (eiu->fd == NULL || Ferror(eiu->fd)) {
		//rpmMessage(RPMMESS_ERROR, _("cannot open file %s: %s\n"),
		rpmError(LASTMSGADD, _("cannot open file %s: %s\n"),
			   eiu->sourceURL[i], Fstrerror(eiu->fd));
		if (eiu->fd) {
		    xx = Fclose(eiu->fd);
		    eiu->fd = NULL;
		}
		continue;
	    }

	    if (!(transFlags & RPMTRANS_FLAG_TEST)) {
		eiu->rpmrc = rpmInstallSourcePackage(rootdir, eiu->fd, NULL,
			rpmShowProgress, (void *) ((long)notifyFlags), NULL);
		if (eiu->rpmrc != RPMRC_OK) eiu->numFailed++;
	    }

	    xx = Fclose(eiu->fd);
	    eiu->fd = NULL;
	}
    }

exit:
    eiu->ts = rpmtransFree(eiu->ts);
    if (eiu->pkgURL != NULL)
    for (i = 0; i < eiu->numPkgs; i++) {
	if (eiu->pkgURL[i] == NULL) continue;
	if (eiu->pkgState[i] == 1)
	    (void) Unlink(eiu->pkgURL[i]);
	eiu->pkgURL[i] = _free(eiu->pkgURL[i]);
    }
    eiu->pkgState = _free(eiu->pkgState);
    eiu->pkgURL = _free(eiu->pkgURL);
    eiu->argv = _free(eiu->argv);
    if (eiu->db != NULL) {
	xx = rpmdbClose(eiu->db);
	eiu->db = NULL;
    }
    return eiu->numFailed;
}

int rpmErase_p(const char * rootdir, const char ** argv,
		rpmtransFlags transFlags,
		rpmEraseInterfaceFlags interfaceFlags)
{
    rpmdb db;
    int mode;
    int count;
    const char ** arg;
    int numFailed = 0;
    rpmTransactionSet ts;
    rpmDependencyConflict conflicts;
    int numConflicts;
    int stopUninstall = 0;
    int numPackages = 0;
    rpmProblemSet probs;

    if (argv == NULL) return 0;

    if (transFlags & RPMTRANS_FLAG_TEST)
	mode = O_RDONLY;
    else
	mode = O_RDWR | O_EXCL;
	
    if (rpmdbOpen(rootdir, &db, mode, 0644)) {
	const char *dn;
	dn = rpmGetPath( (rootdir ? rootdir : ""), "%{_dbpath}", NULL);
	//rpmMessage(RPMMESS_ERROR, _("cannot open %s/packages.rpm\n"), dn);
	rpmError(LASTMSGADD, _("cannot open %s/packages.rpm\n"), dn);
	dn = _free(dn);
	return -1;
    }

    ts = rpmtransCreateSet(db, rootdir);
    for (arg = argv; *arg; arg++) {
	rpmdbMatchIterator mi;

	/* XXX HACK to get rpmdbFindByLabel out of the API */
	mi = rpmdbInitIterator(db, RPMDBI_LABEL, *arg, 0);
	count = rpmdbGetIteratorCount(mi);
	if (count <= 0) {
	    //rpmMessage(RPMMESS_ERROR, _("package %s is not installed\n"), *arg);
	    rpmError(LASTMSGADD, _("package %s is not installed\n"), *arg);
	    numFailed++;
	} else if (!(count == 1 || (interfaceFlags & UNINSTALL_ALLMATCHES))) {
	    //rpmMessage(RPMMESS_ERROR, _("\"%s\" specifies multiple packages\n"),
	    rpmError(LASTMSGADD, _("\"%s\" specifies multiple packages\n"),
			*arg);
	    numFailed++;
	} else {
	    Header h;	/* XXX iterator owns the reference */
	    while ((h = rpmdbNextIterator(mi)) != NULL) {
		unsigned int recOffset = rpmdbGetIteratorOffset(mi);
		if (recOffset) {
		    (void) rpmtransRemovePackage(ts, recOffset);
		    numPackages++;
		}
	    }
	}
	mi = rpmdbFreeIterator(mi);
    }

    if (!(interfaceFlags & UNINSTALL_NODEPS)) {
	if (rpmdepCheck(ts, &conflicts, &numConflicts)) {
	    numFailed = numPackages;
	    stopUninstall = 1;
	}

	/*@-branchstate@*/
	if (!stopUninstall && conflicts) {
	    //rpmMessage(RPMMESS_ERROR, _("removing these packages would break "
	    rpmError(LASTMSGADD, _("removing these packages would break "
			      "dependencies:\n"));
	    printDepProblems_p(stderr, conflicts, numConflicts);
	    conflicts = rpmdepFreeConflicts(conflicts, numConflicts);
	    numFailed += numPackages;
	    stopUninstall = 1;
	}
	/*@=branchstate@*/
    }

    if (!stopUninstall) {
	transFlags |= RPMTRANS_FLAG_REVERSE;
	numFailed += rpmRunTransactions(ts, NULL, NULL, NULL, &probs,
					transFlags, 0);
    }

    ts = rpmtransFree(ts);
    (void) rpmdbClose(db);

    return numFailed;
}

int rpmInstallSource_p(const char * rootdir, const char * arg,
		const char ** specFile, char ** cookie)
{
    FD_t fd;
    int rc;

    fd = Fopen(arg, "r.ufdio");
    if (fd == NULL || Ferror(fd)) {
	//rpmMessage(RPMMESS_ERROR, _("cannot open %s: %s\n"), arg, Fstrerror(fd));
	rpmError(LASTMSGADD, _("cannot open %s: %s\n"), arg, Fstrerror(fd));
	if (fd) (void) Fclose(fd);
	return 1;
    }

    if (rpmIsVerbose())
	fprintf(stdout, _("Installing %s\n"), arg);

    {
	/*@-mayaliasunique@*/
	rpmRC rpmrc = rpmInstallSourcePackage(rootdir, fd, specFile, NULL, NULL,
				 cookie);
	/*@=mayaliasunique@*/
	rc = (rpmrc == RPMRC_OK ? 0 : 1);
    }
    if (rc != 0) {
	//rpmMessage(RPMMESS_ERROR, _("%s cannot be installed\n"), arg);
	rpmError(LASTMSGADD, _("%s cannot be installed\n"), arg);
	/*@-unqualifiedtrans@*/
	if (specFile && *specFile)
	    *specFile = _free(*specFile);
	if (cookie && *cookie)
	    *cookie = _free(*cookie);
	/*@=unqualifiedtrans@*/
    }

    (void) Fclose(fd);

    return rc;
}

