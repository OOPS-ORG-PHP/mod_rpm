/* config.h.  Generated automatically by configure.  */
/* config.h.in.  Generated automatically from configure.in by autoheader.  */

/* Define if on AIX 3.
   System headers sometimes define this.
   We just want to avoid a redefinition error message.  */
#ifndef _ALL_SOURCE
/* #undef _ALL_SOURCE */
#endif

/* Define if using alloca.c.  */
/* #undef C_ALLOCA */

/* Define to empty if the keyword does not work.  */
/* #undef const */

/* Define to one of _getb67, GETB67, getb67 for Cray-2 and Cray-YMP systems.
   This function is required for alloca.c support on those systems.  */
/* #undef CRAY_STACKSEG_END */

/* Define if you have alloca, as a function or macro.  */
#define HAVE_ALLOCA 1

/* Define if you have <alloca.h> and it should be used (not on Ultrix).  */
#define HAVE_ALLOCA_H 1

/* Define if you don't have vprintf but do have _doprnt.  */
/* #undef HAVE_DOPRNT */

/* Define if your system has a working fnmatch function.  */
#define HAVE_FNMATCH 1

/* Define if you have the getmntent function.  */
#define HAVE_GETMNTENT 1

/* Define if you have a working `mmap' system call.  */
#define HAVE_MMAP 1

/* Define if you have the vprintf function.  */
#define HAVE_VPRINTF 1

/* Define as __inline if that's what the C compiler calls it.  */
/* #undef inline */

/* Define if major, minor, and makedev are declared in <mkdev.h>.  */
/* #undef MAJOR_IN_MKDEV */

/* Define if major, minor, and makedev are declared in <sysmacros.h>.  */
/* #undef MAJOR_IN_SYSMACROS */

/* Define if on MINIX.  */
/* #undef _MINIX */

/* Define to `int' if <sys/types.h> doesn't define.  */
/* #undef mode_t */

/* Define to `long' if <sys/types.h> doesn't define.  */
/* #undef off_t */

/* Define to `int' if <sys/types.h> doesn't define.  */
/* #undef pid_t */

/* Define if the system does not provide POSIX.1 features except
   with this defined.  */
/* #undef _POSIX_1_SOURCE */

/* Define if you need to in order for stat and other things to work.  */
/* #undef _POSIX_SOURCE */

/* Define to `unsigned' if <sys/types.h> doesn't define.  */
/* #undef size_t */

/* If using the C implementation of alloca, define if you know the
   direction of stack growth for your system; otherwise it will be
   automatically deduced at run-time.
 STACK_DIRECTION > 0 => grows toward higher addresses
 STACK_DIRECTION < 0 => grows toward lower addresses
 STACK_DIRECTION = 0 => direction of growth unknown
 */
/* #undef STACK_DIRECTION */

/* Define if you have the ANSI C header files.  */
#define STDC_HEADERS 1

/* Define if you can safely include both <sys/time.h> and <time.h>.  */
#define TIME_WITH_SYS_TIME 1

/* Define as 1 if you have mntctl() (only aix?) */
/* #undef HAVE_MNTCTL */

/* Define as 1 if <netdb.h> defines h_errno */
#define HAVE_HERRNO 1

/* Define as 1 if <sys/stat.h> defines S_ISLNK */
#define HAVE_S_ISLNK 1

/* Define as 1 if <sys/stat.h> defines S_IFSOCK */
#define HAVE_S_IFSOCK 1

/* Define as 1 if <sys/stat.h> defines S_ISSOCK */
#define HAVE_S_ISSOCK 1

/* Define as 1 if we need myrealloc */
/* #undef NEED_MYREALLOC */

/* Define as one if we need to include <strings.h> (along with <string.h>) */
/* #undef NEED_STRINGS_H */

/* Define as 1 if you have getmntinfo_r() (only osf?) */
/* #undef HAVE_GETMNTINFO_R */

/* Define as 1 if you have "struct mnttab" (only sco?) */
/* #undef HAVE_STRUCT_MNTTAB */

/* Define as 1 if you have lchown() */
#define HAVE_LCHOWN 1

/* Define as 1 if chown() follows symlinks and you don't have lchown() */
/* #undef CHOWN_FOLLOWS_SYMLINK */

