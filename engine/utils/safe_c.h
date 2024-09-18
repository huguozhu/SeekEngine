#ifndef SAFEC_INCLUDE
#define SAFEC_INCLUDE

#include <stdlib.h>
#include <stdarg.h>

#if defined _WIN32 && !defined __WINOS__
#define __WINOS__
#endif

#define __NOT_USE_UTIL__

#define SSB_NAMESPACE		ssb				//SSB

#define NAMESPACE_BEGIN(x)  namespace x {
#define NAMESPACE_END	}
#define USING_NAMESPACE(x) using namespace x;

#define BEGIN_SSB_NAMESPACE		NAMESPACE_BEGIN(SSB_NAMESPACE)
#define END_SSB_NAMESPACE		NAMESPACE_END
#define USING_SSB_NAMESPACE		USING_NAMESPACE(SSB_NAMESPACE)

#define __EXPORT__

BEGIN_SSB_NAMESPACE
const size_t  MAX_LIMIT_MEM_SIZE = 100 * 1024 * 1024UL;
const size_t  MIN_LIMIT_MEM_SIZE = 1UL;
const size_t MAX_LIMIT_STR_SIZE = 64 * 1024UL;
const size_t MIN_LIMIT_STR_SIZE = 1UL;


enum{
	ESNULLP =   ( 400 ),       /* null ptr                    */
	ESZEROL =   ( 401 ),       /* length is zero              */
	ESLEMIN =   ( 402 ),       /* length is below min         */
	ESLEMAX =  ( 403 ),       /* length exceeds max          */
	ESOVRLP =   ( 404 ),       /* overlap undefined           */
	ESEMPTY =   ( 405 ),       /* empty string                */
	ESNOSPC =  ( 406 ),       /* not enough space for s2     */
	ESUNTERM =( 407 ),       /* unterminated string         */
	ESNODIFF =  ( 408 ),       /* no difference               */
	ESNOTFND= ( 409 ),       /* not found                   */
	EOK =           ( 0 )
};

/*Parameters
dest	pointer to memory to compare against
src	pointer to the source memory to compare with dest
dmax	maximum length of dest, in bytess
smax	length of the source memory block
*diff	pointer to the diff which is an integer greater than, equal to or less than zero according to whether the object pointed to by dest is greater than, equal to or less than the object pointed to by src.
Precondition
Neither dest nor src shall be a null pointer.
Neither dmax nor smax shall be 0.
dmax shall not be greater than MAX_LIMIT_MEM_SIZE.
smax shall not be greater than dmax.
Return values
0	when operation is successful
others are failed
*/
int __EXPORT__ memcmp_s (const void *dest, size_t dmax,	  const void *src,  size_t smax, int *diff);

/*Parameters
dest	-	pointer to the object to fill
ch	-	fill byte
count	-	number of bytes to fill
destsz	-	size of the destination array
Return value
zero on success, non-zero on error. Also on error, if dest is not a null pointer and destsz is valid, writes destsz fill bytes ch to the destination array.
*/
int __EXPORT__ seek_memset_s( void *dest, size_t destsz, int ch, size_t count );

#ifndef __WINOS__
#define strnstr(a, b, c) strstr(a, b)
/*memory copy*/
/*Parameters
dest	-	pointer to the object to copy to
destsz	-	max number of bytes to modify in the destination (typically the size of the destination object)
src	-	pointer to the object to copy from
count	-	number of bytes to copy
Return value
1) Returns a copy of dest
2) Returns zero on success and non-zero value on error. Also on error, if dest is not a null pointer and destsz is valid, writes destsz zero bytes in to the destination array.
*/
int __EXPORT__ memcpy_s( void * dest, size_t destsz,	 const void * src, size_t count );

/*memory move */
/*Parameters
dest	-	pointer to the object to copy to
destsz	-	max number of bytes to modify in the destination (typically the size of the destination object)
src	-	pointer to the object to copy from
count	-	number of bytes to copy
Return value
1) Returns a copy of dest
2) Returns zero on success and non-zero value on error. Also on error, if dest is not a null pointer and destsz is valid, writes destsz zero bytes in to the destination array.
*/
int __EXPORT__ memmove_s(void *dest, size_t destsz, const void *src, size_t count);

