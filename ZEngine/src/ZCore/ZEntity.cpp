#include <ZCore/ZEntity.hpp>
#include <ZCore/ZEngine.hpp>

Entity Entity::spawn() {
    Entity entity;
    std::unique_ptr<IEntity> temp = std::make_unique<IEntity>();
    entity.m_ptr = temp.get();
    entity.validity = ZEngine::spawnEntity(std::move(temp));
    entity.m_ptr->self = entity;
    return entity;
}
