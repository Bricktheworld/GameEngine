#pragma once
// In charge of distributing entity IDs and keeping record of which IDs are in
// use and which are not
//
// The queue will contain every valid entity ID up to the MAX_ENTITIES
// When an entity is created, it will use the ID at the front of the queue as
// its own When an entity is destroyed, it will put its own id at the back of
// the queue

#include "../component/component.hpp"
#include "entity.hpp"
#include <array>
#include <assert.h>
#include <queue>

namespace Vultr::EntityManager
{

struct EntityManager
{
    // Queue of unused entity IDs
    std::queue<Entity> available_entities;

    // Array of signatures where the index corresponds to the entity ID
    std::array<Signature, MAX_ENTITIES> signatures;

    // Entity IDs that are actually being used
    std::set<Entity> living_entites;

    // Total living entities
    u32 living_entity_count;
};

// Constructor
EntityManager InitEntityManager();

// Manages the movement of entity IDs in the queue and their corresponding signatures
Entity CreateEntity(EntityManager &manager);
void DestroyEntity(EntityManager &manager, Entity entity);
void SetSignature(EntityManager &manager, Entity entity, Signature signature);

// Getters
Signature GetSignature(EntityManager &manager, Entity entity);
std::set<Entity> GetEntities(EntityManager &manager, Signature signature);

} // namespace Vultr::EntityManager
