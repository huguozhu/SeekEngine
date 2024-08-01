#pragma once

#include <memory>
#include <string>

#define SEEK_NAMESPACE_BEGIN     namespace seek_engine {
#define SEEK_NAMESPACE_END       }
#define USING_NAMESPACE_SEEK     using namespace seek_engine;

#define CLASS_PTR(class_name)        using class_name##Ptr       = std::shared_ptr<class_name>;
#define CLASS_PTR_SHARED(class_name) using class_name##PtrShared = std::shared_ptr<class_name>;
#define CLASS_PTR_UNIQUE(class_name) using class_name##PtrUnique = std::unique_ptr<class_name>;
#define CLASS_DECLARE(class_name) class class_name; CLASS_PTR(class_name); CLASS_PTR_SHARED(class_name); CLASS_PTR_UNIQUE(class_name);

typedef uint32_t SResult;

SEEK_NAMESPACE_BEGIN


template <typename T, typename... Args>
inline std::shared_ptr<T> MakeSharedPtr(Args&&... args)
{
    return std::make_shared<T>(std::forward<Args>(args)...);
}

template <typename T, typename... Args>
inline std::unique_ptr<T> MakeUniquePtr(Args&& ... args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...), std::default_delete<T>());
}




SEEK_NAMESPACE_END