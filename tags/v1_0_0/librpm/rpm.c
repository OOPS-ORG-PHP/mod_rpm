#include <system.h>

#include <rpmcli.h>
#include <rpmbuild.h>

#include <build.h>
#include <signature.h>
#include <debug.h>

#include <defext.h>
#include <stdarg.h>
void rpm_last_message (int stream, char *format, ...);

#undef stderr
#define stderr 0
#define fprintf rpm_last_message

#define GETOPT_ADDSIGN		1005
#define GETOPT_RESIGN		1006
#define GETOPT_DBPATH		1010
#define GETOPT_REBUILDDB	1013
#define GETOPT_INSTALL		1014
#define GETOPT_RELOCATE		1016
#define GETOPT_SHOWRC		1018
#define GETOPT_EXCLUDEPATH	1019
#define	GETOPT_DEFINEMACRO	1020
#define	GETOPT_EVALMACRO	1021
#define	GETOPT_RCFILE		1022
#define GETOPT_VERIFYDB		1023

enum modes {
    MODE_UNKNOWN	= 0,
    MODE_QUERY		= (1 <<  0),
    MODE_INSTALL	= (1 <<  1),
    MODE_ERASE		= (1 <<  2),
    MODE_VERIFY		= (1 <<  3),
    MODE_BUILD		= (1 <<  4),
    MODE_REBUILD	= (1 <<  5),
    MODE_CHECKSIG	= (1 <<  6),
    MODE_RESIGN		= (1 <<  7),
    MODE_RECOMPILE	= (1 <<  8),
    MODE_QUERYTAGS	= (1 <<  9),
    MODE_INITDB		= (1 << 10),
    MODE_TARBUILD	= (1 << 11),
    MODE_REBUILDDB	= (1 << 12),
    MODE_VERIFYDB	= (1 << 13)
};

#define	MODES_QV (MODE_QUERY | MODE_VERIFY)
#define	MODES_BT (MODE_BUILD | MODE_TARBUILD | MODE_REBUILD | MODE_RECOMPILE)
#define	MODES_IE (MODE_INSTALL | MODE_ERASE)
#define	MODES_DB (MODE_INITDB | MODE_REBUILDDB | MODE_VERIFYDB)
#define	MODES_K	 (MODE_CHECKSIG | MODES_RESIGN)

#define	MODES_FOR_DBPATH	(MODES_BT | MODES_IE | MODES_QV | MODES_DB)
#define	MODES_FOR_NODEPS	(MODES_BT | MODES_IE | MODE_VERIFY)
#define	MODES_FOR_TEST		(MODES_BT | MODES_IE)
#define	MODES_FOR_ROOT		(MODES_BT | MODES_IE | MODES_QV | MODES_DB)

/* the flags for the various options */
static int allFiles;
static int allMatches;
static int applyOnly;
static int badReloc;
static int dirStash;
static int excldocs;
static int force;
extern int _fsm_debug;
extern int _ftp_debug;
static int showHash;
static int help;
static int ignoreArch;
static int ignoreOs;
static int ignoreSize;
static int incldocs;
static int initdb;
static int justdb;
static int noDeps;
static int noGpg;
extern int noLibio;
static int noMd5;
static int noOrder;
static int noPgp;

static int noScripts;
static int noPre;
static int noPost;
static int noPreun;
static int noPostun;

static int noTriggers;
static int noTPrein;
static int noTIn;
static int noTUn;
static int noTPostun;

static int noUsageMsg;
static int oldPackage;
static char * pipeOutput;
static char * prefix;
static int quiet;
static char * rcfile;

static int rePackage;
static int pkgCommit;
static int pkgUndo;
static int tsCommit;
static int tsUndo;

static int replaceFiles;
static int replacePackages;
static char * rootdir;
extern int _rpmio_debug;
static int showPercents;
static int showrc;
static int signIt;
static int test;
extern int _url_debug;
extern int _noDirTokens;
extern int _useDbiMajor;

static int showVersion;

//extern struct MacroContext_s rpmCLIMacroContext;

extern struct rpmBuildArguments_s	rpmBTArgs;

