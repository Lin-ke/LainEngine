#pragma once
#ifndef __RENDERING_SYSTEM_H__
#define __RENDERING_SYSTEM_H__
#include "base.h"
namespace lain{
    class RenderingSystem {
    public:
        RenderingSystem() {
            p_singleton = this;
        };
        ~RenderingSystem();
        L_INLINE static RenderingSystem* GetSingleton() {
            return p_singleton;
        }
        /*****************/
         /**** TEXTURE ****/
         /*****************/

        /*
        Mesh
        */


        /*
        Context
        */
    private:
        static RenderingSystem* p_singleton;
    };
}
#endif // !__RENDER_SYSTEM_H__
