//
// Created by gcaptari on 2/27/26.
//

#ifndef __KFS_MATH_H
#define __KFS_MATH_H


#include <stddef.h>
/**
 * Computes the absolute value of an integer.
 * @param x The integer for which to compute the absolute value.
 * @return The absolute value of x.
 */
int kabs(int x);

/**
 * Computes the maximum of two integers.
 * @param a The first integer.
 * @param b The second integer.
 * @return The larger of the two integers.
 */
int kmax(int a, int b);

/**
 * Computes the minimum of two integers.
 * @param a The first integer.
 * @param b The second integer.
 * @return The smaller of the two integers.
 */
int kmin(int a, int b);

/**
 * Computes the power of an integer.
 * @param base The base value.
 * @param exp The exponent value. If exp is negative, the function returns 0 for simplicity.
 * @return The result of base raised to the power of exp.
 */
int kpow(int base, int exp);

/**
 * Computes the absolute value of a long integer.
 * @param x The long integer for which to compute the absolute value.
 * @return The absolute value of x.
 */
long klabs(long x);

/**
* Computes the maximum of two long integers.
* @param a The first long integer.
* @param b The second long integer.
* @return The larger of the two long integers.
*/
long klmax(long a, long b);

/**
 * Computes the minimum of two long integers.
 * @param a The first long integer.
 * @param b The second long integer.
 * @return The smaller of the two long integers.
 */
long klmin(long a, long b);

/**
 * Computes the power of a long integer.
 * @param base The base value.
 * @param exp The exponent value. If exp is negative, the function returns 0 for simplicity.
 * @return
 */
long klpow(long base, long exp);


#endif //__KFS_MATH_H