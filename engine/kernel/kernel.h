#pragma once

#include <memory>
#include <string>
#include <vector>
#include <map>

#define SEEK_NAMESPACE_BEGIN     namespace seek_engine {
#define SEEK_NAMESPACE_END       }
#define USING_NAMESPACE_SEEK     using namespace seek_engine;

#define CLASS_PTR(class_name)        using class_name##Ptr       = std::shared_ptr<class_name>;
#define CLASS_PTR_SHARED(class_name) using class_name##PtrShared = std::shared_ptr<class_name>;
#define CLASS_PTR_UNIQUE(class_name) using class_name##PtrUnique = std::unique_ptr<class_name>;
#define CLASS_DECLARE(class_name) class class_name; CLASS_PTR(class_name); CLASS_PTR_SHARED(class_name); CLASS_PTR_UNIQUE(class_name);


SEEK_NAMESPACE_BEGIN


typedef uint32_t SResult;
#define S_Success   0
enum SEEK_ERR_CODE // max error code is 255
{
    SEEK_ERR_UNKNOWN                 = 1,
    SEEK_ERR_INVALID_ARG             = 2,
    SEEK_ERR_INVALID_INIT            = 3,
    SEEK_ERR_INVALID_INVOKE_FLOW     = 4,
    SEEK_ERR_INVALID_SHADER          = 5,
    SEEK_ERR_INVALID_MODEL_FILE      = 6,
    SEEK_ERR_INVALID_DATA            = 7,
    SEEK_ERR_NOT_SUPPORT             = 8,
    SEEK_ERR_NOT_IMPLEMENTED         = 9,
    SEEK_ERR_NO_MEM                  = 10,
    SEEK_ERR_NO_DATA                 = 11,
    SEEK_ERR_FILE_NOT_FOUND          = 12,
    SEEK_ERR_SYSTEM_ERROR            = 13,

    SEEK_ERR_CODE_NUM,
};

#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
    TypeName           (const TypeName&) = delete; \
    TypeName& operator=(const TypeName&) = delete;

// Kernel
CLASS_DECLARE(Context);

// Scene Manager
CLASS_DECLARE(SceneManager);

// Effect
CLASS_DECLARE(RendererCommandManager);

// Component
CLASS_DECLARE(Entity);
CLASS_DECLARE(Component);
CLASS_DECLARE(SceneComponent);
CLASS_DECLARE(CameraComponent);
CLASS_DECLARE(LightComponent);
CLASS_DECLARE(DirectionalLightComponent);
CLASS_DECLARE(SpotLightComponent);
CLASS_DECLARE(PointLightComponent);
CLASS_DECLARE(MeshComponent);
CLASS_DECLARE(SkeletalMeshComponent);
CLASS_DECLARE(SkyBoxComponent);
CLASS_DECLARE(Sprite2DComponent);
CLASS_DECLARE(TriangleMeshComponent);
CLASS_DECLARE(ImageComponent);
CLASS_DECLARE(SpringSkeletonComponent);
CLASS_DECLARE(ParticleComponent);

CLASS_DECLARE(KeyFrame);
CLASS_DECLARE(TransformKeyFrame);
CLASS_DECLARE(MorphTargetKeyFrame);
CLASS_DECLARE(AnimationTrack);
CLASS_DECLARE(TransformAnimationTrack);
CLASS_DECLARE(MorphTargetAnimationTrack);
CLASS_DECLARE(AnimationComponent);

// Resource
CLASS_DECLARE(ResourceManager)

// RHI
CLASS_DECLARE(Material);
CLASS_DECLARE(RHIContext);
CLASS_DECLARE(RHIFrameBuffer);
CLASS_DECLARE(RHIMesh);
CLASS_DECLARE(RHIRenderBuffer);
CLASS_DECLARE(RHITexture);
CLASS_DECLARE(RHISampler);
CLASS_DECLARE(RHIShader);
CLASS_DECLARE(RHIProgram);
CLASS_DECLARE(RHIRenderState);
CLASS_DECLARE(RHIRenderView);
CLASS_DECLARE(RHITimerQuery);

// Thread
CLASS_DECLARE(Thread);
CLASS_DECLARE(ThreadManager);

// utils
CLASS_DECLARE(Buffer);
CLASS_DECLARE(BitmapBuffer);
CLASS_DECLARE(Timer);


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
#define MakeSharedPtrMacro(CLASS, ...) std::make_shared<CLASS>(__VA_ARGS__)
#define MakeUniquePtrMacro(CLASS, ...) std::unique_ptr<CLASS>(new CLASS(__VA_ARGS__), std::default_delete<CLASS>())


SEEK_NAMESPACE_END