#include <system.h>

#define	_AUTOHELP

#include <defext.h>
#include <stdarg.h>
void rpm_last_message (int stream, char *format, ...);

#undef stderr
#define stderr 0
#define fprintf rpm_last_message

#if defined(IAM_RPM) || defined(__LCLINT__)
#define	IAM_RPMDB
#define	IAM_RPMQV
#define	IAM_RPMK
#define IAM_RPMEIU
#endif

#include <rpmcli.h>
#include <rpmbuild.h>

#define	POPT_NODEPS		1025
#define	POPT_FORCE		1026
#define	POPT_NOMD5		1027
#define	POPT_NOSCRIPTS		1028

#if defined(IAM_RPMK)
#include <signature.h>
#endif

#include <debug.h>

#define GETOPT_DBPATH		1010
#define GETOPT_SHOWRC		1018
#define	GETOPT_DEFINEMACRO	1020
#define	GETOPT_EVALMACRO	1021
#ifdef	NOTYET
#define	GETOPT_RCFILE		1022
#endif

int rpmVerify_p (QVA_t qva, rpmQVSources source, const char * arg);
int rpmQuery_p(QVA_t qva, rpmQVSources source, const char * arg);
int rpmInstall_p(const char * rootdir, const char ** fileArgv,
		 rpmtransFlags transFlags,
		 rpmInstallInterfaceFlags interfaceFlags,
		 rpmprobFilterFlags probFilter,
		 rpmRelocation * relocations);
int rpmErase_p(const char * rootdir, const char ** argv,
		 rpmtransFlags transFlags,
		 rpmEraseInterfaceFlags interfaceFlags);

enum modes {

    MODE_QUERY		= (1 <<  0),
    MODE_VERIFY		= (1 <<  3),
    MODE_QUERYTAGS	= (1 <<  9),
#define	MODES_QV (MODE_QUERY | MODE_VERIFY)

    MODE_INSTALL	= (1 <<  1),
    MODE_ERASE		= (1 <<  2),
#define	MODES_IE (MODE_INSTALL | MODE_ERASE)

    MODE_BUILD		= (1 <<  4),
    MODE_REBUILD	= (1 <<  5),
    MODE_RECOMPILE	= (1 <<  8),
    MODE_TARBUILD	= (1 << 11),
#define	MODES_BT (MODE_BUILD | MODE_TARBUILD | MODE_REBUILD | MODE_RECOMPILE)

    MODE_CHECKSIG	= (1 <<  6),
    MODE_RESIGN		= (1 <<  7),
#define	MODES_K	 (MODE_CHECKSIG | MODE_RESIGN)

    MODE_INITDB		= (1 << 10),
    MODE_REBUILDDB	= (1 << 12),
    MODE_VERIFYDB	= (1 << 13),
#define	MODES_DB (MODE_INITDB | MODE_REBUILDDB | MODE_VERIFYDB)


    MODE_UNKNOWN	= 0
};

#define	MODES_FOR_DBPATH	(MODES_BT | MODES_IE | MODES_QV | MODES_DB)
#define	MODES_FOR_NODEPS	(MODES_BT | MODES_IE | MODE_VERIFY)
#define	MODES_FOR_TEST		(MODES_BT | MODES_IE)
#define	MODES_FOR_ROOT		(MODES_BT | MODES_IE | MODES_QV | MODES_DB)

/*@-exportheadervar@*/
/*@unchecked@*/
extern int _ftp_debug;
/*@unchecked@*/
extern int noLibio;
/*@unchecked@*/
extern int _rpmio_debug;
/*@-redecl@*/
/*@unchecked@*/
extern int _url_debug;
/*@=redecl@*/

/*@=exportheadervar@*/

/* options for all executables */

/*@unchecked@*/
static int help = 0;
/*@unchecked@*/
static int noUsageMsg = 0;
/*@unchecked@*/ /*@observer@*/ /*@null@*/
static const char * pipeOutput = NULL;
/*@unchecked@*/
static int quiet = 0;
/*@unchecked@*/ /*@observer@*/ /*@null@*/
static const char * rcfile = NULL;
/*@unchecked@*/ /*@observer@*/ /*@null@*/
static char * rootdir = "/";
/*@unchecked@*/
static int showrc = 0;
/*@unchecked@*/
static int showVersion = 0;

