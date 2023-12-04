#include "pch.h"
#include "ScopedTimer.h"
#include "Core/Log.h"

namespace Raito
{

    ScopedTimer::ScopedTimer(const char* name)
        : m_Name(name)
    {
        m_Start = std::chrono::high_resolution_clock::now();
    }

    ScopedTimer::~ScopedTimer()
    {
        m_End = std::chrono::high_resolution_clock::now();
        m_Duration = m_End - m_Start;

        float dt = m_Duration.count() * 1000.f;
        LOG(m_Name.c_str(), "Timer took {}", dt);
    }
}