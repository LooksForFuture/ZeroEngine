#ifndef ENTITY_H
#define ENTITY_H

#include <ZCore/GCObject.hpp>

#include <memory>
#include <string>

class IEntity;

class Entity {
    IEntity* m_ptr;
    std::shared_ptr<bool> validity;

public:
    static Entity spawn();
};

class IEntity : public GCObject {
    friend Entity;
    Entity self;

    std::string name;
public:
    IEntity() = default;
};

#endif
