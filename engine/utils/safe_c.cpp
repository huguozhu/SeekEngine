#include "safe_c.h"
/*
	If you use the complete util library and don't want to use safec alone, 
	you cannot define the __NOT_USE_UTIL__ macro in the project file;

	If you want to use safec alone instead of using the complete util library, 
	you need to copy the platform.h, safec.h, and safec.cpp files, 
	and then define the __NOT_USE_UTIL__ macro in the project file;

	Using the complete util library and defining the __NOT_USE_UTIL__ macro are mutually exclusive, please be careful.
*/

#ifdef __NOT_USE_UTIL__
	#include <stdarg.h>
	#include <stdio.h>
	#include <string.h>
	#include <errno.h>
	#include <stdint.h>
	#include <string>

	#define VAR(x) ", "<<#x<<" = "<<(x)
	#define IS_FALSE(x) (!(x))
	#define UT_FATAL_LOG(msg) 

	#define MSG_ASSERT_RETURN(msg, x, err)	 \
		do {\
			if (IS_FALSE(x)) {\
				return err;\
			}\
		} while (false);
#else
	#include "utils/h/auxi.h"
	#include "utils/h/times.h"
	#include "utils/h/assert.hpp"
	#include "utils/h/log_t.h"
	#include <stdarg.h>
	#include <stdio.h>
#endif //__NOT_USE_UTIL__

#ifndef __WINOS__
#if !defined __MACOS__ && !defined __IOS__
#if defined __USE_GNU || defined __ANDROIDOS__
extern char **environ;
#else
extern char **__environ;
#endif
#endif
#endif

BEGIN_SSB_NAMESPACE
int  memcmp_s (const void *dest, size_t dmax, const void *src,  size_t smax, int *diff){
	MSG_ASSERT_RETURN("memcmp_s invalid parameters differ" << VAR((void*)diff) << VAR((void *)dest) << VAR((void *)src), NULL != dest && NULL != src && NULL != diff, ESNULLP);
	MSG_ASSERT_RETURN("memcmp_s invalid parameters" << VAR(dmax) <<  VAR(smax), dmax >= MIN_LIMIT_MEM_SIZE && smax >= MIN_LIMIT_MEM_SIZE && smax <= dmax, ESLEMIN);
	MSG_ASSERT_RETURN("memcmp_s invalid parameters" << VAR(dmax) <<  VAR(smax), dmax <= MAX_LIMIT_MEM_SIZE && smax <= dmax, ESLEMAX);
	*diff = 0;
	uint8_t *s_cousor = (uint8_t *)src, *d_cousor = (uint8_t *)dest;
	for(size_t idx = 0; idx < smax; ++idx){
		if( *d_cousor != *s_cousor){
			*diff = *d_cousor  < *s_cousor ? -1 : 1;;
			break;
		}
		++s_cousor; 
		++d_cousor;
	}
	return EOK;
}

int seek_memset_s( void *dest, size_t destsz, int ch, size_t count ){
	if(destsz == 0 || count == 0) {
		return EOK;
	}
	MSG_ASSERT_RETURN("seek_memset_s invalid parameters" << VAR(dest) << VAR(destsz) << VAR(ch) << VAR(count), NULL != dest, ESNULLP);
	MSG_ASSERT_RETURN("seek_memset_s invalid parameters" << VAR(destsz) << VAR(count), destsz >= MIN_LIMIT_MEM_SIZE && count >= MIN_LIMIT_MEM_SIZE, ESLEMIN);
	MSG_ASSERT_RETURN("seek_memset_s invalid parameters" << VAR(destsz) << VAR(count), destsz <= MAX_LIMIT_MEM_SIZE, ESLEMAX);
	MSG_ASSERT_RETURN("seek_memset_s invalid parameters" << VAR(destsz) << VAR(count), count <= destsz, ESNOSPC);
	memset(dest, ch, count);/*checked safe*/
	return EOK;
}


