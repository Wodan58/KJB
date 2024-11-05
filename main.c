/*
 *  module  : main.c
 *  version : 1.2
 *  date    : 10/18/24
 */
#include "globals.h"

extern table_t yytable[];	/* present in generated code */

static jmp_buf begin;		/* restart with empty program */

char *bottom_of_stack;		/* needed in gc.c */

int start_of_joy(pEnv env);	/* application specific input */
void joy(pEnv env);		/* result of compilation */
void end_of_joy(pEnv env);	/* application specific output */

/*
 * abort execution and restart reading from srcfile; the stack is not cleared.
 */
void abortexecution_(int num)
{
    fflush(stdin);
    longjmp(begin, num);
}

/*
 * fatal terminates the application with an error message.
 */
#if defined(NOBDW) && defined(_MSC_VER)
void fatal(char *str)
{
    fflush(stdout);
    fprintf(stderr, "fatal error: %s\n", str);
    abortexecution_(ABORT_QUIT);
}
#endif

/*
 * main uses some of the code that the interpreter also uses.
 */
static void my_main(int argc, char **argv)
{
    Env env;	/* global variables */
    int i, j;

    /*
     * Start the clock.
     */
    env.startclock = clock();
    /*
     * Initialize variables.
     */
    memset(&env, 0, sizeof(env));
    /*
     * Initial settings.
     */
    env.autoput = INIAUTOPUT;
    env.echoflag = INIECHOFLAG;
    env.undeferror = INIUNDEFERROR;
    env.tracegc = INITRACEGC;
    env.overwrite = INIWARNING;
    /*
     * First look for options. They start with -.
     */
    for (i = 1; i < argc; i++) {
	if (argv[i][0] == '-') {
	    for (j = 1; argv[i][j]; j++) {
		switch (argv[i][j]) {
		case 'd' : env.debugging = 1; break;
		case 't' : env.debugging = 2; break;
		} /* end switch */
	    } /* end for */
	    /*
	     * Overwrite the option(s) with subsequent parameters. Index i is
	     * decreased, because the next parameter is copied to the current
	     * index and i is increased in the for-loop.
	     */
	    for (--argc, j = i--; j < argc; j++)
		argv[j] = argv[j + 1];
	} /* end if */
    } /* end for */
    /*
     * determine argc and argv.
     */
    env.g_argc = argc;
    env.g_argv = argv;
    /*
     * application specific code.
     */
    if (start_of_joy(&env))	/* check parameters */
	return;
    if (!setjmp(begin))	{	/* return here after error or abort */
	joy(&env);		/* evaluate */
	/*
	 * application specific code.
	 */
	end_of_joy(&env);
    }
}

/*
 * Allow the option to link with gc.c
 */
int main(int argc, char **argv)
{
    void (*volatile m)(int, char **) = my_main;

    bottom_of_stack = (char *)&argc;
    GC_INIT();
    (*m)(argc, argv);
    return 0;
}

/*
 * nickname - return the name of an operator.
 */
char *nickname(int ch)
{
    return yytable[ch].name;
}

/*
 * operindex - return the optable entry for an operator.
 */
int operindex(pEnv env, proc_t proc)
{
    int i;

    for (i = 0; yytable[i].proc; i++)
	if (yytable[i].proc == proc)
	    return i;
    return 0;
}