/* Define if the patch call you'll be using is 2.1 or older */
/* #undef HAVE_OLDPATCH_21 */

/* Define as 1 if your zlib has gzseek() */
#define HAVE_GZSEEK 1

/* Define as 1 if your va_list type is an array */
/* #undef HAVE_VA_LIST_AS_ARRAY */

/* Define as 1 if you bzip2 1.0 */
#define HAVE_BZ2_1_0 1

/* A full path to a program, possibly with arguments, that will create a
   directory and all necessary parent directories, ala `mkdir -p'        */
#define MKDIR_P "/bin/mkdir -p"

/* Full path to rpm locale directory (usually /usr/share/locale) */
#define LOCALEDIR "/usr/share/locale"

/* Full path to rpm global configuration directory (usually /usr/lib/rpm) */
#define RPMCONFIGDIR "/usr/lib/rpm"

/* Full path to rpm system configuration directory (usually /etc/rpm) */
#define SYSCONFIGDIR "/etc/rpm"

/* Full path to find-provides script (usually /usr/lib/rpm/find-provides) */
#define FINDPROVIDES "/usr/lib/rpm/find-provides"

/* Full path to find-requires script (usually /usr/lib/rpm/find-requires) */
#define FINDREQUIRES "/usr/lib/rpm/find-requires"

/* Full path to rpmpopt configuration file (usually /usr/lib/rpm/rpmpopt) */
#define LIBRPMALIAS_FILENAME "/usr/lib/rpm/rpmpopt-4.0.4"

/* Full path to rpmrc configuration file (usually /usr/lib/rpm/rpmrc) */
#define LIBRPMRC_FILENAME "/usr/lib/rpm/rpmrc"

/* Colon separated paths of macro files to read. */
#define MACROFILES "/usr/lib/rpm/macros:/usr/lib/rpm/%{_target_platform}/macros:/etc/rpm/macros:/etc/rpm/macros.specspo:/etc/rpm/macros.db1:/etc/rpm/%{_target_platform}/macros:~/.rpmmacros"

/* statfs in <sys/statvfs.h> (for solaris 2.6+ systems) */
#define STATFS_IN_SYS_STATVFS 1

/* statfs in <sys/vfs.h> (for linux systems) */
/* #undef STATFS_IN_SYS_VFS */

/* statfs in <sys/mount.h> (for Digital Unix 4.0D systems) */
/* #undef STATFS_IN_SYS_MOUNT */

/* statfs in <sys/statfs.h> (for Irix 6.4 systems) */
/* #undef STATFS_IN_SYS_STATFS */

/* define if struct statfs has the f_bavail member */
#define STATFS_HAS_F_BAVAIL 1

/* define if the statfs() call takes 4 arguments */
/* #undef STAT_STATFS4 */

/* Use the included glob.c? */
/* #undef USE_GNU_GLOB */

/* Use the Berkeley db3 API? */
#define USE_DB3 1

/* Use the Berkeley db1 retrofit to db2/db3 API? */
#define USE_DB1 1

/* Define if you have the <db3/db.h> header file.  */
#define HAVE_DB3_DB_H 1

/* Build with dmalloc support? */
/* #undef DMALLOC */

 
/* Define if you have the getmntent() function */
#define HAVE_GETMNTENT 1

/* Define if you have the __argz_count function.  */
#define HAVE___ARGZ_COUNT 1

/* Define if you have the __argz_next function.  */
#define HAVE___ARGZ_NEXT 1

/* Define if you have the __argz_stringify function.  */
#define HAVE___ARGZ_STRINGIFY 1

/* Define if you have the basename function.  */
#define HAVE_BASENAME 1

/* Define if you have the dcgettext function.  */
#define HAVE_DCGETTEXT 1

/* Define if you have the feof_unlocked function.  */
#define HAVE_FEOF_UNLOCKED 1

/* Define if you have the fgets_unlocked function.  */
#define HAVE_FGETS_UNLOCKED 1

/* Define if you have the ftok function.  */
#define HAVE_FTOK 1

/* Define if you have the getcwd function.  */
#define HAVE_GETCWD 1

/* Define if you have the getegid function.  */
#define HAVE_GETEGID 1