#ifndef __WINOS__
int memcpy_s( void * dest,  size_t destsz,	const void * src, size_t count ){
	MSG_ASSERT_RETURN("memcpy_s invalid parameters" << VAR(dest) << VAR(destsz) << VAR((void *)src) << VAR(count), NULL != dest && NULL != src, ESNULLP);
	MSG_ASSERT_RETURN("memcpy_s invalid parameters" << VAR(destsz) << VAR(count), destsz >= MIN_LIMIT_MEM_SIZE && count >= MIN_LIMIT_MEM_SIZE, ESLEMIN);
	MSG_ASSERT_RETURN("memcpy_s invalid parameters" << VAR(destsz) << VAR(count), destsz <= MAX_LIMIT_MEM_SIZE, ESLEMAX);
	MSG_ASSERT_RETURN("memcpy_s invalid parameters" << VAR(destsz) << VAR(count), count <= destsz, ESNOSPC);
	MSG_ASSERT_RETURN("memcpy_s invalid parameters overlap is not allow" << VAR(dest) << VAR((void *)src)<< VAR(count), (((int8_t *)dest) < (int8_t *)src && ((int8_t*)dest  + count) <= ((int8_t *)src)) ||(((int8_t *)dest) > (int8_t *)src && ((int8_t*)dest ) >= ((int8_t *)src + count)), ESOVRLP);
	memcpy(dest, src, count);/*checked safe*/
	return EOK;
}

int  memmove_s(void *dest, size_t destsz, const void *src, size_t count){
	MSG_ASSERT_RETURN("memmove_s invalid parameters" << VAR(dest) << VAR(destsz) << VAR((void *)src) << VAR(count), NULL != dest && NULL != src, ESNULLP);
	MSG_ASSERT_RETURN("memmove_s invalid parameters" << VAR(destsz) << VAR(count), destsz >= MIN_LIMIT_MEM_SIZE && count >= MIN_LIMIT_MEM_SIZE, ESLEMIN);
	MSG_ASSERT_RETURN("memmove_s invalid parameters" << VAR(destsz) << VAR(count), destsz <= MAX_LIMIT_MEM_SIZE, ESLEMAX);
	MSG_ASSERT_RETURN("memmove_s invalid parameters" << VAR(destsz) << VAR(count), count <= destsz, ESNOSPC);
	memmove(dest, src, count);/*checked safe*/
	return EOK;
}

int memset_s( void *dest, size_t destsz, int ch, size_t count ){
	MSG_ASSERT_RETURN("memset_s invalid parameters" << VAR(dest) << VAR(destsz) << VAR(ch) << VAR(count), NULL != dest, ESNULLP);
	MSG_ASSERT_RETURN("memset_s invalid parameters" << VAR(destsz) << VAR(count), destsz >= MIN_LIMIT_MEM_SIZE && count >= MIN_LIMIT_MEM_SIZE, ESLEMIN);
	MSG_ASSERT_RETURN("memset_s invalid parameters" << VAR(destsz) << VAR(count), destsz <= MAX_LIMIT_MEM_SIZE, ESLEMAX);
	MSG_ASSERT_RETURN("memset_s invalid parameters" << VAR(destsz) << VAR(count), count <= destsz, ESNOSPC);
	memset(dest, ch, count);/*checked safe*/
	return EOK;
}