/*Parameters
dest	-	pointer to the object to fill
ch	-	fill byte
count	-	number of bytes to fill
destsz	-	size of the destination array
Return value
1) A copy of dest
2) zero on success, non-zero on error. Also on error, if dest is not a null pointer and destsz is valid, writes destsz fill bytes ch to the destination array.
*/
int __EXPORT__ memset_s( void *dest, size_t destsz, int ch, size_t count );

/*string length*/
/*Parameters
str	-	pointer to the null-terminated byte string to be examined
strsz	-	maximum number of characters to examine
Return value
1) The length of the null-terminated byte string str.
2) The length of the null-terminated byte string str on success, zero if str is a null pointer, strsz if the null character was not found.
*/
size_t __EXPORT__ strnlen_s( const char *str, size_t strsz );

/*
Parameters
str	-	pointer to the null-terminated wide string to be examined
strsz	-	maximum number of wide characters to examine
Return value
1) The length of the null-terminated wide string str.
2) The length of the null-terminated wide string str on success, zero if str is a null pointer, strsz if the null wide character was not found.
*/
size_t __EXPORT__ wcsnlen_s(const wchar_t *str, size_t strsz);

/*Parameters
dest	-	pointer to the null-terminated byte string to append to
src	-	pointer to the null-terminated byte string to copy from
destsz	-	maximum number of characters to write, typically the size of the destination buffer
Return value
1) returns a copy of dest
2) returns zero on success, returns non-zero on error. Also, on error, writes zero to dest[0] (unless dest is a null pointer or destsz is zero or greater than MAX_LIMIT_STR_SIZE).
*/
int __EXPORT__  strcat_s(char * dest, size_t destsz, const char * src);

/*
Parameters
dest	-	pointer to the character array to write to
src	-	pointer to the null-terminated byte string to copy from
destsz	-	maximum number of characters to write, typically the size of the destination buffer
Return value
1) returns a copy of dest
2) returns zero on success, returns non-zero on error. Also, on error, writes zero to dest[0] (unless dest is a null pointer or destsz is zero or greater than RSIZE_MAX).
Notes
*/
int __EXPORT__ strcpy_s(char * dest, size_t destsz, const char * src);
/*Parameters
dest	-	pointer to the character array to copy to
src	-	pointer to the character array to copy from
count	-	maximum number of characters to copy
destsz	-	the size of the destination buffer
Return value
1) returns a copy of dest
2) returns zero on success, returns non-zero on error. Also, on error, writes zero to dest[0] (unless dest is a null pointer or destsz is zero or greater than MAX_LIMIT_STR_SIZE) and may clobber the rest of the destination array with unspecified values.
*/
int __EXPORT__ strncpy_s(char * dest, size_t destsz,  const char * src, size_t count);

/*Parameters
str	-	pointer to the null-terminated byte string to tokenize
delim	-	pointer to the null-terminated byte string identifying delimiters
strmax	-	pointer to an object which initially holds the size of str: strtok_s stores the number of characters that remain to be examined
ptr	-	pointer to an object of type char*, which is used by strtok_s to store its internal state
Return value
Returns pointer to the beginning of the next token or NULL if there are no more tokens.
*/
char  __EXPORT__ *strtok_s(char * str, size_t * strmax,  const char * delim, char ** ptr);

/*Parameters
buffer	-	pointer to a character string to write to
bufsz	-	up to bufsz - 1 characters may be written, plus the null terminator
format	-	pointer to a null-terminated multibyte string specifying how to interpret the data.
Return value
1,2) number of characters transmitted to the output stream or negative value if an output error or an encoding error (for string and character conversion specifiers) occurred
3) number of characters written to buffer (not counting the terminating null character), or a negative value if an encoding error (for string and character conversion specifiers) occurred
4) number of characters (not including the terminating null character) which would have been written to buffer if bufsz was ignored, or a negative value if an encoding error (for string and character conversion specifiers) occurred
5,6) number of characters transmitted to the output stream or negative value if an output error, a runtime constrants violation error, or an encoding error occurred.
7) number of characters written to buffer, not counting the null character (which is always written as long as buffer is not a null pointer and bufsz is not zero and not greater than MAX_LIMIT_STR_SIZE), or zero on runtime constraint violations, and negative value on encoding errors
8) number of characters not including the terminating null character (which is always written as long as buffer is not a null pointer and bufsz is not zero and not greater than MAX_LIMIT_STR_SIZE), which would have been written to buffer if bufsz was ignored, or a negative value if a runtime constraints violation or an encoding error occurred
*/
int __EXPORT__ sprintf_s(char * buffer, size_t bufsz,  const char *format, ...);
int __EXPORT__ snprintf_s(char * buffer, size_t bufsz, const char * format, ...);