/*@unchecked@*/
static struct poptOption rpmAllPoptTable[] = {
 { "version", '\0', 0, &showVersion, 0,
	N_("print the version of rpm being used"),
	NULL },
 { "quiet", '\0', 0, &quiet, 0,
	N_("provide less detailed output"), NULL},
 { "verbose", 'v', 0, 0, 'v',
	N_("provide more detailed output"), NULL},
 { "define", '\0', POPT_ARG_STRING, 0, GETOPT_DEFINEMACRO,
	N_("define macro <name> with value <body>"),
	N_("'<name> <body>'") },
 { "eval", '\0', POPT_ARG_STRING, 0, GETOPT_EVALMACRO,
	N_("print macro expansion of <expr>+"),
	N_("<expr>+") },
 { "pipe", '\0', POPT_ARG_STRING|POPT_ARGFLAG_DOC_HIDDEN, &pipeOutput, 0,
	N_("send stdout to <cmd>"),
	N_("<cmd>") },
 { "root", 'r', POPT_ARG_STRING | POPT_ARGFLAG_SHOW_DEFAULT, &rootdir, 0,
	N_("use <dir> as the top level directory"),
	N_("<dir>") },
 { "macros", '\0', POPT_ARG_STRING, &macrofiles, 0,
	N_("read <file:...> instead of default macro file(s)"),
	N_("<file:...>") },
#if !defined(GETOPT_RCFILE)
 { "rcfile", '\0', POPT_ARG_STRING, &rcfile, 0,
	N_("read <file:...> instead of default rpmrc file(s)"),
	N_("<file:...>") },
#else
 { "rcfile", '\0', 0, 0, GETOPT_RCFILE,	
	N_("read <file:...> instead of default rpmrc file(s)"),
	N_("<file:...>") },
#endif
 { "showrc", '\0', 0, &showrc, GETOPT_SHOWRC,
	N_("display final rpmrc and macro configuration"),
	NULL },

#if HAVE_LIBIO_H && defined(_G_IO_IO_FILE_VERSION)
 { "nolibio", '\0', POPT_ARG_VAL|POPT_ARGFLAG_DOC_HIDDEN, &noLibio, 1,
	N_("disable use of libio(3) API"), NULL},
#endif
 { "ftpdebug", '\0', POPT_ARG_VAL|POPT_ARGFLAG_DOC_HIDDEN, &_ftp_debug, -1,
	N_("debug protocol data stream"), NULL},
 { "rpmiodebug", '\0', POPT_ARG_VAL|POPT_ARGFLAG_DOC_HIDDEN, &_rpmio_debug, -1,
	N_("debug rpmio I/O"), NULL},
 { "urldebug", '\0', POPT_ARG_VAL|POPT_ARGFLAG_DOC_HIDDEN, &_url_debug, -1,
	N_("debug URL cache handling"), NULL},

   POPT_TABLEEND
};

/* the structure describing the options we take and the defaults */
/*@unchecked@*/
static struct poptOption optionsTable[] = {

 /* XXX colliding options */
#if defined(IAM_RPMQV) || defined(IAM_RPMEIU)
 {  NULL, 'i', POPT_ARGFLAG_DOC_HIDDEN, 0, 'i',			NULL, NULL},
 {  "nodeps", 0, POPT_ARGFLAG_DOC_HIDDEN, 0, POPT_NODEPS,	NULL, NULL},
 {  "noscripts", 0, POPT_ARGFLAG_DOC_HIDDEN, 0, POPT_NOSCRIPTS,	NULL, NULL},
 {  "nomd5", 0, POPT_ARGFLAG_DOC_HIDDEN, 0, POPT_NOMD5,		NULL, NULL},
 {  "force", 0, POPT_ARGFLAG_DOC_HIDDEN, 0, POPT_FORCE,		NULL, NULL},
#endif

#ifdef	IAM_RPMQV
 { NULL, '\0', POPT_ARG_INCLUDE_TABLE, rpmQueryPoptTable, 0,
	N_("Query options (with -q or --query):"),
	NULL },
 { NULL, '\0', POPT_ARG_INCLUDE_TABLE, rpmVerifyPoptTable, 0,
	N_("Verify options (with -V or --verify):"),
	NULL },
#endif	/* IAM_RPMQV */

#ifdef	IAM_RPMK
 { NULL, '\0', POPT_ARG_INCLUDE_TABLE, rpmSignPoptTable, 0,
	N_("Signature options:"),
	NULL },
#endif	/* IAM_RPMK */

#ifdef	IAM_RPMDB
 { NULL, '\0', POPT_ARG_INCLUDE_TABLE, rpmDatabasePoptTable, 0,
	N_("Database options:"),
	NULL },
#endif	/* IAM_RPMDB */

#ifdef	IAM_RPMEIU
 { NULL, '\0', POPT_ARG_INCLUDE_TABLE, rpmInstallPoptTable, 0,
	N_("Install/Upgrade/Erase options:"),
	NULL },
#endif	/* IAM_RPMEIU */

 { NULL, '\0', POPT_ARG_INCLUDE_TABLE, rpmAllPoptTable, 0,
	N_("Common options for all rpm modes:"),
	NULL },

   POPT_AUTOALIAS
   POPT_AUTOHELP
   POPT_TABLEEND
};

#ifdef __MINT__
/* MiNT cannot dynamically increase the stack.  */
long _stksize = 64 * 1024L;
#endif

/*@exits@*/ static void argerror(const char * desc)
	/*@globals fileSystem @*/
	/*@modifies fileSystem @*/
{
    fprintf(stderr, _("%s: %s\n"), __progname, desc);
}

