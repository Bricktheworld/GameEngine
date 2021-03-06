#include <core/systems/mesh_loader_system.h>
#include <core/system_providers/mesh_loader_system_provider.h>
#include <fstream>
#include <helpers/mesh_importer.h>
#include <iostream>
#include <map>
#include <sstream>
#include <string.h>
#include <string>
#include <thread>
#include <ecs/world/world.hpp>
#include <engine.hpp>

namespace Vultr::MeshLoaderSystem
{
    static void import(const ModelSource &source)
    {
        auto &provider = get_provider();
        Mesh *mesh = MeshImporter::import_mesh(source);
        if (mesh == nullptr)
            return;
        add_mesh(source, mesh);
    }
    static void check_and_load_mesh(Entity entity)
    {
        auto &component = entity_get_component<StaticMeshComponent>(entity);
        auto source = component.source;

        if (get_mesh(source) == nullptr)
        {
            MeshLoaderSystem::import(source);
        }
    }
    void register_system()
    {
        Signature signature;
        signature.set(get_component_type<StaticMeshComponent>(), true);
        register_global_system<Component>(signature, on_create_entity, nullptr);
    }

    void update()
    {
        auto &provider = get_provider();
        for (Entity entity : provider.entities)
        {
            check_and_load_mesh(entity);
        }
    }

    void on_create_entity(Entity entity)
    {
        check_and_load_mesh(entity);
    }

} // namespace Vultr::MeshLoaderSystem
