#pragma once
#pragma once

#include <memory>

namespace lain
{
    class WindowSystem;
    class RenderingSystem;

    struct WindowUIInitInfo
    {
        std::shared_ptr<WindowSystem> window_system;
        std::shared_ptr<RenderingSystem> render_system;
    };

    class WindowUI
    {
    public:
        virtual void initialize(WindowUIInitInfo init_info) = 0;
        virtual void preRender() = 0;
    };
} // namespace lain