/**/
int strcat_s(char * dest, size_t destsz, const char * src){
	MSG_ASSERT_RETURN("strcat_s invalid parameters" << VAR(dest) << VAR(destsz) << VAR((void *)src), NULL != dest && NULL != src, ESNULLP);
	MSG_ASSERT_RETURN("strcat_s invalid parameters" << VAR(dest) << VAR(destsz) << VAR((void *)src),  destsz <= MAX_LIMIT_MEM_SIZE, ESLEMAX);
	MSG_ASSERT_RETURN("strcat_s invalid parameters" << VAR(dest) << VAR(destsz) << VAR((void *)src), destsz >= MIN_LIMIT_MEM_SIZE, ESLEMIN);
	size_t _srclen = strnlen_s(dest, MAX_LIMIT_STR_SIZE);
	MSG_ASSERT_RETURN("strcat_s invalid parameters" << VAR(dest) << VAR(destsz) << VAR((void *)src), destsz > _srclen, ESNOSPC);
	char *_cursor1 =(dest + _srclen), *_cursor2 = (char *)src;
	size_t _cats = _srclen;
	char *_overlap_cursor;
	if(_cursor1 > _cursor2){
		_overlap_cursor = _cursor1;
	while(*_cursor2 != '\0'){
			MSG_ASSERT_RETURN("strcat_s, overlap is not allow" << VAR(dest) << VAR(src) << VAR(_overlap_cursor), _overlap_cursor != _cursor2, ESOVRLP);
		*_cursor1++ = *_cursor2++;
			++_cats;
			if(_cats >= destsz){
				return ESNOSPC;
			}
		}
	}
	else{
		_overlap_cursor = _cursor2;
	while(*_cursor2 != '\0'){
			MSG_ASSERT_RETURN("strcat_s, overlap is not allow" << VAR(dest) << VAR(src) << VAR(_overlap_cursor), _overlap_cursor != _cursor1, ESOVRLP);
		*_cursor1++ = *_cursor2++;
			++_cats;
			if(_cats >= destsz){
				return ESNOSPC;
			}
		}
	}
	*_cursor1 = '\0';
	return EOK;
}

size_t strnlen_s( const char *str, size_t strsz ){
	MSG_ASSERT_RETURN("strnlen_s string size limitation out of scope" << VAR(strsz), strsz <= MAX_LIMIT_STR_SIZE, 0);
	size_t _len = 0;
	char *_cursor = (char *)str;
	if(_cursor){
		while(*_cursor++ != '\0'){
			++_len;
			MSG_ASSERT_RETURN("strnlen_s string size limitation out of scope" << VAR(_len), _len < strsz, _len);
		}
	}
	return _len;
}


