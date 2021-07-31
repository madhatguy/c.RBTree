/**
 * @file Structs.c
 * @author  Guy Lev <thehatguy>
 * @version 1.0
 * @date 22 may 2020
 *
 * @brief Functions to use on an RBTree.
 *
 * @section DESCRIPTION
 * Contains functions to use in a vector RBTree and strings RBTree.
 */
// ------------------------------ includes ------------------------------
#include "Structs.h"
#include <stdlib.h>
#include <string.h>
// -------------------------- const definitions -------------------------
#define FAILURE (0)

#define SUCCESS (1)

const int VECTOR_AMOUNT = 1;

const int START_VAL = 0;

const int STARTING_IDX = 0;

const int SMALLER = -1;

const int GREATER = 1;
// ------------------------------ functions -----------------------------
/**
 * CompFunc for strings (assumes strings end with "\0")
 * @param a - char* pointer
 * @param b - char* pointer
 * @return equal to 0 iff a == b. lower than 0 if a < b. Greater than 0 iff b < a. (lexicographic
 * order)
 */
int stringCompare(const void *a, const void *b)
{
    return strcmp((char *) a, (char *) b);
}

/**
 * ForEach function that concatenates the given word and \n to pConcatenated. pConcatenated is
 * already allocated with enough space.
 * @param word - char* to add to pConcatenated
 * @param pConcatenated - char*
 * @return 0 on failure, other on success
 */
int concatenate(const void *word, void *pConcatenated)
{
    char *res = strcat((char *) pConcatenated, (char *) word);
    if (res == NULL)
    {
        return FAILURE;
    }
    return SUCCESS;
}

/**
 * FreeFunc for strings
 */
void freeString(void *s)
{
    free((char *) s);
}

/**
 * CompFunc for Vectors, compares element by element, the vector that has the first larger
 * element is considered larger. If vectors are of different lengths and identify for the length
 * of the shorter vector, the shorter vector is considered smaller.
 * @param a - first vector
 * @param b - second vector
 * @return equal to 0 iff a == b. lower than 0 if a < b. Greater than 0 iff b < a.
 */
int vectorCompare1By1(const void *a, const void *b)
{
    Vector *vecA = (Vector *) a;
    Vector *vecB = (Vector *) b;
    int shortVec, len;
    if (vecA->len == vecB->len)
    {
        len = vecA->len;
        shortVec = START_VAL;
    }
    else if (vecA->len < vecB->len)
    {
        len = vecA->len;
        shortVec = SMALLER;
    }
    else
    {
        len = vecB->len;
        shortVec = GREATER;
    }
    double aCoord, bCoord;
    for (int i = STARTING_IDX; i < len; ++i)
    {
        aCoord = vecA->vector[i], bCoord = vecB->vector[i];
        if (aCoord < bCoord)
        {
            return SMALLER;
        }
        else if (aCoord > bCoord)
        {
            return GREATER;
        }
    }
    return shortVec;
}

/**
 * FreeFunc for vectors
 */
void freeVector(void *pVector)
{
    Vector *pVec = (Vector *) pVector;
    free(pVec->vector);
    free(pVector);
}

/**
 * @param vec The vector to get the norm of
 * @return The norm of vec
 */
double getNorm(Vector *vec, int vecLen)
{
    double squaredNorm = START_VAL;
    for (int i = STARTING_IDX; i < vecLen; ++i)
    {
        squaredNorm += (vec->vector[i] * vec->vector[i]);
    }
    return squaredNorm;
}

/**
 * copy pVector to pMaxVector if : 1. The norm of pVector is greater then the norm of pMaxVector.
 * 								   2. pMaxVector->vector == NULL.
 * @param pVector pointer to Vector
 * @param pMaxVector pointer to Vector
 * @return 1 on success, 0 on failure (if pVector == NULL: failure).
 */
int copyIfNormIsLarger(const void *pVector, void *pMaxVector)
{
    Vector *pVec = (Vector *) pVector;
    Vector *pMaxVec = (Vector *) pMaxVector;
    if (pVec == NULL || pMaxVec == NULL)
    {
        return FAILURE;
    }
    double curNorm = getNorm(pVec, pVec->len);
    double maxNorm = getNorm(pMaxVec, pMaxVec->len);
    if (curNorm <= maxNorm)
    {
        return SUCCESS;
    }
    double *alloc = (double *) realloc(pMaxVec->vector, pVec->len * sizeof(double));
    if (alloc == NULL)
    {
        return FAILURE;
    }
    pMaxVec->vector = alloc;
    pMaxVec->len = pVec->len;
    for (int i = STARTING_IDX; i < pVec->len; ++i)
    {
        pMaxVec->vector[i] = pVec->vector[i];
    }
    return SUCCESS;
}

/**
 * @param tree a pointer to a tree of Vectors
 * @return pointer to a *copy* of the vector that has the largest norm (L2 Norm).
 */
Vector *findMaxNormVectorInTree(RBTree *tree) // You must use copyIfNormIsLarger in the implementation!
{
    Vector *vec = (Vector *) calloc(VECTOR_AMOUNT, sizeof(Vector));
    if (vec == NULL)
    {
        return NULL;
    }
    if (!forEachRBTree(tree, copyIfNormIsLarger, (void *) vec))
    {
        freeVector((void *) vec);
        return NULL;
    }
    return vec;
}