/* Define if you have the geteuid function.  */
#define HAVE_GETEUID 1

/* Define if you have the getgid function.  */
#define HAVE_GETGID 1

/* Define if you have the getpagesize function.  */
#define HAVE_GETPAGESIZE 1

/* Define if you have the getpassphrase function.  */
/* #undef HAVE_GETPASSPHRASE */

/* Define if you have the getuid function.  */
#define HAVE_GETUID 1

/* Define if you have the getwd function.  */
#define HAVE_GETWD 1

/* Define if you have the glob function.  */
#define HAVE_GLOB 1

/* Define if you have the inet_aton function.  */
#define HAVE_INET_ATON 1

/* Define if you have the mempcpy function.  */
#define HAVE_MEMPCPY 1

/* Define if you have the mtrace function.  */
#define HAVE_MTRACE 1

/* Define if you have the munmap function.  */
#define HAVE_MUNMAP 1

/* Define if you have the personality function.  */
#define HAVE_PERSONALITY 1

/* Define if you have the putenv function.  */
#define HAVE_PUTENV 1

/* Define if you have the realpath function.  */
#define HAVE_REALPATH 1

/* Define if you have the regcomp function.  */
#define HAVE_REGCOMP 1

/* Define if you have the setenv function.  */
#define HAVE_SETENV 1

/* Define if you have the setlocale function.  */
#define HAVE_SETLOCALE 1

/* Define if you have the snprintf function.  */
#define HAVE_SNPRINTF 1

/* Define if you have the stpcpy function.  */
#define HAVE_STPCPY 1

/* Define if you have the stpncpy function.  */
#define HAVE_STPNCPY 1

/* Define if you have the strcasecmp function.  */
#define HAVE_STRCASECMP 1

/* Define if you have the strchr function.  */
#define HAVE_STRCHR 1

/* Define if you have the strcspn function.  */
#define HAVE_STRCSPN 1

/* Define if you have the strdup function.  */
#define HAVE_STRDUP 1

/* Define if you have the strerror function.  */
#define HAVE_STRERROR 1

/* Define if you have the strspn function.  */
#define HAVE_STRSPN 1

/* Define if you have the strstr function.  */
#define HAVE_STRSTR 1

/* Define if you have the strtol function.  */
#define HAVE_STRTOL 1

/* Define if you have the strtoul function.  */
#define HAVE_STRTOUL 1

/* Define if you have the tsearch function.  */
#define HAVE_TSEARCH 1

/* Define if you have the vsnprintf function.  */
#define HAVE_VSNPRINTF 1

/* Define if you have the <argz.h> header file.  */
#define HAVE_ARGZ_H 1

/* Define if you have the <bzlib.h> header file.  */
#define HAVE_BZLIB_H 1

/* Define if you have the <db3/db.h> header file.  */
#define HAVE_DB3_DB_H 1

/* Define if you have the <dirent.h> header file.  */
#define HAVE_DIRENT_H 1

/* Define if you have the <dlfcn.h> header file.  */
#define HAVE_DLFCN_H 1

/* Define if you have the <err.h> header file.  */
#define HAVE_ERR_H 1

/* Define if you have the <fcntl.h> header file.  */
#define HAVE_FCNTL_H 1

/* Define if you have the <float.h> header file.  */
#define HAVE_FLOAT_H 1

/* Define if you have the <getopt.h> header file.  */
#define HAVE_GETOPT_H 1

/* Define if you have the <glob.h> header file.  */
#define HAVE_GLOB_H 1

/* Define if you have the <grp.h> header file.  */
#define HAVE_GRP_H 1

/* Define if you have the <libio.h> header file.  */
#define HAVE_LIBIO_H 1

/* Define if you have the <limits.h> header file.  */
#define HAVE_LIMITS_H 1

/* Define if you have the <locale.h> header file.  */
#define HAVE_LOCALE_H 1

/* Define if you have the <machine/types.h> header file.  */
/* #undef HAVE_MACHINE_TYPES_H */

/* Define if you have the <malloc.h> header file.  */
#define HAVE_MALLOC_H 1

/* Define if you have the <mcheck.h> header file.  */
#define HAVE_MCHECK_H 1

/* Define if you have the <memory.h> header file.  */
#define HAVE_MEMORY_H 1