//////////////////////////////////////////////////////////////////////////
#if !defined __MACOS__ && !defined __IOS__
/*
Parameters
name	-	null-terminated character string identifying the name of the environmental variable to look for
len	-	pointer to a user-provided location where getenv_s will store the length of the environment variable
value	-	pointer to a user-provided character array where getenv_s will store the contents of the environment variable
valuesz	-	maximum number of characters that getenv_s is allowed to write to dest (size of the buffer)
Return value
1) character string identifying the value of the environmental variable or null pointer if such variable is not found.
2) zero if the environment variable was found, non-zero if it was not found of if a runtime constrant violation occurred. On any error, writes zero to *len (unless len is a null pointer).
*/
int __EXPORT__ getenv_s( size_t * len, char * value, size_t valuesz, const char * name );
#endif

/*
Parameters
_src - the input string which we need to grab something out
_format - the pattern we need to catch out
... - output args
Return value
	the number values to get from the _src, -1 is failed
*/
int __EXPORT__ sscanf_s(const char * _src, const char * _format, ...);

/*
Composes a string with the same text that would be printed if format was used on printf, but instead of being printed, the content is stored as a C string in the buffer pointed by s (taking n as the maximum buffer capacity to fill).
If the resulting string would be longer than n-1 characters, the remaining characters are discarded and not stored, but counted for the value returned by the function.
A terminating null character is automatically appended after the content written.
After the format parameter, the function expects at least as many additional arguments as needed for format.

Parameters:
_dstbuf->Pointer to a buffer where the resulting C-string is stored.
The buffer should have a size of at least n characters.

_dstbuf_size->Maximum number of bytes to be used in the buffer.
The generated string has a length of at most n-1, leaving space for the additional terminating null character.
size_t is an unsigned integral type.
_max_limit-> The max limition, should less or equal _dstbuf_size here
_format-> C string that contains a format string that follows the same specifications as format in printf (see printf for details).

Return:
The number of characters that would have been written if _dstbuf_size had been sufficiently large, not counting the terminating null character.
If an encoding error occurs, a negative number is returned.
Notice that only when this returned value is non-negative and less than _dstbuf_size, the string has been completely written.
*/
int __EXPORT__ vsnprintf_s(char * _dstbuf, size_t _dstbuf_size, const char * _format, va_list _args);

/*
Composes a string with the same text that would be printed if format was used on printf, but instead of being printed, the content is stored as a C string in the buffer pointed by s (taking n as the maximum buffer capacity to fill).
If the resulting string would be longer than n-1 characters, the remaining characters are discarded and not stored, but counted for the value returned by the function.
A terminating null character is automatically appended after the content written.
After the format parameter, the function expects at least as many additional arguments as needed for format.

Parameters:
_dstbuf->Pointer to a buffer where the resulting C-string is stored.
The buffer should have a size of at least n characters.

_dstbuf_size->Maximum number of bytes to be used in the buffer.
The generated string has a length of at most n-1, leaving space for the additional terminating null character.
size_t is an unsigned integral type.
_format-> C string that contains a format string that follows the same specifications as format in printf (see printf for details).

Return:
The number of characters that would have been written if _dstbuf_size had been sufficiently large, not counting the terminating null character.
If an encoding error occurs, a negative number is returned.
Notice that only when this returned value is non-negative and less than _dstbuf_size, the string has been completely written.
*/
int __EXPORT__ vsprintf_s(char * _dstbuf, size_t _dstbuf_size, const char * _format, va_list _args);
#endif
END_SSB_NAMESPACE
#endif
