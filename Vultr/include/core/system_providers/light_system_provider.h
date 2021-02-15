#pragma once
#include <ecs/system/system_provider.hpp>

namespace Brick3D
{
class LightSystemProvider : public SystemProvider
{
  public:
    // Singleton pattern for all providers
    static LightSystemProvider &Get()
    {
        static LightSystemProvider instance;
        return instance;
    }
    Entity light = Entity(-1);
};
} // namespace Brick3D