size_t wcsnlen_s(const wchar_t *str, size_t strsz){
	MSG_ASSERT_RETURN("wcsnlen_s string size limitation out of scope" << VAR(strsz), strsz <= MAX_LIMIT_STR_SIZE, 0);
	if(NULL == str) return 0;
	std::wstring _str((std::wstring::value_type *)str);
	return _str.length();
}
int strcpy_s(char * dest, size_t destsz, const char * src){
	MSG_ASSERT_RETURN("strcpy_s invalid parameters" << VAR((void *)dest) << VAR(destsz) << VAR((void *)src) , NULL != dest && NULL != src, ESNULLP);
	MSG_ASSERT_RETURN("strcpy_s invalid parameters" << VAR(destsz), destsz >= MIN_LIMIT_STR_SIZE , ESLEMIN);
	MSG_ASSERT_RETURN("strcpy_s invalid parameters" << VAR(destsz), destsz <= MAX_LIMIT_STR_SIZE , ESLEMAX);
	char *_cursor1 = dest, *_cursor2 = (char *)src,  *_overlap_cursor;
	size_t _count = 0;
	if(_cursor1 < _cursor2){
		_overlap_cursor = _cursor2;
	while(*_cursor2 != '\0'){
			MSG_ASSERT_RETURN("strcpy_s, overlap is not allow" << VAR(dest) << VAR(src) << VAR(_overlap_cursor), _overlap_cursor != _cursor1, ESOVRLP);
		*_cursor1++ = *_cursor2++;
			++_count;
			MSG_ASSERT_RETURN("strcpy_s, space is not enough" << VAR(dest) << VAR(src) << VAR(destsz), _count < destsz, ESNOSPC);
		}
	}
	else{
		_overlap_cursor = _cursor1;
	while(*_cursor2 != '\0'){
			MSG_ASSERT_RETURN("strcpy_s, overlap is not allow" << VAR(dest) << VAR(src) << VAR(_overlap_cursor), _overlap_cursor != _cursor2, ESOVRLP);
		*_cursor1++ = *_cursor2++;
			++_count;
			MSG_ASSERT_RETURN("strcpy_s, space is not enough" << VAR(dest) << VAR(src) << VAR(destsz), _count < destsz, ESNOSPC);
		}
	}
	*_cursor1 = '\0';
	return EOK;
}
int strncpy_s(char * dest, size_t destsz,  const char * src, size_t count){
	MSG_ASSERT_RETURN("strncpy_s invalid parameters" << VAR(dest) << VAR((void *)src), NULL != dest && NULL != src, ESNULLP);
	MSG_ASSERT_RETURN("strncpy_s invalid parameters" << VAR(destsz) << VAR(count), destsz <= MAX_LIMIT_STR_SIZE && count <= MAX_LIMIT_STR_SIZE, ESLEMAX);
	MSG_ASSERT_RETURN("strncpy_s invalid parameters" << VAR(destsz) << VAR(count), destsz >= MIN_LIMIT_STR_SIZE && count >= MIN_LIMIT_STR_SIZE , ESLEMIN);
	MSG_ASSERT_RETURN("strncpy_s invalid parameters" << VAR(destsz) << VAR(count), count <= destsz, ESNOSPC);
	char *_cursor1 = dest, *_cursor2 = (char *)src,  *_overlap_cursor;
	size_t _idx = 0;
	if(_cursor1 > _cursor2){
		_overlap_cursor = _cursor1;
		while(*_cursor2 != '\0' && _idx < count){
			MSG_ASSERT_RETURN("strncpy_s invalid parameters overlap is not allow" << VAR((void *)dest) << VAR((void *)src)<< VAR(count) << VAR(destsz),  _cursor2 != _overlap_cursor, ESOVRLP);
		*_cursor1++ = *_cursor2++;
			++_idx;
			MSG_ASSERT_RETURN("strncpy_s, space is not enough" << VAR(dest) << VAR(src) << VAR(destsz), _idx < destsz, ESNOSPC);
		}
	}
	else{
		_overlap_cursor = _cursor2;
		while(*_cursor2 != '\0' && _idx < count){
			MSG_ASSERT_RETURN("strncpy_s invalid parameters overlap is not allow" << VAR((void *)dest) << VAR((void *)src)<< VAR(count) << VAR(destsz),  _cursor1 != _overlap_cursor, ESOVRLP);
		*_cursor1++ = *_cursor2++;
			++_idx;
			MSG_ASSERT_RETURN("strncpy_s, space is not enough" << VAR(dest) << VAR(src) << VAR(destsz), _idx < destsz, ESNOSPC);
	}
	}
	if(_idx < destsz)
	*_cursor1 = '\0';
	else
		dest[_idx -1 ] = '\0';
	return EOK;
}

char *strtok_s(char * str, size_t * strmax,  const char * delim, char ** ptr){
	MSG_ASSERT_RETURN("strtok_s invalid parameters" << VAR((void *)str) << VAR((void *)strmax) << VAR((void *)delim) << VAR((void *)ptr), NULL != strmax && *strmax <= MAX_LIMIT_STR_SIZE && NULL != delim && NULL != ptr, NULL);
	if(NULL == str){
		MSG_ASSERT_RETURN("strtok_s invalid parameters *ptr should be not null if str is null" << VAR((void *)str) << VAR((void *)strmax) << VAR((void *)delim) << VAR((void *)ptr), NULL != *ptr, NULL);
	}
    const char *pt;
    char *ptoken;
    size_t dlen;
    size_t slen;

	const size_t STRTOK_DELIM_MAX_LEN = 64;
    /* if the source was NULL, use the tokenizer context */
    if (NULL == str) {
        str = *ptr;
    }

    /*
     * scan dest for a delimiter
     */
    dlen = *strmax;
    ptoken = NULL;
    while (*str != '\0' && !ptoken) {
        if (dlen == 0) {
            *ptr = NULL;
             return (NULL);
        }

        /*
         * must scan the entire delimiter list
         * ISO should have included a delimiter string limit!!
         */
        slen = STRTOK_DELIM_MAX_LEN;
        pt = delim;
        while (*pt != '\0') {

            if (slen == 0) {
                *ptr = NULL;
                return (NULL);
            }
            slen--;

            if (*str == *pt) {
                ptoken = NULL;
                break;
            } else {
                pt++;
                ptoken = str;
            }
        }
        str++;
        dlen--;
    }

    /*
     * if the beginning of a token was not found, then no
     * need to continue the scan.
     */
    if (ptoken == NULL) {
        *strmax = dlen;
        return (ptoken);
    }

    /*
     * Now we need to locate the end of the token
     */
    while (*str != '\0') {

        if (dlen == 0) {
            *ptr = NULL;
            return (NULL);
        }

        slen = STRTOK_DELIM_MAX_LEN;
        pt = delim;
        while (*pt != '\0') {

            if (slen == 0) {
                *ptr = NULL;
                 return (NULL);
            }
            slen--;

            if (*str == *pt) {
                /*
                 * found a delimiter, set to null
                 * and return context ptr to next char
                 */
                *str = '\0';
                *ptr = (str + 1);  /* return pointer for next scan */
                *strmax = dlen - 1;   /* account for the nulled delimiter */
                return (ptoken);
            } else {
                /*
                 * simply scanning through the delimiter string
                 */
                pt++;
            }
        }
        str++;
        dlen--;
    }

    *strmax = dlen;
    return (ptoken);
}

