#pragma once
#include <Windows.h>
#include <functional>
#include <map>
#include <vector>
#include <atomic>

// 鼠标事件类型枚举
enum class MouseEventType {
    LeftButtonDown,
    LeftButtonUp,
    RightButtonDown,
    RightButtonUp,
    MiddleButtonDown,
    MiddleButtonUp,
    MouseMove,
    MouseWheel,
    MouseHWheel,
    Unknown
};

// 鼠标事件数据结构
struct MouseEvent {
    MouseEventType type;
    POINT position;
    DWORD timestamp;
    int wheelDelta;
    bool isInjected; // 是否由程序注入的事件
};

// 带用户数据的回调信息结构
struct CallbackInfo {
    int id;
    void* userData;
};

// 修改后的鼠标事件回调函数类型
using MouseEventCallback = std::function<bool(const MouseEvent&, void* userData)>;

// 鼠标钩子管理器类
class MouseHookManager {
public:
    static MouseHookManager& GetInstance();

    // 禁止拷贝和移动
    MouseHookManager(const MouseHookManager&) = delete;
    MouseHookManager& operator=(const MouseHookManager&) = delete;

    // 注册事件回调 - 重载版本，支持用户数据
    int RegisterCallback(MouseEventType eventType, MouseEventCallback callback, void* userData = nullptr);

    // 注销事件回调
    void UnregisterCallback(int callbackId);

    // 更新回调的用户数据
    void UpdateCallbackData(int callbackId, void* userData);

    // 安装鼠标钩子
    bool InstallHook();

    // 卸载鼠标钩子
    bool UninstallHook();

    // 检查钩子状态
    bool IsHookInstalled() const;

    // 设置事件阻止
    void SetEventBlocking(bool enable);
    bool IsEventBlocking() const;

    // 模拟鼠标事件
    static void SimulateLeftClick(int x, int y);
    static void SimulateRightClick(int x, int y);
    static void SimulateMouseMove(int x, int y);
    static void SimulateMouseWheel(int delta);

private:
    MouseHookManager();
    ~MouseHookManager();

    // 钩子处理函数
    static LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam);

    // 处理鼠标事件
    LRESULT HandleMouseEvent(int nCode, WPARAM wParam, LPARAM lParam);

    // 转换消息类型
    MouseEventType ConvertMessageType(WPARAM wParam, MSG* msg);

    // 创建鼠标事件对象
    MouseEvent CreateMouseEvent(WPARAM wParam, MSG* msg);

    // 通知所有回调
    bool NotifyCallbacks(const MouseEvent& event);

private:
    static MouseHookManager* instance_;
    HHOOK mouseHook_;
    std::atomic<bool> hookInstalled_;
    std::atomic<bool> blockEvents_;

    // 回调管理 - 修改为存储回调信息
    std::map<MouseEventType, std::map<int, std::pair<MouseEventCallback, CallbackInfo>>> callbacks_;
    std::atomic<int> nextCallbackId_;
};