/* Define if you have the <mntent.h> header file.  */
#define HAVE_MNTENT_H 1

/* Define if you have the <ndir.h> header file.  */
/* #undef HAVE_NDIR_H */

/* Define if you have the <netdb.h> header file.  */
#define HAVE_NETDB_H 1

/* Define if you have the <netinet/in_systm.h> header file.  */
#define HAVE_NETINET_IN_SYSTM_H 1

/* Define if you have the <nl_types.h> header file.  */
#define HAVE_NL_TYPES_H 1

/* Define if you have the <pwd.h> header file.  */
#define HAVE_PWD_H 1

/* Define if you have the <stddef.h> header file.  */
#define HAVE_STDDEF_H 1

/* Define if you have the <stdlib.h> header file.  */
#define HAVE_STDLIB_H 1

/* Define if you have the <string.h> header file.  */
#define HAVE_STRING_H 1

/* Define if you have the <sys/dir.h> header file.  */
/* #undef HAVE_SYS_DIR_H */

/* Define if you have the <sys/ipc.h> header file.  */
#define HAVE_SYS_IPC_H 1

/* Define if you have the <sys/mman.h> header file.  */
#define HAVE_SYS_MMAN_H 1

/* Define if you have the <sys/mntctl.h> header file.  */
/* #undef HAVE_SYS_MNTCTL_H */

/* Define if you have the <sys/mnttab.h> header file.  */
/* #undef HAVE_SYS_MNTTAB_H */

/* Define if you have the <sys/mount.h> header file.  */
#define HAVE_SYS_MOUNT_H 1

/* Define if you have the <sys/ndir.h> header file.  */
/* #undef HAVE_SYS_NDIR_H */

/* Define if you have the <sys/param.h> header file.  */
#define HAVE_SYS_PARAM_H 1

/* Define if you have the <sys/resource.h> header file.  */
#define HAVE_SYS_RESOURCE_H 1

/* Define if you have the <sys/select.h> header file.  */
#define HAVE_SYS_SELECT_H 1

/* Define if you have the <sys/socket.h> header file.  */
#define HAVE_SYS_SOCKET_H 1

/* Define if you have the <sys/stat.h> header file.  */
#define HAVE_SYS_STAT_H 1

/* Define if you have the <sys/stdtypes.h> header file.  */
/* #undef HAVE_SYS_STDTYPES_H */

/* Define if you have the <sys/systemcfg.h> header file.  */
/* #undef HAVE_SYS_SYSTEMCFG_H */

/* Define if you have the <sys/types.h> header file.  */
#define HAVE_SYS_TYPES_H 1

/* Define if you have the <sys/utsname.h> header file.  */
#define HAVE_SYS_UTSNAME_H 1

/* Define if you have the <sys/vmount.h> header file.  */
/* #undef HAVE_SYS_VMOUNT_H */

/* Define if you have the <sys/wait.h> header file.  */
#define HAVE_SYS_WAIT_H 1

/* Define if you have the <unistd.h> header file.  */
#define HAVE_UNISTD_H 1

/* Define if you have the <utime.h> header file.  */
#define HAVE_UTIME_H 1

/* Define if you have the <zlib.h> header file.  */
#define HAVE_ZLIB_H 1

/* Define if you have the port library (-lport).  */
/* #undef HAVE_LIBPORT */

/* Name of package */
#define PACKAGE "rpm"

/* Version number of package */
#define VERSION "4.0.4"

/* Define if compiler has function prototypes */
#define PROTOTYPES 1

/* Define if you have the iconv() function. */
#define HAVE_ICONV 1

/* Define as const if the declaration of iconv() needs const. */
#define ICONV_CONST 

/* Define if you have <langinfo.h> and nl_langinfo(CODESET). */
#define HAVE_LANGINFO_CODESET 1

/* Define if your <locale.h> file defines LC_MESSAGES. */
#define HAVE_LC_MESSAGES 1

/* Define to 1 if translation of program messages to the user's native language
   is requested. */
#define ENABLE_NLS 1

/* Define if the GNU gettext() function is already present or preinstalled. */
#define HAVE_GETTEXT 1

/* Defined if getmntent replacement is used */
/* #undef USE_GETMNTENT */

