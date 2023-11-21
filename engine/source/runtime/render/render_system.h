#pragma once
#ifndef __RENDER_SYSTEM_H__
#define __RENDER_SYSTEM_H__
#include "base.h"
namespace lain{
    class RenderSystem {
    public:
        RenderSystem() {
            p_singleton = this;
        };
        ~RenderSystem();
        L_INLINE static RenderSystem* GetSingleton() {
            return p_singleton;
        }
        L_INLINE bool IsInLoop() {
            return m_render_loop_enabled;
        }
    private:
        static RenderSystem* p_singleton;
        bool m_render_loop_enabled = true;
    };
}
#endif // !__RENDER_SYSTEM_H__