/* the structure describing the options we take and the defaults */
static struct poptOption optionsTable[] = {
 { "addsign", '\0', 0, 0, GETOPT_ADDSIGN,	NULL, NULL},
 { "allfiles", '\0', 0, &allFiles, 0,		NULL, NULL},
 { "allmatches", '\0', 0, &allMatches, 0,	NULL, NULL},
 { "apply", '\0', 0, &applyOnly, 0,		NULL, NULL},
 { "badreloc", '\0', 0, &badReloc, 0,		NULL, NULL},
 { "checksig", 'K', 0, 0, 'K',			NULL, NULL},
 { "define", '\0', POPT_ARG_STRING, 0, GETOPT_DEFINEMACRO,NULL, NULL},
 { "dirstash", '\0', POPT_ARG_VAL, &dirStash, 1,	NULL, NULL},
 { "dirtokens", '\0', POPT_ARG_VAL, &_noDirTokens, 0,	NULL, NULL},
 { "erase", 'e', 0, 0, 'e',			NULL, NULL},
 { "eval", '\0', POPT_ARG_STRING, 0, GETOPT_EVALMACRO, NULL, NULL},
 { "excludedocs", '\0', 0, &excldocs, 0,	NULL, NULL},
 { "excludepath", '\0', POPT_ARG_STRING, 0, GETOPT_EXCLUDEPATH,	NULL, NULL},
 { "force", '\0', 0, &force, 0,			NULL, NULL},
 { "freshen", 'F', 0, 0, 'F',			NULL, NULL},
 { "fsmdebug", '\0', POPT_ARG_VAL, &_fsm_debug, -1,		NULL, NULL},
 { "ftpdebug", '\0', POPT_ARG_VAL, &_ftp_debug, -1,		NULL, NULL},
 { "hash", 'h', 0, &showHash, 0,		NULL, NULL},
 { "help", '\0', 0, &help, 0,			NULL, NULL},
 {  NULL, 'i', 0, 0, 'i',			NULL, NULL},
 { "ignorearch", '\0', 0, &ignoreArch, 0,	NULL, NULL},
 { "ignoreos", '\0', 0, &ignoreOs, 0,		NULL, NULL},
 { "ignoresize", '\0', 0, &ignoreSize, 0,	NULL, NULL},
 { "includedocs", '\0', 0, &incldocs, 0,	NULL, NULL},
 { "initdb", '\0', 0, &initdb, 0,		NULL, NULL},
/* info and install both using 'i' is dumb */
 { "install", '\0', 0, 0, GETOPT_INSTALL,	NULL, NULL},
 { "justdb", '\0', 0, &justdb, 0,		NULL, NULL},
 { "macros", '\0', POPT_ARG_STRING, &macrofiles, 0,	NULL, NULL},
 { "nodeps", '\0', 0, &noDeps, 0,		NULL, NULL},
 { "nodirtokens", '\0', POPT_ARG_VAL, &_noDirTokens, 1,	NULL, NULL},
 { "nogpg", '\0', 0, &noGpg, 0,			NULL, NULL},
#if HAVE_LIBIO_H && defined(_G_IO_IO_FILE_VERSION)
 { "nolibio", '\0', POPT_ARG_VAL, &noLibio, 1,		NULL, NULL},
#endif
 { "nomd5", '\0', 0, &noMd5, 0,			NULL, NULL},
 { "noorder", '\0', 0, &noOrder, 0,		NULL, NULL},
 { "nopgp", '\0', 0, &noPgp, 0,			NULL, NULL},

 { "noscripts", '\0', 0, &noScripts, 0,		NULL, NULL},
 { "nopre", '\0', 0, &noPre, 0,			NULL, NULL},
 { "nopost", '\0', 0, &noPost, 0,		NULL, NULL},
 { "nopreun", '\0', 0, &noPreun, 0,		NULL, NULL},
 { "nopostun", '\0', 0, &noPostun, 0,		NULL, NULL},

 { "notriggers", '\0', 0, &noTriggers, 0,	NULL, NULL},
 { "notriggerprein", '\0', 0, &noTPrein, 0,	NULL, NULL},
 { "notriggerin", '\0', 0, &noTIn, 0,		NULL, NULL},
 { "notriggerun", '\0', 0, &noTUn, 0,		NULL, NULL},
 { "notriggerpostun", '\0', 0, &noTPostun, 0,	NULL, NULL},

 { "oldpackage", '\0', 0, &oldPackage, 0,	NULL, NULL},
 { "percent", '\0', 0, &showPercents, 0,	NULL, NULL},
 { "pipe", '\0', POPT_ARG_STRING, &pipeOutput, 0,	NULL, NULL},
 { "pkgcommit", '\0', POPT_ARG_VAL, &pkgCommit, 1,	NULL, NULL},
 { "pkgundo", '\0', POPT_ARG_VAL, &pkgUndo, 1,	NULL, NULL},
 { "prefix", '\0', POPT_ARG_STRING, &prefix, 0,	NULL, NULL},
 { "quiet", '\0', 0, &quiet, 0,			NULL, NULL},
#ifndef	DYING
 { "rcfile", '\0', POPT_ARG_STRING, &rcfile, 0,	NULL, NULL},
#else
 { "rcfile", '\0', 0, 0, GETOPT_RCFILE,		NULL, NULL},
#endif
 { "rebuilddb", '\0', 0, 0, GETOPT_REBUILDDB,	NULL, NULL},
 { "verifydb", '\0', 0, 0, GETOPT_VERIFYDB,	NULL, NULL},
 { "relocate", '\0', POPT_ARG_STRING, 0, GETOPT_RELOCATE,	NULL, NULL},
 { "repackage", '\0', POPT_ARG_VAL, &rePackage, 1,	NULL, NULL},
 { "replacefiles", '\0', 0, &replaceFiles, 0,	NULL, NULL},
 { "replacepkgs", '\0', 0, &replacePackages, 0,	NULL, NULL},
 { "resign", '\0', 0, 0, GETOPT_RESIGN,		NULL, NULL},
 { "root", 'r', POPT_ARG_STRING, &rootdir, 0,	NULL, NULL},
 { "rpmiodebug", '\0', POPT_ARG_VAL, &_rpmio_debug, -1,		NULL, NULL},
 { "showrc", '\0', 0, &showrc, GETOPT_SHOWRC,	NULL, NULL},
 { "sign", '\0', 0, &signIt, 0,			NULL, NULL},
 { "test", '\0', 0, &test, 0,			NULL, NULL},
 { "commit", '\0', POPT_ARG_VAL, &tsCommit, 1,	NULL, NULL},
 { "undo", '\0', POPT_ARG_VAL, &tsUndo, 1,	NULL, NULL},
 { "upgrade", 'U', 0, 0, 'U',			NULL, NULL},
 { "urldebug", '\0', POPT_ARG_VAL, &_url_debug, -1,		NULL, NULL},
 { "uninstall", 'u', 0, 0, 'u',			NULL, NULL},
 { "verbose", 'v', 0, 0, 'v',			NULL, NULL},
 { "version", '\0', 0, &showVersion, 0,		NULL, NULL},