int sprintf_s(char * buffer, size_t bufsz,  const char *format, ...){

	MSG_ASSERT_RETURN("sprintf_s invalid parameters" << VAR((void *)buffer) << VAR((void *)format), NULL != buffer && NULL != format, -ESNULLP);
	MSG_ASSERT_RETURN("sprintf_s invalid parameters" << VAR(bufsz), bufsz <= MAX_LIMIT_STR_SIZE, -ESLEMAX);
	MSG_ASSERT_RETURN("sprintf_s invalid parameters" << VAR(bufsz), bufsz >= MIN_LIMIT_STR_SIZE, -ESLEMIN);

	va_list ap;
     int ret = -1;

	 va_start(ap, format);

	 ret = vsnprintf(buffer, (size_t)bufsz, format, ap);/*checked safe*/
 
     if (ret >= (int)bufsz) {
         *buffer = 0;
         ret = -ESNOSPC;
     }
     va_end(ap);
      return ret;
}
int snprintf_s(char * buffer, size_t bufsz,  const char *format, ...){
	MSG_ASSERT_RETURN("snprintf_s invalid parameters" << VAR((void *)buffer) << VAR((void *)format), NULL != buffer && NULL != format, -ESNULLP);
	MSG_ASSERT_RETURN("snprintf_s invalid parameters" << VAR(bufsz), bufsz <= MAX_LIMIT_STR_SIZE, -ESLEMAX);
	MSG_ASSERT_RETURN("snprintf_s invalid parameters" << VAR(bufsz), bufsz >= MIN_LIMIT_STR_SIZE, -ESLEMIN);

	va_list ap;
	int ret = -1;
	va_start(ap, format);
	ret = vsnprintf(buffer, (size_t)bufsz, format, ap);/*checked safe*/

	if (ret >= (int)bufsz) {
		*buffer = 0;
		ret = -ESNOSPC;
	}

	va_end(ap);
	return ret;
}
#if 0
#if !defined __MACOS__ && !defined __IOS__
int  getenv_s( size_t * len, char * value, size_t valuesz, const char * name ){
	MSG_ASSERT_RETURN("getenv_s invalid parameters" << VAR((void *)len) << VAR((void *)value) << VAR((void *)name), NULL != len && NULL != value && NULL != name, ESNULLP);
	MSG_ASSERT_RETURN("getenv_s invalid parameters" << VAR(*len) << VAR(valuesz), *len > MIN_LIMIT_STR_SIZE && valuesz > MIN_LIMIT_STR_SIZE, ESLEMIN);
	MSG_ASSERT_RETURN("getenv_s invalid parameters" << VAR(*len) << VAR(valuesz), *len <= valuesz, ESNOSPC);
#ifdef __WINOS__
	DWORD _env_len = GetEnvironmentVariableA((LPCSTR)name, (LPSTR)value, (DWORD)*len);
	if(_env_len == (DWORD)*len){ //not enough
		*len = _env_len;
		return ESNOSPC;
	}
	*len = _env_len;
	return EOK;
#else

#if defined __USE_GNU || defined __ANDROIDOS__
	char **_env = environ;
#else
	char **_env = __environ;
#endif
	MSG_ASSERT_RETURN("getenv_s invalid parameters" << VAR((void *)_env) , NULL != _env, ESNULLP);

	const char *np = name;
	for (; *np && *np != '='; ++np)
		continue;

	size_t _len = np - name;

	char **p = _env, *c;

	for (p = _env; (c = *p) != NULL; ++p)
		if (strncmp(c, name, _len) == 0 && c[_len] == '=') {
			size_t _vlen = p - _env;
			if(_vlen > *len){
				*len = _vlen;
				return ESNOSPC;
			}
			strcpy_s(value, valuesz, c+_len + 1);
			return EOK;
		}
		return ESNOTFND;
#endif
}
#endif

