#include "mouse_hook.h"
#include <iostream>
#include <sstream>

// 静态成员初始化
MouseHookManager* MouseHookManager::instance_ = nullptr;

MouseHookManager::MouseHookManager()
    : mouseHook_(nullptr)
    , hookInstalled_(false)
    , blockEvents_(false)
    , nextCallbackId_(1) {
}

MouseHookManager::~MouseHookManager() {
    UninstallHook();
}

MouseHookManager& MouseHookManager::GetInstance() {
    static MouseHookManager instance;
    instance_ = &instance;
    return instance;
}

int MouseHookManager::RegisterCallback(MouseEventType eventType, MouseEventCallback callback, void* userData) {
    int id = nextCallbackId_++;

    CallbackInfo info;
    info.id = id;
    info.userData = userData;

    callbacks_[eventType][id] = std::make_pair(callback, info);
    return id;
}

void MouseHookManager::UnregisterCallback(int callbackId) {
    for (auto& eventPair : callbacks_) {
        eventPair.second.erase(callbackId);
    }
}

void MouseHookManager::UpdateCallbackData(int callbackId, void* userData) {
    for (auto& eventPair : callbacks_) {
        auto it = eventPair.second.find(callbackId);
        if (it != eventPair.second.end()) {
            it->second.second.userData = userData;
            break;
        }
    }
}

bool MouseHookManager::InstallHook() {
    if (hookInstalled_) {
        return true;
    }

    // 设置低级鼠标钩子
    mouseHook_ = SetWindowsHookEx(WH_MOUSE_LL, MouseProc, GetModuleHandle(nullptr), 0);
    if (mouseHook_) {
        hookInstalled_ = true;
        std::cout << "鼠标钩子安装成功" << std::endl;
        return true;
    }

    std::cerr << "鼠标钩子安装失败: " << GetLastError() << std::endl;
    return false;
}

bool MouseHookManager::UninstallHook() {
    if (mouseHook_) {
        BOOL result = UnhookWindowsHookEx(mouseHook_);
        if (result) {
            mouseHook_ = nullptr;
            hookInstalled_ = false;
            std::cout << "鼠标钩子卸载成功" << std::endl;
            return true;
        }
    }
    return false;
}

bool MouseHookManager::IsHookInstalled() const {
    return hookInstalled_;
}

void MouseHookManager::SetEventBlocking(bool enable) {
    blockEvents_ = enable;
}

bool MouseHookManager::IsEventBlocking() const {
    return blockEvents_;
}

LRESULT CALLBACK MouseHookManager::MouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (instance_ && nCode >= 0) {
        return instance_->HandleMouseEvent(nCode, wParam, lParam);
    }
    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

LRESULT MouseHookManager::HandleMouseEvent(int nCode, WPARAM wParam, LPARAM lParam) {
    MSG* msg = reinterpret_cast<MSG*>(lParam);

    // 创建鼠标事件
    MouseEvent event = CreateMouseEvent(wParam, msg);

    // 通知所有回调
    bool shouldBlock = NotifyCallbacks(event);

    // 如果设置了阻止事件或者回调要求阻止，则阻止事件传递
    if (blockEvents_ || shouldBlock) {
        return 1;
    }

    return CallNextHookEx(mouseHook_, nCode, wParam, lParam);
}

MouseEventType MouseHookManager::ConvertMessageType(WPARAM wParam, MSG* msg) {
    switch (wParam) {
    case WM_LBUTTONDOWN: return MouseEventType::LeftButtonDown;
    case WM_LBUTTONUP: return MouseEventType::LeftButtonUp;
    case WM_RBUTTONDOWN: return MouseEventType::RightButtonDown;
    case WM_RBUTTONUP: return MouseEventType::RightButtonUp;
    case WM_MBUTTONDOWN: return MouseEventType::MiddleButtonDown;
    case WM_MBUTTONUP: return MouseEventType::MiddleButtonUp;
    case WM_MOUSEMOVE: return MouseEventType::MouseMove;
    case WM_MOUSEWHEEL: return MouseEventType::MouseWheel;
    case WM_MOUSEHWHEEL: return MouseEventType::MouseHWheel;
    default: return MouseEventType::Unknown;
    }
}

