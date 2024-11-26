#include <ZCore/ZEngine.hpp>

std::string ZEngine::name = "";
std::vector<std::unique_ptr<GCObject>> ZEngine::garbages = {};
std::vector<IEntityPair> ZEngine::entities = {};

ZEngine::ZEngine(std::string name) {
    if (ZApp::constructed) return;
    ZApp::constructed = true;

    ZEngine::name = name;
    ZEngine::garbages = std::vector<std::unique_ptr<GCObject>>();
    ZEngine::entities = std::vector<IEntityPair>();
}

std::shared_ptr<bool>ZEngine::spawnEntity(std::unique_ptr<IEntity> entity) {
    std::shared_ptr<bool> validity = std::make_shared<bool>(true);
    ZEngine::entities.push_back(IEntityPair());
    IEntityPair* entityPair = &ZEngine::entities.back();
    entityPair->ptr = std::move(entity);
    entityPair->validity = validity;
    
    return validity;
}
