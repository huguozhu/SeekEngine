#include "thread/Mutex.h"

#if defined(SEEK_PLATFORM_WINDOWS)
#include <windows.h>
#endif

SEEK_NAMESPACE_BEGIN

#if defined(SEEK_PLATFORM_WINDOWS)
typedef CRITICAL_SECTION pthread_mutex_t;
typedef uint32_t pthread_mutexattr_t;
inline int pthread_mutex_lock(pthread_mutex_t* _mutex)
{
	EnterCriticalSection(_mutex);
	return 0;
}
inline int pthread_mutex_unlock(pthread_mutex_t* _mutex)
{
	LeaveCriticalSection(_mutex);
	return 0;
}
inline int pthread_mutex_init(pthread_mutex_t* _mutex, pthread_mutexattr_t* /*_attr*/)
{
#if defined(SEEK_PLATFORM_WINDOWS)
	InitializeCriticalSectionEx(_mutex, 4000, 0);
#else
	InitializeCriticalSection(_mutex);
#endif
	return 0;
}

inline int pthread_mutex_destroy(pthread_mutex_t* _mutex)
{
	DeleteCriticalSection(_mutex);
	return 0;
}
#endif

Mutex::Mutex()
{
    pthread_mutexattr_t attr;
    
#if defined(SEEK_PLATFORM_WINDOWS)
#else
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
#endif
    pthread_mutex_t* pMutex = (pthread_mutex_t*)m_Data;
	pthread_mutex_init(pMutex, &attr);
}
Mutex::~Mutex()
{
	pthread_mutex_t* pMetex = (pthread_mutex_t*)m_Data;
	pthread_mutex_destroy(pMetex);
}
void Mutex::Lock()
{
	pthread_mutex_t* pMutex = (pthread_mutex_t*)m_Data;
	pthread_mutex_lock(pMutex);
}
void Mutex::Unlock()
{
	pthread_mutex_t* pMutex = (pthread_mutex_t*)m_Data;
	pthread_mutex_unlock(pMutex);
}



MutexScope::MutexScope(Mutex& m)
	:m_Mutex(m)
{
	m_Mutex.Lock();
}
MutexScope::~MutexScope()
{
	m_Mutex.Unlock();
}

SEEK_NAMESPACE_END