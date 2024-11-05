/*
 *  module  : schidx.c
 *  version : 1.1
 *  date    : 10/18/24
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define NODE_SIZE	512	/* size of a node */
#define INT_LENG	9	/* length of internal key */
#define LEAF_LENG	7	/* length of key in leaf nodes */

/*
 * key x is searched in array a with n entries;
 * the comparison is restricted to valid bytes.
 */
static int search_intern(unsigned char *x, unsigned char *a, int n, int valid)
{
    int i, left = 0, right = n - 1;

    if (memcmp(x, a, valid) <= 0)
	return 0;
    if (memcmp(x, a + right * INT_LENG, valid) > 0)
	return n;
    while (right - left > 1) {
	i = left + (right - left) / 2;
	if (memcmp(x, a + i * INT_LENG, valid) <= 0)
	    right = i;
	else
	    left = i;
    }
    return right;
}

/*
 * Search an internal node and return offset and length when found, or offset
 * and 0 when not found.
 */
static int offset_intern(unsigned char *key, unsigned char *buf, int *offset)
{
    short cnt, ptr;	/* short is assumed to be 2 bytes */
    int index, length;	/* int is assumed to be 4 bytes */

    memcpy(&cnt, buf, sizeof(short));
    memcpy(&ptr, &buf[2], sizeof(short));
    /*
     * The index structure contains information about keylen etc.
     */
    index = search_intern(key, &buf[4], cnt, 3);
    /*
     * If within bounds and equal, the key has been found.
     */
    if (index < cnt && !memcmp(key, &buf[4 + index * INT_LENG], 3)) {
	length = buf[4 + index * INT_LENG + 6];
		 buf[4 + index * INT_LENG + 6] = 0;
	memcpy(offset, &buf[4 + index * INT_LENG + 3], sizeof(int));
	return length;
    }
    /*
     * Otherwise, the index points to the subtree to be tried.
     */
    if (index)
	memcpy(&ptr, &buf[4 + index * INT_LENG - sizeof(short)], sizeof(short));
    *offset = ptr;
    *offset *= NODE_SIZE;
    return 0;
}

/*
 * key x is searched in array a with n entries;
 * the comparison is restricted to valid bytes.
 */
static int search_leaf(unsigned char *x, unsigned char *a, int n, int valid)
{
    int i, left = 0, right = n - 1;

    if (memcmp(x, a, valid) <= 0)
	return 0;
    if (memcmp(x, a + right * LEAF_LENG, valid) > 0)
	return n;
    while (right - left > 1) {
	i = left + (right - left) / 2;
	if (memcmp(x, a + i * LEAF_LENG, valid) <= 0)
	    right = i;
	else
	    left = i;
    }
    return right;
}

/*
 * Search an internal node and return offset and length when found, or offset
 * and 0 when not found.
 */
static int offset_leaf(unsigned char *key, unsigned char *buf, int *offset)
{
    short cnt;		/* short is assumed to be 2 bytes */
    int index, length;	/* int is assumed to be 4 bytes */

    memcpy(&cnt, buf, sizeof(short));
    /*
     * The index structure contains information about keylen etc.
     */
    index = search_leaf(key, &buf[4], cnt, 3);
    /*
     * If within bounds and equal, the key has been found.
     */
    if (index < cnt && !memcmp(key, &buf[4 + index * LEAF_LENG], 3)) {
	length = buf[4 + index * LEAF_LENG + 6];
		 buf[4 + index * LEAF_LENG + 6] = 0;
	memcpy(offset, &buf[4 + index * LEAF_LENG + 3], sizeof(int));
	return length;
    }
    return 0;
}

/*
 * The task is to read the root node, followed by internal nodes, until the key
 * has been found; return offset and length, or 0 when the key was not found.
 */
int search_index(unsigned char *key, int *offset)
{
    FILE *fp;
    int length = 0;
    unsigned char buf[NODE_SIZE];

    /*
     * Read the root node from the index file.
     */
    if ((fp = fopen("bible.idx", "rb")) == 0) {
	fprintf(stderr, "could not read bible.idx\n");
	return 0;
    }
    /*
     * First search the root node, that has the same format as an internal node.
     */
    fseek(fp, -NODE_SIZE, SEEK_END);
    fread(buf, NODE_SIZE, 1, fp);
    if ((length = offset_intern(key, buf, offset)) != 0)
	goto einde;
    /*
     * Otherwise, the index points to the subtree to be tried.
     */
    fseek(fp, *offset, SEEK_SET);
    fread(buf, NODE_SIZE, 1, fp);
    if ((length = offset_intern(key, buf, offset)) != 0)
	goto einde;
    /*
     * And if that fails, try a leaf node.
     */
    fseek(fp, *offset, SEEK_SET);
    fread(buf, NODE_SIZE, 1, fp);
    length = offset_leaf(key, buf, offset);
einde:    
    fclose(fp);
    return length;
}