int  sscanf_s(const char * _src, const char * _format, ...){
	va_list ap;
	int ret = -1;
	if(NULL == _src || NULL == _format){
		errno = ESNULLP;
		MSG_ASSERT_RETURN("sscanf_s invalid parameters" << VAR((void *)_src) << VAR((void *)_format), false, -1);
	}
	errno = 0;
	va_start(ap, _format);
	ret = vsscanf(_src, _format, ap);/*checked safe*/
	va_end(ap);
	return ret;
}
#endif
int vsnprintf_s(char * _dstbuf, size_t _dstbuf_size, const char * _format, va_list _args){

	int ret = -1;
	const char *p;

	if(NULL == _dstbuf || NULL == _format){
		errno = ESNULLP;
		MSG_ASSERT_RETURN("vsnprintf_s: invalid parameters" << VAR((void *)_dstbuf) << VAR((void *)_format), false, -(ESNULLP));
	}

	if (0 == _dstbuf_size) {
		errno = ESZEROL;
		MSG_ASSERT_RETURN("vsnprintf_s: _dstbuf_size cannot be zero " << _dstbuf_size, false, -(ESNULLP));
	}
	if (_dstbuf_size > MAX_LIMIT_STR_SIZE) {
		errno = ESLEMAX;
		MSG_ASSERT_RETURN("vsnprintf_s: _dstbuf_size exceeds max" << _dstbuf_size, false, -(ESLEMAX));
	}


	if (((p = strnstr(_format, "%n", MAX_LIMIT_STR_SIZE)))) {
		if ((p - _format == 0) || *(p - 1) != '%') {
			errno = EINVAL;
			MSG_ASSERT_RETURN("vsnprintf_s: illegal %n" << _dstbuf, false, -(EINVAL));
		}
	}

	errno = 0;
	ret = vsnprintf(_dstbuf, (size_t)_dstbuf_size, _format, _args);/*checked safe*/

	_dstbuf[_dstbuf_size - 1] = '\0';

	return ret;
}

int vsprintf_s(char * _dstbuf, size_t _dstbuf_size, const char * _format, va_list _args){
	int ret = -1;
	const char *p;

	if(NULL == _dstbuf || NULL == _format){
		errno = ESNULLP;
		MSG_ASSERT_RETURN("vsprintf_s invalid parameters" << VAR((void *)_dstbuf) << VAR((void *)_format), false, -(ESNULLP));
	}

	if (((p = strnstr(_format, "%n", MAX_LIMIT_STR_SIZE)))) {
		if ((p - _format == 0) || *(p - 1) != '%') {
			errno = EINVAL;
			MSG_ASSERT_RETURN("vsprintf_s: illegal %n" << _dstbuf, false, -(EINVAL));
		}
	}

	errno = 0;
	ret = vsnprintf(_dstbuf, (size_t)_dstbuf_size, _format, _args);/*checked safe*/

	if(ret >= (_dstbuf_size)){
		errno = ESNOSPC;
		MSG_ASSERT_RETURN("vsprintf_s have not more expace for the formatted string" << VAR((void *)_dstbuf) << VAR((void *)_format), false, -(ESNOSPC));
	}

	_dstbuf[_dstbuf_size - 1] = '\0';
	
	return ret;
}
#endif

END_SSB_NAMESPACE