 { NULL, '\0', POPT_ARG_INCLUDE_TABLE, 
		rpmQVSourcePoptTable, 0,	(void *) &rpmQVArgs, NULL },
 { NULL, '\0', POPT_ARG_INCLUDE_TABLE, 
		rpmQueryPoptTable, 0,		(void *) &rpmQVArgs, NULL },
 { NULL, '\0', POPT_ARG_INCLUDE_TABLE, 
		rpmVerifyPoptTable, 0,		(void *) &rpmQVArgs, NULL },

 { NULL, '\0', POPT_ARG_INCLUDE_TABLE,
		rpmBuildPoptTable, 0,           (void *) &rpmBTArgs, NULL },

 { 0, 0, 0, 0, 0,	NULL, NULL }
};

#ifdef __MINT__
/* MiNT cannot dynamically increase the stack.  */
long _stksize = 64 * 1024L;
#endif

static void argerror(const char * desc) {
    fprintf(stderr, _("rpm: %s\n"), desc);
}

int __rpmexec (int argc, const char ** argv)
{
    enum modes bigMode = MODE_UNKNOWN;
    QVA_t qva = &rpmQVArgs;
    int arg;
    rpmtransFlags transFlags = RPMTRANS_FLAG_NONE;
    rpmInstallInterfaceFlags installInterfaceFlags = INSTALL_NONE;
    rpmEraseInterfaceFlags eraseInterfaceFlags = UNINSTALL_NONE;
    int verifyFlags;
    int checksigFlags = 0;
    rpmResignFlags addSign = RESIGN_NEW_SIGNATURE;
    char * passPhrase = "";
    const char * optArg;
    pid_t pipeChild = 0;
    const char * pkg;
    char * errString = NULL;
    poptContext optCon;
    int ec = 0;
    int status;
    int p[2];
    rpmRelocation * relocations = NULL;
    int numRelocations = 0;
    int sigTag;
    int upgrade = 0;
    int freshen = 0;
    int probFilter = 0;
	
#if HAVE_MCHECK_H && HAVE_MTRACE
    mtrace();	/* Trace malloc only if MALLOC_TRACE=mtrace-output-file. */
#endif
    setprogname(argv[0]);	/* Retrofit glibc __progname */

    /* set the defaults for the various command line options */
    allFiles = 0;
    allMatches = 0;
    applyOnly = 0;
    badReloc = 0;
    excldocs = 0;
    force = 0;
    _ftp_debug = 0;
    showHash = 0;
    help = 0;
    ignoreArch = 0;
    ignoreOs = 0;
    ignoreSize = 0;
    incldocs = 0;
    initdb = 0;
    justdb = 0;
    noDeps = 0;
    noGpg = 0;
#if HAVE_LIBIO_H && defined(_G_IO_IO_FILE_VERSION)
    noLibio = 0;
#else
    noLibio = 1;
#endif
    noMd5 = 0;
    noOrder = 0;
    noPgp = 0;

    noScripts = 0;
    noPre = 0;
    noPost = 0;
    noPreun = 0;
    noPostun = 0;

    noTriggers = 0;
    noTPrein = 0;
    noTIn = 0;
    noTUn = 0;
    noTPostun = 0;

    noUsageMsg = 0;
    oldPackage = 0;
    showPercents = 0;
    pipeOutput = NULL;
    prefix = NULL;
    quiet = 0;
    _rpmio_debug = 0;
    replaceFiles = 0;
    replacePackages = 0;
    rootdir = "/";
    showrc = 0;
    signIt = 0;
    showVersion = 0;
    specedit = 0;
    test = 0;
    _url_debug = 0;

    /* XXX Eliminate query linkage loop */
    parseSpecVec = parseSpec;
    freeSpecVec = freeSpec;

    /* set up the correct locale */
    setlocale(LC_ALL, "" );

#ifdef	__LCLINT__
#define	LOCALEDIR	"/usr/share/locale"
#endif
    bindtextdomain(PACKAGE, LOCALEDIR);
    textdomain(PACKAGE);

    rpmSetVerbosity(RPMMESS_NORMAL);	/* XXX silly use by showrc */

    /* Make a first pass through the arguments, looking for --rcfile */
    /* We need to handle that before dealing with the rest of the arguments. */
    optCon = poptGetContext("rpm", argc, argv, optionsTable, 0);
    poptReadConfigFile(optCon, LIBRPMALIAS_FILENAME);
    poptReadDefaultConfig(optCon, 1);
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
	return EXIT_FAILURE;

    if (showrc) {
	rpmShowRC(stdout);
	return EXIT_SUCCESS;
    }

    rpmSetVerbosity(RPMMESS_NORMAL);	/* XXX silly use by showrc */

    poptResetContext(optCon);

    if (qva->qva_queryFormat) free((void *)qva->qva_queryFormat);
    memset(qva, 0, sizeof(*qva));
    qva->qva_source = RPMQV_PACKAGE;
    qva->qva_mode = ' ';
    qva->qva_char = ' ';

    while ((arg = poptGetNextOpt(optCon)) > 0) {
	optArg = poptGetOptArg(optCon);

	switch (arg) {
	  case 'K':
	    if (bigMode != MODE_UNKNOWN && bigMode != MODE_CHECKSIG) {
		argerror(_("only one major mode may be specified"));
		return RPM_WRONGOPT;
	    }
	    bigMode = MODE_CHECKSIG;
	    break;
	    
	  case 'u':
	    if (bigMode != MODE_UNKNOWN && bigMode != MODE_ERASE) {
		argerror(_("only one major mode may be specified"));
		return RPM_WRONGOPT;
	    }
	    bigMode = MODE_ERASE;
	    fprintf(stderr, _("-u and --uninstall are deprecated and no"
			" longer work.\n"));
	    fprintf(stderr, _("Use -e or --erase instead.\n"));
	    return EXIT_FAILURE;
	
	  case 'e':
	    if (bigMode != MODE_UNKNOWN && bigMode != MODE_ERASE) {
		argerror(_("only one major mode may be specified"));
		return RPM_WRONGOPT;
	    }
	    bigMode = MODE_ERASE;
	    break;
	
	  case 'v':
	    rpmIncreaseVerbosity();
	    break;

	  case 'i':
	    if (bigMode == MODE_QUERY) {
		const char * infoCommand[] = { "--info", NULL };
		poptStuffArgs(optCon, infoCommand);
	    } else if (bigMode == MODE_INSTALL)
		/*@-ifempty@*/ ;
	    else if (bigMode == MODE_UNKNOWN) {
		const char * installCommand[] = { "--install", NULL };
		poptStuffArgs(optCon, installCommand);
	    }
	    break;

	  case GETOPT_INSTALL:
	    if (bigMode != MODE_UNKNOWN && bigMode != MODE_INSTALL) {
		argerror(_("only one major mode may be specified"));
		return RPM_WRONGOPT;
	    }
	    bigMode = MODE_INSTALL;
	    break;

	  case 'U':
	    if (bigMode != MODE_UNKNOWN && bigMode != MODE_INSTALL) {
		argerror(_("only one major mode may be specified"));
		return RPM_WRONGOPT;
	    }
	    bigMode = MODE_INSTALL;
	    upgrade = 1;
	    break;

	  case 'F':
	    if (bigMode != MODE_UNKNOWN && bigMode != MODE_INSTALL) {
		argerror(_("only one major mode may be specified"));
		return RPM_WRONGOPT;
	    }
	    bigMode = MODE_INSTALL;
	    upgrade = 1;  /* Freshen implies upgrade */
	    freshen = 1;
	    break;

	  case GETOPT_RESIGN:
	    if (bigMode != MODE_UNKNOWN && bigMode != MODE_RESIGN) {
		argerror(_("only one major mode may be specified"));
		return RPM_WRONGOPT;
	    }
	    bigMode = MODE_RESIGN;
	    addSign = RESIGN_NEW_SIGNATURE;
	    signIt = 1;
	    break;

	  case GETOPT_ADDSIGN:
	    if (bigMode != MODE_UNKNOWN && bigMode != MODE_RESIGN) {
		argerror(_("only one major mode may be specified"));
		return RPM_WRONGOPT;
	    }
	    bigMode = MODE_RESIGN;
	    addSign = RESIGN_ADD_SIGNATURE;
	    signIt = 1;
	    break;

	  case GETOPT_DEFINEMACRO:
	    rpmDefineMacro(NULL, optArg, RMIL_CMDLINE);
	    //rpmDefineMacro(&rpmCLIMacroContext, optArg, RMIL_CMDLINE);
	    rpmDefineMacro(rpmCLIMacroContext, optArg, RMIL_CMDLINE);
	    noUsageMsg = 1;
	    break;

	  case GETOPT_EVALMACRO:
	  { const char *val = rpmExpand(optArg, NULL);
	    php_printf("%s\n", val);
	    free((void *)val);
	    noUsageMsg = 1;
	  } break;

	  case GETOPT_REBUILDDB:
	    if (bigMode != MODE_UNKNOWN && bigMode != MODE_REBUILDDB) {
		argerror(_("only one major mode may be specified"));
		return RPM_WRONGOPT;
	    }
	    bigMode = MODE_REBUILDDB;
	    break;

	  case GETOPT_VERIFYDB:
	    if (bigMode != MODE_UNKNOWN && bigMode != MODE_VERIFYDB) {
		argerror(_("only one major mode may be specified"));
		return RPM_WRONGOPT;
	    }
	    bigMode = MODE_VERIFYDB;
	    break;

	  case GETOPT_RELOCATE:
	    if (*optArg != '/')  {
		argerror(_("relocations must begin with a /"));
		return RPM_WRONGOPT;
	    }
	    if (!(errString = strchr(optArg, '='))) {
		argerror(_("relocations must contain a ="));
		return RPM_WRONGOPT;
	    }
	    *errString++ = '\0';
	    if (*errString != '/')  {
		argerror(_("relocations must have a / following the ="));
		return RPM_WRONGOPT;
	    }
	    relocations = xrealloc(relocations, 
				  sizeof(*relocations) * (numRelocations + 1));
	    relocations[numRelocations].oldPath = optArg;
	    relocations[numRelocations++].newPath = errString;
	    break;

	  case GETOPT_EXCLUDEPATH:
	    if (*optArg != '/')  {
		argerror(_("exclude paths must begin with a /"));
		return RPM_WRONGOPT;
	    }

	    relocations = xrealloc(relocations, 
				  sizeof(*relocations) * (numRelocations + 1));
	    relocations[numRelocations].oldPath = optArg;
	    relocations[numRelocations++].newPath = NULL;
	    break;

	  case GETOPT_RCFILE:
	    fprintf(stderr, _("The --rcfile option has been eliminated.\n"));
	    fprintf(stderr, _("Use --macros with a colon separated list of macro files to read.\n"));
	    return EXIT_FAILURE;
	    /*@notreached@*/ break;

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

    if (bigMode == MODE_UNKNOWN && qva->qva_mode != ' ') {
	switch (qva->qva_mode) {
	case 'q':	bigMode = MODE_QUERY;		break;
	case 'V':	bigMode = MODE_VERIFY;		break;
	case 'Q':	bigMode = MODE_QUERYTAGS;	break;
	}
    }

    if (initdb) {
	if (bigMode != MODE_UNKNOWN)  {
	    argerror(_("only one major mode may be specified"));
	    return RPM_WRONGOPT;
	} else
	    bigMode = MODE_INITDB;
    }

    if (qva->qva_sourceCount) {
	if (qva->qva_sourceCount > 1) {
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

    if (!(bigMode == MODE_INSTALL) && force) {
	argerror(_("only installation, upgrading, rmsource and rmspec may be forced"));
	return RPM_WRONGOPT;
    }

    if (bigMode != MODE_INSTALL && badReloc) {
	argerror(_("files may only be relocated during package installation"));
	return RPM_WRONGOPT;
    }

    if (relocations && prefix) {
	argerror(_("only one of --prefix or --relocate may be used"));
	return RPM_WRONGOPT;
    }

    if (bigMode != MODE_INSTALL && relocations) {
	argerror(_("--relocate and --excludepath may only be used when installing new packages"));
	return RPM_WRONGOPT;
    }

    if (bigMode != MODE_INSTALL && prefix) {
	argerror(_("--prefix may only be used when installing new packages"));
	return RPM_WRONGOPT;
    }

    if (prefix && prefix[0] != '/') {
	argerror(_("arguments to --prefix must begin with a /"));
	return RPM_WRONGOPT;
    }

    if (bigMode != MODE_INSTALL && showHash) {
	argerror(_("--hash (-h) may only be specified during package "
			"installation"));
	return RPM_WRONGOPT;
    }

    if (bigMode != MODE_INSTALL && showPercents) {
	argerror(_("--percent may only be specified during package "
			"installation"));
	return RPM_WRONGOPT;
    }

    if (bigMode != MODE_INSTALL && replaceFiles) {
	argerror(_("--replacefiles may only be specified during package "
			"installation"));
	return RPM_WRONGOPT;
    }

    if (bigMode != MODE_INSTALL && replacePackages) {
	argerror(_("--replacepkgs may only be specified during package "
			"installation"));
	return RPM_WRONGOPT;
    }

    if (bigMode != MODE_INSTALL && excldocs) {
	argerror(_("--excludedocs may only be specified during package "
		   "installation"));
	return RPM_WRONGOPT;
    }

    if (bigMode != MODE_INSTALL && incldocs) {
	argerror(_("--includedocs may only be specified during package "
		   "installation"));
	return RPM_WRONGOPT;
    }

    if (excldocs && incldocs) {
	argerror(_("only one of --excludedocs and --includedocs may be "
		 "specified"));
	return RPM_WRONGOPT;
    }
  
    if (bigMode != MODE_INSTALL && ignoreArch) {
	argerror(_("--ignorearch may only be specified during package "
		   "installation"));
	return RPM_WRONGOPT;
    }

    if (bigMode != MODE_INSTALL && ignoreOs) {
	argerror(_("--ignoreos may only be specified during package "
		   "installation"));
	return RPM_WRONGOPT;
    }

    if (bigMode != MODE_INSTALL && ignoreSize) {
	argerror(_("--ignoresize may only be specified during package "
		   "installation"));
	return RPM_WRONGOPT;
    }

    if (allMatches && bigMode != MODE_ERASE) {
	argerror(_("--allmatches may only be specified during package "
		   "erasure"));
	return RPM_WRONGOPT;
    }

    if (allFiles && bigMode != MODE_INSTALL) {
	argerror(_("--allfiles may only be specified during package "
		   "installation"));
	return RPM_WRONGOPT;
    }

    if (justdb && bigMode != MODE_INSTALL && bigMode != MODE_ERASE) {
	argerror(_("--justdb may only be specified during package "
		   "installation and erasure"));
	return RPM_WRONGOPT;
    }

    if (bigMode != MODE_INSTALL && bigMode != MODE_ERASE && 
	bigMode != MODE_VERIFY &&
	(noScripts | noPre | noPost | noPreun | noPostun |
	 noTriggers | noTPrein | noTIn | noTUn | noTPostun)) {
	argerror(_("script disabling options may only be specified during package "
		   "installation, erasure, and verification"));
	return RPM_WRONGOPT;
    }

    if (bigMode != MODE_INSTALL && applyOnly) {
	argerror(_("--apply may only be specified during package "
		   "installation"));
	return RPM_WRONGOPT;
    }

    if (bigMode != MODE_INSTALL && bigMode != MODE_ERASE &&
	(noTriggers | noTPrein | noTIn | noTUn | noTPostun)) {
	argerror(_("trigger disabling options may only be specified during package "
		   "installation and erasure"));
	return RPM_WRONGOPT;
    }

    if (noDeps & (bigMode & ~MODES_FOR_NODEPS)) {
	argerror(_("--nodeps may only be specified during package "
		   "building, rebuilding, recompilation, installation,"
		   "erasure, and verification"));
	return RPM_WRONGOPT;
    }

    if (test && (bigMode & ~MODES_FOR_TEST)) {
	argerror(_("--test may only be specified during package installation, "
		 "erasure, and building"));
	return RPM_WRONGOPT;
    }

    if (rootdir[1] && (bigMode & ~MODES_FOR_ROOT)) {
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

    if (oldPackage && !upgrade) {
	argerror(_("--oldpackage may only be used during upgrades"));
	return RPM_WRONGOPT;
    }

    if (noPgp && bigMode != MODE_CHECKSIG) {
	argerror(_("--nopgp may only be used during signature checking"));
	return RPM_WRONGOPT;
    }

    if (noGpg && bigMode != MODE_CHECKSIG) {
	argerror(_("--nogpg may only be used during signature checking"));
	return RPM_WRONGOPT;
    }

    if (noMd5 && bigMode != MODE_CHECKSIG && bigMode != MODE_VERIFY) {
	argerror(_("--nomd5 may only be used during signature checking and "
		   "package verification"));
	return RPM_WRONGOPT;
    }

    if (signIt) {
        if (bigMode == MODE_REBUILD || bigMode == MODE_BUILD ||
	    bigMode == MODE_RESIGN || bigMode == MODE_TARBUILD) {
	    const char ** argv;
	    struct stat sb;
	    int errors = 0;

	    if ((argv = poptGetArgs(optCon)) == NULL) {
		fprintf(stderr, _("no files to sign\n"));
		errors++;
	    } else
	    while (*argv) {
		if (stat(*argv, &sb)) {
		    fprintf(stderr, _("cannot access file %s\n"), *argv);
		    errors++;
		}
		argv++;
	    }

	    if (errors) return errors;

            if (poptPeekArg(optCon)) {
		switch (sigTag = rpmLookupSignatureType(RPMLOOKUPSIG_QUERY)) {
		  case 0:
		    break;
		  case RPMSIGTAG_PGP:
		    if ((sigTag == RPMSIGTAG_PGP || sigTag == RPMSIGTAG_PGP5) &&
		        !rpmDetectPGPVersion(NULL)) {
			fprintf(stderr, _("pgp not found: "));
			return EXIT_FAILURE;
		    }	/*@fallthrough@*/
		  case RPMSIGTAG_GPG:
		    passPhrase = rpmGetPassPhrase(_("Enter pass phrase: "), sigTag);
		    if (passPhrase == NULL) {
			fprintf(stderr, _("Pass phrase check failed\n"));
			return EXIT_FAILURE;
		    }
		    fprintf(stderr, _("Pass phrase is good.\n"));
		    passPhrase = xstrdup(passPhrase);
		    break;
		  default:
		    fprintf(stderr,
		            _("Invalid %%_signature spec in macro file.\n"));
		    return EXIT_FAILURE;
		    /*@notreached@*/ break;
		}
	    }
	} else {
	    argerror(_("--sign may only be used during package building"));
	    return RPM_WRONGOPT;
	}
    } else {
    	/* Make rpmLookupSignatureType() return 0 ("none") from now on */
        rpmLookupSignatureType(RPMLOOKUPSIG_DISABLE);
    }

    if (pipeOutput) {
	pipe(p);

	if (!(pipeChild = fork())) {
	    close(p[1]);
	    dup2(p[0], STDIN_FILENO);
	    close(p[0]);
	    execl("/bin/sh", "/bin/sh", "-c", pipeOutput, NULL);
	    fprintf(stderr, _("exec failed\n"));
	}

	close(p[0]);
	dup2(p[1], STDOUT_FILENO);
	close(p[1]);
    }
	
    switch (bigMode) {
      case MODE_UNKNOWN:
	break;

      case MODE_REBUILDDB:
	ec = rpmdbRebuild(rootdir);
	break;

      case MODE_VERIFYDB:
	ec = rpmdbVerify(rootdir);
	break;

      case MODE_QUERYTAGS:
	if (argc != 2) {
	    argerror(_("unexpected arguments to --querytags "));
	    return RPM_WRONGOPT;
	}

	rpmDisplayQueryTags(stdout);
	break;

      case MODE_INITDB:
	rpmdbInit(rootdir, 0644);
	break;

      case MODE_CHECKSIG:
	if (!poptPeekArg(optCon)) {
	    argerror(_("no packages given for signature check"));
	    return RPM_NOPKG;
	}
	if (!noPgp) checksigFlags |= CHECKSIG_PGP;
	if (!noGpg) checksigFlags |= CHECKSIG_GPG;
	if (!noMd5) checksigFlags |= CHECKSIG_MD5;
	ec = rpmCheckSig(checksigFlags, (const char **)poptGetArgs(optCon));
	/* XXX don't overflow single byte exit status */
	if (ec > 255) ec = 255;
	break;

      case MODE_RESIGN:
	if (!poptPeekArg(optCon)) {
	    argerror(_("no packages given for signing"));
	    return RPM_NOPKG;
	}
	ec = rpmReSign(addSign, passPhrase, (const char **)poptGetArgs(optCon));
	/* XXX don't overflow single byte exit status */
	if (ec > 255) ec = 255;
	break;
	
      case MODE_REBUILD:
      case MODE_RECOMPILE:
	break;

      case MODE_BUILD:
      case MODE_TARBUILD:
	break;

      case MODE_ERASE:
	if (!poptPeekArg(optCon)) {
	    argerror(_("no packages given for uninstall"));
	    return RPM_NOPKG;
	}

	if (noScripts) transFlags |= (_noTransScripts | _noTransTriggers);
	if (noPre) transFlags |= RPMTRANS_FLAG_NOPRE;
	if (noPost) transFlags |= RPMTRANS_FLAG_NOPOST;
	if (noPreun) transFlags |= RPMTRANS_FLAG_NOPREUN;
	if (noPostun) transFlags |= RPMTRANS_FLAG_NOPOSTUN;

	if (noTriggers) transFlags |= _noTransTriggers;
	if (noTPrein) transFlags |= RPMTRANS_FLAG_NOTRIGGERPREIN;
	if (noTIn) transFlags |= RPMTRANS_FLAG_NOTRIGGERIN;
	if (noTUn) transFlags |= RPMTRANS_FLAG_NOTRIGGERUN;
	if (noTPostun) transFlags |= RPMTRANS_FLAG_NOTRIGGERPOSTUN;

	if (test) transFlags |= RPMTRANS_FLAG_TEST;
	if (justdb) transFlags |= RPMTRANS_FLAG_JUSTDB;
	if (dirStash) transFlags |= RPMTRANS_FLAG_DIRSTASH;
	if (rePackage) transFlags |= RPMTRANS_FLAG_REPACKAGE;
	if (pkgCommit) transFlags |= RPMTRANS_FLAG_PKGCOMMIT;
	if (pkgUndo) transFlags |= RPMTRANS_FLAG_PKGUNDO;
	if (tsCommit) transFlags |= RPMTRANS_FLAG_COMMIT;
	if (tsUndo) transFlags |= RPMTRANS_FLAG_UNDO;

	if (noDeps) eraseInterfaceFlags |= UNINSTALL_NODEPS;
	if (allMatches) eraseInterfaceFlags |= UNINSTALL_ALLMATCHES;

	ec = rpmErase(rootdir, (const char **)poptGetArgs(optCon), 
			 transFlags, eraseInterfaceFlags);
	break;

      case MODE_INSTALL:
	if (force) {
	    probFilter |= RPMPROB_FILTER_REPLACEPKG | 
			  RPMPROB_FILTER_REPLACEOLDFILES |
			  RPMPROB_FILTER_REPLACENEWFILES |
			  RPMPROB_FILTER_OLDPACKAGE;
	}
	if (replaceFiles) probFilter |= RPMPROB_FILTER_REPLACEOLDFILES |
			                RPMPROB_FILTER_REPLACENEWFILES;
	if (badReloc) probFilter |= RPMPROB_FILTER_FORCERELOCATE;
	if (replacePackages) probFilter |= RPMPROB_FILTER_REPLACEPKG;
	if (oldPackage) probFilter |= RPMPROB_FILTER_OLDPACKAGE;
	if (ignoreArch) probFilter |= RPMPROB_FILTER_IGNOREARCH; 
	if (ignoreOs) probFilter |= RPMPROB_FILTER_IGNOREOS;
	if (ignoreSize) probFilter |= RPMPROB_FILTER_DISKSPACE;

	if (applyOnly)
	    transFlags = (_noTransScripts | _noTransTriggers |
		RPMTRANS_FLAG_APPLYONLY | RPMTRANS_FLAG_PKGCOMMIT);

	if (test) transFlags |= RPMTRANS_FLAG_TEST;
	/* RPMTRANS_FLAG_BUILD_PROBS */

	if (noScripts) transFlags |= (_noTransScripts | _noTransTriggers);
	if (noPre) transFlags |= RPMTRANS_FLAG_NOPRE;
	if (noPost) transFlags |= RPMTRANS_FLAG_NOPOST;
	if (noPreun) transFlags |= RPMTRANS_FLAG_NOPREUN;
	if (noPostun) transFlags |= RPMTRANS_FLAG_NOPOSTUN;

	if (noTriggers) transFlags |= RPMTRANS_FLAG_NOTRIGGERS;
	if (noTPrein) transFlags |= RPMTRANS_FLAG_NOTRIGGERPREIN;
	if (noTIn) transFlags |= RPMTRANS_FLAG_NOTRIGGERIN;
	if (noTUn) transFlags |= RPMTRANS_FLAG_NOTRIGGERUN;
	if (noTPostun) transFlags |= RPMTRANS_FLAG_NOTRIGGERPOSTUN;

	if (justdb) transFlags |= RPMTRANS_FLAG_JUSTDB;
	if (!incldocs) {
	    if (excldocs)
		transFlags |= RPMTRANS_FLAG_NODOCS;
	    else if (rpmExpandNumeric("%{_excludedocs}"))
		transFlags |= RPMTRANS_FLAG_NODOCS;
	}
	if (allFiles) transFlags |= RPMTRANS_FLAG_ALLFILES;
	if (dirStash) transFlags |= RPMTRANS_FLAG_DIRSTASH;
	if (rePackage) transFlags |= RPMTRANS_FLAG_REPACKAGE;
	if (pkgCommit) transFlags |= RPMTRANS_FLAG_PKGCOMMIT;
	if (pkgUndo) transFlags |= RPMTRANS_FLAG_PKGUNDO;
	if (tsCommit) transFlags |= RPMTRANS_FLAG_COMMIT;
	if (tsUndo) transFlags |= RPMTRANS_FLAG_UNDO;
	/* RPMTRANS_FLAG_KEEPOBSOLETE */

	if (showPercents) installInterfaceFlags |= INSTALL_PERCENT;
	if (showHash) installInterfaceFlags |= INSTALL_HASH;
	if (noDeps) installInterfaceFlags |= INSTALL_NODEPS;
	if (noOrder) installInterfaceFlags |= INSTALL_NOORDER;
	if (upgrade) installInterfaceFlags |= INSTALL_UPGRADE;
	if (freshen) installInterfaceFlags |= (INSTALL_UPGRADE|INSTALL_FRESHEN);

	if (!poptPeekArg(optCon)) {
	    argerror(_("no packages given for install"));
	    return RPM_NOPKG;
	}

	/* we've already ensured !(!prefix && !relocations) */
	if (prefix) {
	    relocations = alloca(2 * sizeof(*relocations));
	    relocations[0].oldPath = NULL;   /* special case magic */
	    relocations[0].newPath = prefix;
	    relocations[1].oldPath = relocations[1].newPath = NULL;
	} else if (relocations) {
	    relocations = xrealloc(relocations, 
				  sizeof(*relocations) * (numRelocations + 1));
	    relocations[numRelocations].oldPath = NULL;
	    relocations[numRelocations].newPath = NULL;
	}

	ec += rpmInstall(rootdir, (const char **)poptGetArgs(optCon), 
			transFlags, installInterfaceFlags, probFilter,
			relocations);
	break;

      case MODE_QUERY:
	qva->qva_prefix = rootdir;
	if (qva->qva_source == RPMQV_ALL) {
	    if (poptPeekArg(optCon)) {
		argerror(_("extra arguments given for query of all packages"));
		return RPM_WRONGOPT;
	    }

	    ec = rpmQuery_p(qva, RPMQV_ALL, NULL);
	} else {
	    if (!poptPeekArg(optCon)) {
		argerror(_("no arguments given for query"));
		return RPM_WRONGOPT;
	    }
	    while ((pkg = poptGetArg(optCon)))
		ec += rpmQuery_p(qva, qva->qva_source, pkg);
	}
	break;

      case MODE_VERIFY:
	verifyFlags = VERIFY_ALL;
	verifyFlags &= ~qva->qva_flags;
	if (noDeps)	verifyFlags &= ~VERIFY_DEPS;
	if (noScripts)	verifyFlags &= ~VERIFY_SCRIPT;
	if (noMd5)	verifyFlags &= ~VERIFY_MD5;

	qva->qva_prefix = rootdir;
	qva->qva_flags = verifyFlags;
	if (qva->qva_source == RPMQV_ALL) {
	    if (poptPeekArg(optCon)) {
		argerror(_("extra arguments given for verify of all packages"));
		return RPM_WRONGOPT;
	    }
	    ec = rpmVerify(qva, RPMQV_ALL, NULL);
	} else {
	    if (!poptPeekArg(optCon)) {
		argerror(_("no arguments given for verify"));
		return RPM_WRONGOPT;
	    }
	    while ((pkg = poptGetArg(optCon)))
		ec += rpmVerify(qva, qva->qva_source, pkg);
	}
	break;
    }

    optCon = poptFreeContext(optCon);
    rpmFreeMacros(NULL);
    //rpmFreeMacros(&rpmCLIMacroContext);
    rpmFreeMacros(rpmCLIMacroContext);
    rpmFreeRpmrc();

    if (pipeChild) {
	fclose(stdout);
	(void)waitpid(pipeChild, &status, 0);
    }

    /* keeps memory leak checkers quiet */
    freeNames();
    freeFilesystems();
    urlFreeCache();
    if (qva->qva_queryFormat) free((void *)qva->qva_queryFormat);

#if HAVE_MCHECK_H && HAVE_MTRACE
    muntrace();   /* Trace malloc only if MALLOC_TRACE=mtrace-output-file. */
#endif
    return ec;
}
