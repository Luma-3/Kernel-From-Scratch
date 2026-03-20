//
// Created by gcaptari on 2/27/26.
//

#ifndef __KFS_STRING_H
#define __KFS_STRING_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

/**
 * String manipulation functions for the kernel.
 * These functions are designed to be simple and efficient, suitable for use in a kernel environment.
 * They do not perform any dynamic memory allocation and assume that the input strings are properly null-terminated.
 */
size_t kstrlen(const char *str);

/**
 * Compares two strings lexicographically.
 * @param s1 The first string to compare.
 * @param s2 The second string to compare.
 * @return An integer less than, equal to, or greater than zero if s1 is found, respectively, to be less than, to match, or be greater than s2.
 */
int kstrcmp(const char *s1, const char *s2);
/**
 * Compares up to n characters of two strings lexicographically.
 * @param s1 The first string to compare.
 * @param s2 The second string to compare.
 * @param n The maximum number of characters to compare.
 * @return An integer less than, equal to, or greater than zero if the first n characters of s1 are found, respectively, to be less than, to match, or be greater than the first n characters of s2.
 */
int kstrncmp(const char *s1, const char *s2, size_t n);
/**
 * Finds the first occurrence of the substring needle in the string haystack.
 * @param haystack The string to search within.
 * @param needle The substring to search for.
 * @return A pointer to the beginning of the located substring, or NULL if the substring is not found.
 */
const char *kstrstr(const char *haystack, const char *needle);
/**
 * Finds the first occurrence of the substring needle in the string haystack, where not more than n characters are searched.
 * @param haystack The string to search within.
 * @param needle The substring to search for.
 * @param n The maximum number of characters to search.
 * @return A pointer to the beginning of the located substring, or NULL if the substring is not found within the first n characters of haystack.
 */
const char *kstrnstr(const char *haystack, const char *needle, size_t n);

/**
 * Finds the first occurrence of the character c in the string str.
 * @param str The string to search within.
 * @param c The character to search for (passed as an int, but interpreted as a char).
 * @return A pointer to the first occurrence of the character in the string, or NULL if the character is not found.
 */
const char *kstrchr(const char *str, int c);

/**
 * Finds the last occurrence of the character c in the string str.
 * @param str The string to search within.
 * @param c The character to search for (passed as an int, but interpreted as a char).
 * @return A pointer to the last occurrence of the character in the string, or NULL if the character is not found.
 */
const char *kstrrchr(const char *str, int c);

/**
 * Checks if the string str starts with the prefix.
 * @param str The string to check.
 * @param prefix The prefix to look for.
 * @return true if str starts with prefix, false otherwise.
 */
bool kstarts_with(const char *str, const char *prefix);

/**
 * Checks if the string str ends with the suffix.
 * @param str The string to check.
 * @param suffix The suffix to look for.
 * @return true if str ends with suffix, false otherwise.
 */
bool kends_with(const char *str, const char *suffix);

/**
 * Converts an integer value to a null-terminated string using the specified base and stores the result in the buffer pointed to by str.
 * @param value The integer value to convert.
 * @param str The buffer where the resulting string will be stored. It must be large enough to hold the resulting string, including the null terminator.
 * @return The number of characters written to the buffer, excluding the null terminator. If an error occurs (e.g., if str is NULL), the function returns 0.
 */
int kitoa(int value, char *str);

#endif //__KFS_STRING_H