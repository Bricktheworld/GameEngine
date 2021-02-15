#pragma once
#include <core/system_providers/mesh_loader_system_provider.h>
#include <glm/glm.hpp>
#include <rendering/models/mesh.h>
#include <string>
#include <vector>

namespace Brick3D
{
struct StaticMeshComponent
{
    StaticMeshComponent()
    {
    }

    static StaticMeshComponent &Create(std::string p_path)
    {
        StaticMeshComponent *component = new StaticMeshComponent();
        component->m_path = p_path;
        return *component;
    }

    std::string m_path;

    Mesh *GetMesh()
    {
        // We cannot find a mesh for a path that has not been intialized
        if (m_path.empty())
            return nullptr;
        return MeshLoaderSystemProvider::GetMesh(m_path);
    }
};
} // namespace Brick3D