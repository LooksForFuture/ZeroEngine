#ifndef ZENGINE_H
#define ZENGINE_H

#include <ZCore/ZApp.hpp>
#include <ZCore/GCObject.hpp>
#include <ZCore/ZEntity.hpp>

#include <string>
#include <vector>
#include <memory>

struct IEntityPair {
    std::unique_ptr<IEntity> ptr;
    std::shared_ptr<bool> validity;
};

class ZEngine : public ZApp {
    static std::string name;
    static std::vector<std::unique_ptr<GCObject>> garbages;
    static std::vector<IEntityPair> entities;

public:
    ZEngine() = default;
    ZEngine(std::string name);

    static std::shared_ptr<bool> spawnEntity(std::unique_ptr<IEntity> entity);
};

#endif
