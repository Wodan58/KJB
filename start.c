/*
 *  module  : start.c
 *  version : 1.5
 *  date    : 10/18/24
 */
#include "globals.h"

/*
 * The longest verse is book 17, chapter 8, verse 9.
 * It takes 7 lines to print and uses 109 bytes in the compressed file.
 */
#define MAX_LINE	78

/*
 * File schidx.c
 */
int search_index(unsigned char *str, int *offset);

/*
 * Parameters: book, chapter, verse.
 */
int start_of_joy(pEnv env)
{
    int leng, offset;
    unsigned char str[3];

    /*
     * Check number of parameters.
     */
    if (env->g_argc != 4) {
	fprintf(stderr, "Use: %s book chapter verse\n", env->g_argv[0]);
	return 1;
    }
    /*
     * Assume 3 parameters, use them to search the index.
     */
    str[0] = atoi(env->g_argv[1]);
    str[1] = atoi(env->g_argv[2]);
    str[2] = atoi(env->g_argv[3]);
    /*
     * Instead of passing the parameters to Joy, it is also possible to read
     * an index file and pass the offset and length to Joy.
     */
    if ((leng = search_index(str, &offset)) == 0) {
	fprintf(stderr, "Verse not found\n");
	return 1;
    }
    NULLARY(INTEGER_NEWNODE, offset);
    NULLARY(INTEGER_NEWNODE, leng);
    return 0;
}

/*
 * The result of the joy function is present at the top of the stack.
 */
void end_of_joy(pEnv env)
{
    int i, j = 0;
    char *temp, *ptr;

    /*
     * Read the string from the stack. Words are followed by space characters.
     */
    temp = nodevalue(env->stck).str;
    POP(env->stck);
    /*
     * The first character of a verse needs to be in upper case.
     */
    *temp = toupper((int)*temp);
    /*
     * Set the first non-space character after Q in upper case and shift the
     * remaining string over the Q.
     */
    for (ptr = temp; (ptr = strstr(ptr, " Q ")) != 0; ) {
	strcpy(&ptr[1], &ptr[3]);
	ptr[1] = toupper((int)ptr[1]);
    }
    /*
     * If a punctuation character is seen, then the space preceding it can be
     * removed. The exception is an open parentheses. Here the space after it
     * needs to be removed.
     *
     * ! = 33
     * ' = 39
     * ) = 41
     * , = 44
     * . = 46
     * : = 58
     * ; = 59
     * ? = 63
     */
    for (ptr = temp; (ptr = strpbrk(&ptr[1], "!'),.:;?")) != 0; )
	strcpy(&ptr[-1], ptr);
    for (ptr = temp; (ptr = strchr(&ptr[1], '(')) != 0; )
	strcpy(&ptr[1], &ptr[2]);
    /*
     * After a full stop, the next word must also start with an upper case
     * character.
     */
    for (ptr = temp; (ptr = strstr(&ptr[1], ". ")) != 0; )
	ptr[2] = toupper((int)ptr[2]);
    /*
     * If a quote is followed by a single s or S, then the space character must
     * be removed.
     */
    for (ptr = temp; (ptr = strstr(&ptr[1], "' s ")) != 0; )
	strcpy(&ptr[1], &ptr[2]);
    for (ptr = temp; (ptr = strstr(&ptr[1], "' S ")) != 0; )
	strcpy(&ptr[1], &ptr[2]);
    /*
     * Only one other correction is possible: change -- into an m-dash,
     * preceded by a space character. This sequence occurs only once.
     */

    /*
     * The string can now be split into lines. The maximum length of a line is
     * set to 78 characters.
     */
    for (ptr = temp; *ptr; ptr += j + 1) {
	for (j = i = 0; i <= MAX_LINE && ptr[i]; i++)
	    if (isspace((int)ptr[i]))
		j = i;			/* remember the last break */
	ptr[j] = '\n';			/* replace by newline */
    }
    printf("%s", temp);			/* print the line(s) */
}