int __rpmquery (int argc, const char ** argv, char ** envp)
{
    enum modes bigMode = MODE_UNKNOWN;

#ifdef	IAM_RPMQV
    QVA_t qva = &rpmQVArgs;
#endif

#ifdef	IAM_RPMEIU
   struct rpmInstallArguments_s * ia = &rpmIArgs;
#endif

#if defined(IAM_RPMDB)
   struct rpmDatabaseArguments_s * da = &rpmDBArgs;
#endif

#if defined(IAM_RPMK)
   struct rpmSignArguments_s * ka = &rpmKArgs;
   char * passPhrase = "";
#endif

    int arg;
    int gotDbpath = 0;

    const char * optArg;
    pid_t pipeChild = 0;
    poptContext optCon;
    int ec = 0;
    int status;
    int p[2];
	
#if HAVE_MCHECK_H && HAVE_MTRACE
    mtrace();	/* Trace malloc only if MALLOC_TRACE=mtrace-output-file. */
#endif
    setprogname(argv[0]);	/* Retrofit glibc __progname */

#if !defined(__GLIBC__)
    environ = envp;
#endif

    /* XXX glibc churn sanity */
    if (__progname == NULL) {
	if ((__progname = strrchr(argv[0], '/')) != NULL) __progname++;
	else __progname = argv[0];
    }

    /* Set the major mode based on argv[0] */
    /*@-nullpass@*/
#ifdef	IAM_RPMQV
    if (!strcmp(__progname, "rpmq"))	bigMode = MODE_QUERY;
    if (!strcmp(__progname, "rpmv"))	bigMode = MODE_VERIFY;
    if (!strcmp(__progname, "rpmquery"))	bigMode = MODE_QUERY;
    if (!strcmp(__progname, "rpmverify"))	bigMode = MODE_VERIFY;
#endif
#ifdef	RPMEIU
    if (!strcmp(__progname, "rpme"))	bigMode = MODE_ERASE;
    if (!strcmp(__progname, "rpmi"))	bigMode = MODE_INSTALL;
    if (!strcmp(__progname, "rpmu"))	bigMode = MODE_INSTALL;
#endif
    /*@=nullpass@*/

    /* set the defaults for the various command line options */
    _ftp_debug = 0;

#if HAVE_LIBIO_H && defined(_G_IO_IO_FILE_VERSION)
    noLibio = 0;
#else
    noLibio = 1;
#endif
    _rpmio_debug = 0;
    _url_debug = 0;

    /* XXX Eliminate query linkage loop */
    specedit = 0;
    parseSpecVec = parseSpec;
    freeSpecVec = freeSpec;

    /* set up the correct locale */
    (void) setlocale(LC_ALL, "" );

#ifdef	__LCLINT__
#define	LOCALEDIR	"/usr/share/locale"
#endif
    bindtextdomain(PACKAGE, LOCALEDIR);
    textdomain(PACKAGE);

    rpmSetVerbosity(RPMMESS_NORMAL);	/* XXX silly use by showrc */

    /* Make a first pass through the arguments, looking for --rcfile */
    /* We need to handle that before dealing with the rest of the arguments. */
    /*@-nullpass -temptrans@*/
    optCon = poptGetContext(__progname, argc, argv, optionsTable, 0);
    /*@=nullpass =temptrans@*/
    (void) poptReadConfigFile(optCon, LIBRPMALIAS_FILENAME);
    (void) poptReadDefaultConfig(optCon, 1);
    poptSetExecPath(optCon, RPMCONFIGDIR, 1);

    /* reading rcfile early makes it easy to override */
    /* XXX only --rcfile (and --showrc) need this pre-parse */

    while ((arg = poptGetNextOpt(optCon)) > 0) {
	switch(arg) {
	case 'v':
	    rpmIncreaseVerbosity();	/* XXX silly use by showrc */
	    break;
        default:
	    break;
      }
    }

    if (rpmReadConfigFiles(rcfile, NULL))  
	return  EXIT_FAILURE;

    if (showrc) {
	(void) rpmShowRC(stdout);
	return EXIT_SUCCESS;
    }

    rpmSetVerbosity(RPMMESS_NORMAL);	/* XXX silly use by showrc */

    poptResetContext(optCon);

#ifdef	IAM_RPMQV
    qva->qva_queryFormat = _free(qva->qva_queryFormat);
    memset(qva, 0, sizeof(*qva));
    qva->qva_source = RPMQV_PACKAGE;
    qva->qva_fflags = RPMFILE_ALL;
    qva->qva_mode = ' ';
    qva->qva_char = ' ';
#endif

#ifdef	IAM_RPMDB
    memset(da, 0, sizeof(*da));
#endif

#ifdef	IAM_RPMK
    memset(ka, 0, sizeof(*ka));
    ka->addSign = RESIGN_NONE;
    ka->checksigFlags = CHECKSIG_ALL;
#endif

#ifdef	IAM_RPMEIU
    ia->relocations = _free(ia->relocations);
    memset(ia, 0, sizeof(*ia));
    ia->transFlags = RPMTRANS_FLAG_NONE;
    ia->probFilter = RPMPROB_FILTER_NONE;
    ia->installInterfaceFlags = INSTALL_NONE;
    ia->eraseInterfaceFlags = UNINSTALL_NONE;
#endif

    while ((arg = poptGetNextOpt(optCon)) > 0) {
	optArg = poptGetOptArg(optCon);

	switch (arg) {
	    
	case 'v':
	    rpmIncreaseVerbosity();
	    break;

/* XXX options used in multiple rpm modes */
#if defined(IAM_RPMQV) || defined(IAM_RPMK)
	case POPT_NOMD5:
#ifdef	IAM_RPMQV
	    if (bigMode == MODE_VERIFY || qva->qva_mode == 'V')
		qva->qva_flags |= VERIFY_MD5;
	    else
#endif
#ifdef	IAM_RPMK
	    if (bigMode & MODES_K)
		ka->checksigFlags &= ~CHECKSIG_MD5;
	    else
#endif
#ifdef	IAM_RPMEIU
	    if (bigMode & MODES_IE)
		ia->transFlags |= RPMTRANS_FLAG_NOMD5;
	    else
#endif
		{};
	    break;
#endif	/* IAM_RPMQV || IAM_RPMK */

#if defined(IAM_RPMQV) || defined(IAM_RPMEIU)
	case POPT_NODEPS:
#ifdef	IAM_RPMQV
	    if (bigMode == MODE_VERIFY || qva->qva_mode == 'V')
		qva->qva_flags |= VERIFY_DEPS;
	    else
#endif
#ifdef	IAM_RPMEIU
	    if ((bigMode & MODES_IE) ||
		(ia->installInterfaceFlags &
	    (INSTALL_UPGRADE|INSTALL_FRESHEN|INSTALL_INSTALL|INSTALL_ERASE)))
		ia->noDeps = 1;
	    else
#endif
		/*@-ifempty@*/ ;
	    break;

	case POPT_FORCE:
#ifdef	IAM_RPMEIU
	    if ((bigMode & MODES_IE) ||
		(ia->installInterfaceFlags &
	    (INSTALL_UPGRADE|INSTALL_FRESHEN|INSTALL_INSTALL|INSTALL_ERASE)))
		ia->probFilter |=
			( RPMPROB_FILTER_REPLACEPKG
			| RPMPROB_FILTER_REPLACEOLDFILES
			| RPMPROB_FILTER_REPLACENEWFILES
			| RPMPROB_FILTER_OLDPACKAGE);
	    else
#endif
		/*@-ifempty@*/ ;
	    break;

	case 'i':
#ifdef	IAM_RPMQV
	    if (bigMode == MODE_QUERY || qva->qva_mode == 'q') {
		/*@-nullassign -readonlytrans@*/
		const char * infoCommand[] = { "--info", NULL };
		/*@=nullassign =readonlytrans@*/
		(void) poptStuffArgs(optCon, infoCommand);
	    } else
#endif
#ifdef	IAM_RPMEIU
	    if (bigMode == MODE_INSTALL ||
		(ia->installInterfaceFlags &
		    (INSTALL_UPGRADE|INSTALL_FRESHEN|INSTALL_INSTALL)))
		/*@-ifempty@*/ ;
	    else if (bigMode == MODE_UNKNOWN) {
		/*@-nullassign -readonlytrans@*/
		const char * installCommand[] = { "--install", NULL };
		/*@=nullassign =readonlytrans@*/
		(void) poptStuffArgs(optCon, installCommand);
	    } else
#endif
		/*@-ifempty@*/ ;
	    break;

	case POPT_NOSCRIPTS:
#ifdef	IAM_RPMQV
	    if (bigMode == MODE_VERIFY || qva->qva_mode == 'V')
		qva->qva_flags |= VERIFY_SCRIPT;
	    else
#endif
#ifdef	IAM_RPMEIU
	    if ((bigMode & MODES_IE) ||
		(ia->installInterfaceFlags &
	    (INSTALL_UPGRADE|INSTALL_FRESHEN|INSTALL_INSTALL|INSTALL_ERASE)))
		ia->transFlags |= (_noTransScripts | _noTransTriggers);
	    else
#endif
		/*@-ifempty@*/ ;
	    break;

#endif	/* IAM_RPMQV || IAM_RPMEIU */

	case GETOPT_DEFINEMACRO:
	    if (optArg) {
		(void) rpmDefineMacro(NULL, optArg, RMIL_CMDLINE);
		(void) rpmDefineMacro(rpmCLIMacroContext, optArg,RMIL_CMDLINE);
	    }
	    noUsageMsg = 1;
	    break;

	case GETOPT_EVALMACRO:
	    if (optArg) {
		const char *val = rpmExpand(optArg, NULL);
		fprintf (stderr, "%s\n", val);
		val = _free(val);
	    }
	    noUsageMsg = 1;
	    break;

#if defined(GETOPT_RCFILE)
	case GETOPT_RCFILE:
	    fprintf(stderr, _("The --rcfile option has been eliminated.\n"));
	    fprintf(stderr, _("Use \"--macros <file:...>\" instead.\n"));
	    return EXIT_FAILURE;
	    /*@notreached@*/ break;
#endif

	default:
	    fprintf(stderr, _("Internal error in argument processing (%d) :-(\n"), arg);
	    return EXIT_FAILURE;
	}
    }

    if (quiet)
	rpmSetVerbosity(RPMMESS_QUIET);

    if (arg < -1) {
	fprintf(stderr, "%s: %s\n", 
		poptBadOption(optCon, POPT_BADOPTION_NOALIAS), 
		poptStrerror(arg));
	return EXIT_FAILURE;
    }

#ifdef	IAM_RPMDB
  if (bigMode == MODE_UNKNOWN || (bigMode & MODES_DB)) {
    if (da->init) {
	if (bigMode != MODE_UNKNOWN) {
	    argerror(_("only one major mode may be specified"));
	    return RPM_WRONGOPT;
	} else
	    bigMode = MODE_INITDB;
    } else
    if (da->rebuild) {
	if (bigMode != MODE_UNKNOWN) {
	    argerror(_("only one major mode may be specified"));
	    return RPM_WRONGOPT;
	} else
	    bigMode = MODE_REBUILDDB;
    } else
    if (da->verify) {
	if (bigMode != MODE_UNKNOWN) {
	    argerror(_("only one major mode may be specified"));
	    return RPM_WRONGOPT;
	} else
	    bigMode = MODE_VERIFYDB;
    }
  }
#endif	/* IAM_RPMDB */

#ifdef	IAM_RPMQV
  if (bigMode == MODE_UNKNOWN || (bigMode & MODES_QV)) {
    switch (qva->qva_mode) {
    case 'q':	bigMode = MODE_QUERY;		break;
    case 'V':	bigMode = MODE_VERIFY;		break;
    case 'Q':	bigMode = MODE_QUERYTAGS;	break;
    }

    if (qva->qva_sourceCount) {
	if (qva->qva_sourceCount > 2) {
	    argerror(_("one type of query/verify may be performed at a "
			"time"));
	    return RPM_WRONGOPT;
	}
    }
    if (qva->qva_flags && (bigMode & ~MODES_QV)) {
	argerror(_("unexpected query flags"));
	return RPM_WRONGOPT;
    }

    if (qva->qva_queryFormat && (bigMode & ~MODES_QV)) {
	argerror(_("unexpected query format"));
	return RPM_WRONGOPT;
    }

    if (qva->qva_source != RPMQV_PACKAGE && (bigMode & ~MODES_QV)) {
	argerror(_("unexpected query source"));
	return RPM_WRONGOPT;
    }
  }
#endif	/* IAM_RPMQV */

#ifdef	IAM_RPMEIU
  if (bigMode == MODE_UNKNOWN || (bigMode & MODES_IE))
    {	int iflags = (ia->installInterfaceFlags &
		(INSTALL_UPGRADE|INSTALL_FRESHEN|INSTALL_INSTALL));
	int eflags = (ia->installInterfaceFlags & INSTALL_ERASE);

	if (iflags & eflags) {
	    argerror(_("only one major mode may be specified"));
	    return RPM_WRONGOPT;
	}
	else if (iflags)
	    bigMode = MODE_INSTALL;
	else if (eflags)
	    bigMode = MODE_ERASE;
    }
#endif	/* IAM_RPMQV */

#ifdef	IAM_RPMK
  if (bigMode == MODE_UNKNOWN || (bigMode & MODES_K)) {
	switch (ka->addSign) {
	case RESIGN_NONE:
	    break;
	case RESIGN_CHK_SIGNATURE:
	    bigMode = MODE_CHECKSIG;
	    break;
	case RESIGN_ADD_SIGNATURE:
	case RESIGN_NEW_SIGNATURE:
	    bigMode = MODE_RESIGN;
	    break;

	}
  }
#endif	/* IAM_RPMK */

    /* XXX TODO: never happens. */
    if (gotDbpath && (bigMode & ~MODES_FOR_DBPATH)) {
	argerror(_("--dbpath given for operation that does not use a "
			"database"));
	return RPM_WRONGOPT;
    }

#if defined(IAM_RPMEIU)
    if (!( bigMode == MODE_INSTALL ) &&
(ia->probFilter & (RPMPROB_FILTER_REPLACEPKG | RPMPROB_FILTER_REPLACEOLDFILES | RPMPROB_FILTER_REPLACENEWFILES | RPMPROB_FILTER_OLDPACKAGE))) {
	argerror(_("only installation, upgrading, rmsource and rmspec may be forced"));
	return RPM_WRONGOPT;
    }
    if (bigMode != MODE_INSTALL && (ia->probFilter & RPMPROB_FILTER_FORCERELOCATE)) {
	argerror(_("files may only be relocated during package installation"));
	return RPM_WRONGOPT;
    }

    if (ia->relocations && ia->prefix) {
	argerror(_("only one of --prefix or --relocate may be used"));
	return RPM_WRONGOPT;
    }

    if (bigMode != MODE_INSTALL && ia->relocations) {
	argerror(_("--relocate and --excludepath may only be used when installing new packages"));
	return RPM_WRONGOPT;
    }

    if (bigMode != MODE_INSTALL && ia->prefix) {
	argerror(_("--prefix may only be used when installing new packages"));
	return RPM_WRONGOPT;
    }

    if (ia->prefix && ia->prefix[0] != '/') {
	argerror(_("arguments to --prefix must begin with a /"));
	return RPM_WRONGOPT;
    }

    if (bigMode != MODE_INSTALL && (ia->installInterfaceFlags & INSTALL_HASH)) {
	argerror(_("--hash (-h) may only be specified during package "
			"installation"));
	return RPM_WRONGOPT;
    }

    if (bigMode != MODE_INSTALL && (ia->installInterfaceFlags & INSTALL_PERCENT)) {
	argerror(_("--percent may only be specified during package "
			"installation"));
	return RPM_WRONGOPT;
    }

    if (bigMode != MODE_INSTALL &&
 (ia->probFilter & (RPMPROB_FILTER_REPLACEOLDFILES|RPMPROB_FILTER_REPLACENEWFILES))) {
	argerror(_("--replacefiles may only be specified during package "
			"installation"));
	return RPM_WRONGOPT;
    }

    if (bigMode != MODE_INSTALL && (ia->probFilter & RPMPROB_FILTER_REPLACEPKG)) {
	argerror(_("--replacepkgs may only be specified during package "
			"installation"));
	return RPM_WRONGOPT;
    }

    if (bigMode != MODE_INSTALL && (ia->transFlags & RPMTRANS_FLAG_NODOCS)) {
	argerror(_("--excludedocs may only be specified during package "
		   "installation"));
	return RPM_WRONGOPT;
    }

    if (bigMode != MODE_INSTALL && ia->incldocs) {
	argerror(_("--includedocs may only be specified during package "
		   "installation"));
	return RPM_WRONGOPT;
    }

    if (ia->incldocs && (ia->transFlags & RPMTRANS_FLAG_NODOCS)) {
	argerror(_("only one of --excludedocs and --includedocs may be "
		 "specified"));
	return RPM_WRONGOPT;
    }
  
    if (bigMode != MODE_INSTALL && (ia->probFilter & RPMPROB_FILTER_IGNOREARCH)) {
	argerror(_("--ignorearch may only be specified during package "
		   "installation"));
	return RPM_WRONGOPT;
    }

    if (bigMode != MODE_INSTALL && (ia->probFilter & RPMPROB_FILTER_IGNOREOS)) {
	argerror(_("--ignoreos may only be specified during package "
		   "installation"));
	return RPM_WRONGOPT;
    }

    if (bigMode != MODE_INSTALL &&
	(ia->probFilter & (RPMPROB_FILTER_DISKSPACE|RPMPROB_FILTER_DISKNODES))) {
	argerror(_("--ignoresize may only be specified during package "
		   "installation"));
	return RPM_WRONGOPT;
    }

    if ((ia->eraseInterfaceFlags & UNINSTALL_ALLMATCHES) && bigMode != MODE_ERASE) {
	argerror(_("--allmatches may only be specified during package "
		   "erasure"));
	return RPM_WRONGOPT;
    }

    if ((ia->transFlags & RPMTRANS_FLAG_ALLFILES) && bigMode != MODE_INSTALL) {
	argerror(_("--allfiles may only be specified during package "
		   "installation"));
	return RPM_WRONGOPT;
    }

    if ((ia->transFlags & RPMTRANS_FLAG_JUSTDB) &&
	bigMode != MODE_INSTALL && bigMode != MODE_ERASE) {
	argerror(_("--justdb may only be specified during package "
		   "installation and erasure"));
	return RPM_WRONGOPT;
    }

    if (bigMode != MODE_INSTALL && bigMode != MODE_ERASE &&
	(ia->transFlags & (RPMTRANS_FLAG_NOSCRIPTS | _noTransScripts | _noTransTriggers))) {
	argerror(_("script disabling options may only be specified during "
		   "package installation and erasure"));
	return RPM_WRONGOPT;
    }

    if (bigMode != MODE_INSTALL && bigMode != MODE_ERASE &&
	(ia->transFlags & (RPMTRANS_FLAG_NOTRIGGERS | _noTransTriggers))) {
	argerror(_("trigger disabling options may only be specified during "
		   "package installation and erasure"));
	return RPM_WRONGOPT;
    }

    if (ia->noDeps & (bigMode & ~MODES_FOR_NODEPS)) {
	argerror(_("--nodeps may only be specified during package "
		   "building, rebuilding, recompilation, installation,"
		   "erasure, and verification"));
	return RPM_WRONGOPT;
    }

    if ((ia->transFlags & RPMTRANS_FLAG_TEST) && (bigMode & ~MODES_FOR_TEST)) {
	argerror(_("--test may only be specified during package installation, "
		 "erasure, and building"));
	return RPM_WRONGOPT;
    }
#endif	/* IAM_RPMEIU */

    if (rootdir && rootdir[1] && (bigMode & ~MODES_FOR_ROOT)) {
	argerror(_("--root (-r) may only be specified during "
		 "installation, erasure, querying, and "
		 "database rebuilds"));
	return RPM_WRONGOPT;
    }

    if (rootdir) {
	switch (urlIsURL(rootdir)) {
	default:
	    if (bigMode & MODES_FOR_ROOT)
		break;
	    /*@fallthrough@*/
	case URL_IS_UNKNOWN:
	    if (rootdir[0] != '/') {
		argerror(_("arguments to --root (-r) must begin with a /"));
		return RPM_WRONGOPT;
	    }
	    break;
	}
    }

#if defined(IAM_RPMK)
    if (0 || ka->sign) {
        if (bigMode == MODE_REBUILD || bigMode == MODE_BUILD ||
	    bigMode == MODE_RESIGN || bigMode == MODE_TARBUILD) {
	    const char ** av;
	    struct stat sb;
	    int errors = 0;

	    if ((av = poptGetArgs(optCon)) == NULL) {
		fprintf(stderr, _("no files to sign\n"));
		errors++;
	    } else
	    while (*av) {
		if (stat(*av, &sb)) {
		    fprintf(stderr, _("cannot access file %s\n"), *av);
		    errors++;
		}
		av++;
	    }

	    if (errors) {
		ec = errors;
		goto exit;
	    }

            if (poptPeekArg(optCon)) {
		int sigTag;
		switch (sigTag = rpmLookupSignatureType(RPMLOOKUPSIG_QUERY)) {
		  case 0:
		    break;
		  case RPMSIGTAG_PGP:
		    if ((sigTag == RPMSIGTAG_PGP || sigTag == RPMSIGTAG_PGP5) &&
		        !rpmDetectPGPVersion(NULL)) {
		        fprintf(stderr, _("pgp not found: "));
			ec = EXIT_FAILURE;
			goto exit;
		    }	/*@fallthrough@*/
		  case RPMSIGTAG_GPG:
		    passPhrase = rpmGetPassPhrase(_("Enter pass phrase: "), sigTag);
		    if (passPhrase == NULL) {
			fprintf(stderr, _("Pass phrase check failed\n"));
			ec = EXIT_FAILURE;
			goto exit;
		    }
		    fprintf(stderr, _("Pass phrase is good.\n"));
		    passPhrase = xstrdup(passPhrase);
		    break;
		  default:
		    fprintf(stderr,
		            _("Invalid %%_signature spec in macro file.\n"));
		    ec = EXIT_FAILURE;
		    goto exit;
		    /*@notreached@*/ break;
		}
	    }
	} else {
	    argerror(_("--sign may only be used during package building"));
	    return RPM_WRONGOPT;
	}
    } else {
    	/* Make rpmLookupSignatureType() return 0 ("none") from now on */
        (void) rpmLookupSignatureType(RPMLOOKUPSIG_DISABLE);
    }
#endif	/* IAM_RPMK */

    if (pipeOutput) {
	(void) pipe(p);

	if (!(pipeChild = fork())) {
	    (void) close(p[1]);
	    (void) dup2(p[0], STDIN_FILENO);
	    (void) close(p[0]);
	    (void) execl("/bin/sh", "/bin/sh", "-c", pipeOutput, NULL);
	    fprintf(stderr, _("exec failed\n"));
	}

	(void) close(p[0]);
	(void) dup2(p[1], STDOUT_FILENO);
	(void) close(p[1]);
    }
	
    switch (bigMode) {
#ifdef	IAM_RPMDB
    case MODE_INITDB:
	(void) rpmdbInit(rootdir, 0644);
	break;

    case MODE_REBUILDDB:
	ec = rpmdbRebuild(rootdir);
	break;
    case MODE_VERIFYDB:
	ec = rpmdbVerify(rootdir);
	break;
#endif	/* IAM_RPMDB */

#ifdef	IAM_RPMEIU
    case MODE_ERASE:
	ia->rootdir = rootdir;
	if (ia->noDeps) ia->eraseInterfaceFlags |= UNINSTALL_NODEPS;

	if (!poptPeekArg(optCon)) {
	    if (ia->rbtid == 0) {
		argerror(_("no packages given for erase"));
		return RPM_NOPKG;
	    }
ia->transFlags |= RPMTRANS_FLAG_NOMD5;
ia->probFilter |= RPMPROB_FILTER_OLDPACKAGE;
	    ec += rpmRollback(ia, NULL);
	} else {
	    ec += rpmErase_p(rootdir, (const char **)poptGetArgs(optCon), 
			 ia->transFlags, ia->eraseInterfaceFlags);
	}
	break;

    case MODE_INSTALL:
	/* RPMTRANS_FLAG_BUILD_PROBS */
	/* RPMTRANS_FLAG_KEEPOBSOLETE */
	ia->rootdir = rootdir;
	if (!ia->incldocs) {
	    if (ia->transFlags & RPMTRANS_FLAG_NODOCS)
		;
	    else if (rpmExpandNumeric("%{_excludedocs}"))
		ia->transFlags |= RPMTRANS_FLAG_NODOCS;
	}

	if (ia->noDeps) ia->installInterfaceFlags |= INSTALL_NODEPS;

	/* we've already ensured !(!ia->prefix && !ia->relocations) */
	if (ia->prefix) {
	    ia->relocations = xmalloc(2 * sizeof(*ia->relocations));
	    ia->relocations[0].oldPath = NULL;   /* special case magic */
	    ia->relocations[0].newPath = ia->prefix;
	    ia->relocations[1].oldPath = ia->relocations[1].newPath = NULL;
	} else if (ia->relocations) {
	    ia->relocations = xrealloc(ia->relocations, 
			sizeof(*ia->relocations) * (ia->numRelocations + 1));
	    ia->relocations[ia->numRelocations].oldPath = NULL;
	    ia->relocations[ia->numRelocations].newPath = NULL;
	}

	if (!poptPeekArg(optCon)) {
	    if (ia->rbtid == 0) {
		argerror(_("no packages given for install"));
		return RPM_NOPKG;
	    }
	    ia->transFlags |= RPMTRANS_FLAG_NOMD5;
	    ia->probFilter |= RPMPROB_FILTER_OLDPACKAGE;
	    ec += rpmRollback(ia, NULL);
	} else {

	    /*@-compdef@*/ /* FIX: ia->relocations[0].newPath undefined */
	    ec += rpmInstall_p(rootdir, (const char **)poptGetArgs(optCon), 
			ia->transFlags, ia->installInterfaceFlags,
			ia->probFilter, ia->relocations);
	}
	/*@=compdef@*/
	break;
#endif	/* IAM_RPMEIU */

#ifdef	IAM_RPMQV
    case MODE_QUERY:
      { const char * pkg;

	qva->qva_prefix = rootdir;
	if (qva->qva_source == RPMQV_ALL) {
#ifdef	DYING
	    if (poptPeekArg(optCon)) {
		argerror(_("extra arguments given for query of all packages"));
		return RPM_WRONGOPT;
	    }
#else
	    const char ** av = poptGetArgs(optCon);
#endif
	    /*@-nullpass@*/	/* FIX: av can be NULL */
	    ec = rpmQuery_p(qva, RPMQV_ALL, (const char *) av);
	    /*@=nullpass@*/
	} else {
	    if (!poptPeekArg(optCon)) {
		argerror(_("no arguments given for query"));
		return RPM_WRONGOPT;
	    }
	    while ((pkg = poptGetArg(optCon)))
		ec += rpmQuery_p(qva, qva->qva_source, pkg);
	}
      } break;

    case MODE_VERIFY:
      { const char * pkg;
	rpmVerifyFlags verifyFlags = VERIFY_ALL;

	verifyFlags &= ~qva->qva_flags;
	qva->qva_flags = (rpmQueryFlags) verifyFlags;
	qva->qva_prefix = rootdir;

	if (qva->qva_source == RPMQV_ALL) {
	    if (poptPeekArg(optCon)) {
		argerror(_("extra arguments given for verify of all packages"));
		return RPM_WRONGOPT;
	    }
	    ec = rpmVerify_p(qva, RPMQV_ALL, NULL);
	} else {
	    if (!poptPeekArg(optCon)) {
		argerror(_("no arguments given for verify"));
		return RPM_WRONGOPT;
	    }
	    while ((pkg = poptGetArg(optCon)))
		ec += rpmVerify_p(qva, qva->qva_source, pkg);
	}
      }	break;

    case MODE_QUERYTAGS:
	if (argc != 2) {
	    argerror(_("unexpected arguments to --querytags "));
	    return RPM_WRONGOPT;
	}

	rpmDisplayQueryTags(stdout);
	break;
#endif	/* IAM_RPMQV */

#ifdef IAM_RPMK
    case MODE_CHECKSIG:
	if (!poptPeekArg(optCon)) {
	    argerror(_("no packages given for signature check"));
	    return RPM_NOPKG;
	}
	ec = rpmCheckSig_p(ka->checksigFlags,
			(const char **)poptGetArgs(optCon));
	/* XXX don't overflow single byte exit status */
	if (ec > 255) ec = 255;
	break;

    case MODE_RESIGN:
	if (!poptPeekArg(optCon)) {
	    argerror(_("no packages given for signing"));
	    return RPM_NOPKG;
	}
	ec = rpmReSign(ka->addSign, passPhrase,
			(const char **)poptGetArgs(optCon));
	/* XXX don't overflow single byte exit status */
	if (ec > 255) ec = 255;
	break;
#endif	/* IAM_RPMK */
	
#if !defined(IAM_RPMQV)
    case MODE_QUERY:
    case MODE_VERIFY:
    case MODE_QUERYTAGS:
#endif
#if !defined(IAM_RPMK)
    case MODE_CHECKSIG:
    case MODE_RESIGN:
#endif
#if !defined(IAM_RPMDB)
    case MODE_INITDB:
    case MODE_REBUILDDB:
    case MODE_VERIFYDB:
#endif
#if !defined(IAM_RPMEIU)
    case MODE_INSTALL:
    case MODE_ERASE:
#endif
    case MODE_UNKNOWN:
	break;
    }

#if defined(IAM_RPMK)
exit:
#endif	/* IAM_RPMK */
    optCon = poptFreeContext(optCon);
    rpmFreeMacros(NULL);
    rpmFreeMacros(rpmCLIMacroContext);
    rpmFreeRpmrc();

    if (pipeChild) {
	(void) fclose(stdout);
	(void) waitpid(pipeChild, &status, 0);
    }

    /* keeps memory leak checkers quiet */
    freeNames();
    freeFilesystems();
    urlFreeCache();
    rpmlogClose();
    dbiTags = _free(dbiTags);

#ifdef	IAM_RPMQV
    qva->qva_queryFormat = _free(qva->qva_queryFormat);
#endif

#ifdef	IAM_RPMEIU
    ia->relocations = _free(ia->relocations);
#endif

#if HAVE_MCHECK_H && HAVE_MTRACE
    muntrace();   /* Trace malloc only if MALLOC_TRACE=mtrace-output-file. */
#endif
    /*@-globstate@*/
    return ec;
    /*@=globstate@*/
}