MouseEvent MouseHookManager::CreateMouseEvent(WPARAM wParam, MSG* msg) {
    MouseEvent event;
    event.type = ConvertMessageType(wParam, msg);
    event.position = msg->pt;
    event.timestamp = msg->time;
    //event.isInjected = (msg->dwExtraInfo != 0);

    // 处理滚轮事件
    if (wParam == WM_MOUSEWHEEL || wParam == WM_MOUSEHWHEEL) {
        event.wheelDelta = GET_WHEEL_DELTA_WPARAM(msg->wParam);
    }
    else {
        event.wheelDelta = 0;
    }

    return event;
}

bool MouseHookManager::NotifyCallbacks(const MouseEvent& event) {
    bool shouldBlock = false;

    auto it = callbacks_.find(event.type);
    if (it != callbacks_.end()) {
        for (const auto& callbackPair : it->second) {
            const auto& callback = callbackPair.second.first;
            const auto& info = callbackPair.second.second;
            if (callback(event, info.userData)) {
                shouldBlock = true;
            }
        }
    }

    // 同时通知通用回调（监听所有事件类型）
    auto allIt = callbacks_.find(MouseEventType::Unknown);
    if (allIt != callbacks_.end()) {
        for (const auto& callbackPair : allIt->second) {
            const auto& callback = callbackPair.second.first;
            const auto& info = callbackPair.second.second;
            if (callback(event, info.userData)) {
                shouldBlock = true;
            }
        }
    }

    return shouldBlock;
}

void MouseHookManager::SimulateLeftClick(int x, int y) {
    SetCursorPos(x, y);

    INPUT inputs[2] = { 0 };

    // 按下左键
    inputs[0].type = INPUT_MOUSE;
    inputs[0].mi.dx = (x * 65535) / GetSystemMetrics(SM_CXSCREEN);
    inputs[0].mi.dy = (y * 65535) / GetSystemMetrics(SM_CYSCREEN);
    inputs[0].mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTDOWN;
    inputs[0].mi.dwExtraInfo = 1; // 标记为注入事件

    // 释放左键
    inputs[1].type = INPUT_MOUSE;
    inputs[1].mi.dx = inputs[0].mi.dx;
    inputs[1].mi.dy = inputs[0].mi.dy;
    inputs[1].mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTUP;
    inputs[1].mi.dwExtraInfo = 1; // 标记为注入事件

    SendInput(2, inputs, sizeof(INPUT));
}

void MouseHookManager::SimulateRightClick(int x, int y) {
    SetCursorPos(x, y);

    INPUT inputs[2] = { 0 };

    // 按下右键
    inputs[0].type = INPUT_MOUSE;
    inputs[0].mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
    inputs[0].mi.dwExtraInfo = 1;

    // 释放右键
    inputs[1].type = INPUT_MOUSE;
    inputs[1].mi.dwFlags = MOUSEEVENTF_RIGHTUP;
    inputs[1].mi.dwExtraInfo = 1;

    SendInput(2, inputs, sizeof(INPUT));
}

void MouseHookManager::SimulateMouseMove(int x, int y) {
    INPUT input = { 0 };
    input.type = INPUT_MOUSE;
    input.mi.dx = (x * 65535) / GetSystemMetrics(SM_CXSCREEN);
    input.mi.dy = (y * 65535) / GetSystemMetrics(SM_CYSCREEN);
    input.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE;
    input.mi.dwExtraInfo = 1;

    SendInput(1, &input, sizeof(INPUT));
}

void MouseHookManager::SimulateMouseWheel(int delta) {
    INPUT input = { 0 };
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_WHEEL;
    input.mi.mouseData = delta;
    input.mi.dwExtraInfo = 1;

    SendInput(1, &input, sizeof(INPUT));
}