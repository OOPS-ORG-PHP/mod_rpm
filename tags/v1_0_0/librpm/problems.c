/**
 * \file lib/problems.c
 */

#include "system.h"

#include <rpmlib.h>

#include "depends.h"
#include "misc.h"
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

#define LASTMSGADD -100

/*@access rpmProblemSet@*/
/*@access rpmProblem@*/
/*@access rpmDependencyConflict@*/

/* XXX FIXME: merge into problems */
/* XXX used in verify.c rpmlibprov.c */
void printDepFlags_p(FILE * fp, const char * version, int flags)
{
    if (flags)
	rpm_last_message (LASTMSGADD, " ");

    if (flags & RPMSENSE_LESS) 
	rpm_last_message (LASTMSGADD, "<");
    if (flags & RPMSENSE_GREATER)
	rpm_last_message (LASTMSGADD, ">");
    if (flags & RPMSENSE_EQUAL)
	rpm_last_message (LASTMSGADD, "=");

    if (flags)
	rpm_last_message (LASTMSGADD, " %s", version);
}

static int sameProblem(const rpmDependencyConflict ap,
		const rpmDependencyConflict bp)
	/*@*/
{

    if (ap->sense != bp->sense)
	return 1;

    if (ap->byName && bp->byName && strcmp(ap->byName, bp->byName))
	return 1;
    if (ap->byVersion && bp->byVersion && strcmp(ap->byVersion, bp->byVersion))
	return 1;
    if (ap->byRelease && bp->byRelease && strcmp(ap->byRelease, bp->byRelease))
	return 1;

    if (ap->needsName && bp->needsName && strcmp(ap->needsName, bp->needsName))
	return 1;
    if (ap->needsVersion && bp->needsVersion && strcmp(ap->needsVersion, bp->needsVersion))
	return 1;
    if (ap->needsFlags && bp->needsFlags && ap->needsFlags != bp->needsFlags)
	return 1;

    return 0;
}

/* XXX FIXME: merge into problems */
void printDepProblems_p(FILE * fp,
		const rpmDependencyConflict conflicts, int numConflicts)
{
    int i;

    for (i = 0; i < numConflicts; i++) {
	int j;

	/* Filter already displayed problems. */
	for (j = 0; j < i; j++) {
	    if (!sameProblem(conflicts + i, conflicts + j))
		/*@innerbreak@*/ break;
	}
	if (j < i)
	    continue;

	rpm_last_message (LASTMSGADD, "\t%s", conflicts[i].needsName);
	if (conflicts[i].needsFlags)
	    printDepFlags_p(fp, conflicts[i].needsVersion, 
			  conflicts[i].needsFlags);

	if (conflicts[i].sense == RPMDEP_SENSE_REQUIRES) 
	    rpm_last_message (LASTMSGADD, _(" is needed by %s-%s-%s\n"), conflicts[i].byName, 
		    conflicts[i].byVersion, conflicts[i].byRelease);
	else
	    rpm_last_message (LASTMSGADD, _(" conflicts with %s-%s-%s\n"), conflicts[i].byName, 
		    conflicts[i].byVersion, conflicts[i].byRelease);
    }
}

void rpmProblemPrint_p(FILE *fp, rpmProblem prob)
{
    const char * msg = rpmProblemString(prob);
    rpm_last_message (LASTMSGADD, "%s\n", msg);
    msg = _free(msg);
}

void rpmProblemSetPrint_p(FILE *fp, rpmProblemSet probs)
{
    int i;

    if (probs == NULL)
	return;

    if (fp == NULL)
	fp = stderr;

    for (i = 0; i < probs->numProblems; i++) {
	rpmProblem myprob = probs->probs + i;
	if (!myprob->ignoreProblem)
	    rpmProblemPrint_p(fp, myprob);
    }
}
