#include "runtime/engine.h"

#include "runtime/base.h"
#include "runtime/core/meta/reflection/reflection_register.h"


namespace Lain
{
    bool                            g_is_editor_mode{ false };
    std::unordered_set<std::string> g_editor_tick_component_types{};

    void Engine::startEngine(const std::string& config_file_path)
    {

        L_CORE_INFO("engine start");
        Reflection::TypeMetaRegister::metaRegister();
    }

    void Engine::shutdownEngine()
    {
        L_CORE_INFO("engine shutdown");

        Reflection::TypeMetaRegister::metaUnregister();
    }

    void Engine::initialize() {}
    void Engine::clear() {}

    void Engine::run()
    {

        while (true);
    }

    float Engine::calculateDeltaTime()
    {
        float delta_time;
        {
            using namespace std::chrono;

            steady_clock::time_point tick_time_point = steady_clock::now();
            duration<float> time_span = duration_cast<duration<float>>(tick_time_point - m_last_tick_time_point);
            delta_time = time_span.count();

            m_last_tick_time_point = tick_time_point;
        }
        return delta_time;
    }

    bool Engine::tickOneFrame(float delta_time)
    {
        logicalTick(delta_time);
        calculateFPS(delta_time);
        return true;
    }

    void Engine::logicalTick(float delta_time)
    {
    }

    bool Engine::rendererTick(float delta_time)
    {
        return true;
    }

    const float Engine::s_fps_alpha = 1.f / 100;
    void        Engine::calculateFPS(float delta_time)
    {
        m_frame_count++;

        if (m_frame_count == 1)
        {
            m_average_duration = delta_time;
        }
        else
        {
            m_average_duration = m_average_duration * (1 - s_fps_alpha) + delta_time * s_fps_alpha;
        }

        m_fps = static_cast<int>(1.f / m_average_duration);
    }
} // namespace Piccolo
