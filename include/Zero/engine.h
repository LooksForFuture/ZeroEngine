#include <vector>
#include <string>
#include <iostream>
#include <unordered_map>
#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define Mat4 glm::mat4
#define Vec2 glm::vec2
#define Vec3 glm::vec3
#define Vec4 glm::vec4
#define IVec2 glm::ivec2

#include <Zero/Mesh.h>
#include <Zero/Shader.h>
#include <Zero/Texture.h>

#define STB_IMAGE_IMPLEMENTATION
#include <Zero/stb_image.h>

#include <Zero/Rtti.h>

class Engine;
class Entity;
class Transform;

struct Event : virtual RTTI::Enable {
    RTTI_DECLARE_TYPEINFO(Event);
};

class Component {
protected:
    friend Engine;
    friend Entity;

    Entity* owner;
    Engine* engine;
    bool enabled = true;
    Transform* transform;

public:
    Component() {}

    bool isEnabled() { return enabled; }
    void setEnabled(bool state) {
        if (state != enabled) {
            enabled = state;
            if (state) { onEnable(); }
            else { onDisable(); }
        }
    }

    Entity* getOwner() {
        return owner;
    }

    virtual void onEnable() {};
    virtual void onDisable() {};
    virtual void onDestroy() {};

    virtual void configure() {};
    virtual void fixedUpdate() {};
    virtual void update() {};
    virtual void lateUpdate() {};

    virtual void receiveEvent(Event* event) {};
};

class Entity {
    friend Engine;

    bool pendingDestroy = false;
    Engine* engine;
    std::vector<Component*> components;
    bool enabled = true;

public:
    std::string name = "entity";

    Transform* transform = nullptr;
    Entity() {}

    bool isPendingDestroy() {
        return pendingDestroy;
    }

    bool isEnabled() { return enabled; }
    void setEnabled(bool state) {
        if (state != enabled) {
            enabled = state;
            for (Component* component : components) {
                component->setEnabled(state);
            }
        }
    }

    template <typename T, typename... Args>
    T* addComponent(Args&&... args) {
        T* newComponent = new T();
        newComponent->owner = this;
        newComponent->engine = engine;
        newComponent->transform = transform;
        newComponent->configure(args...);
        components.push_back(newComponent);
        return newComponent;
    }

    template <typename T>
    bool hasComponent() {
        for (Component* component : components) {
            if (dynamic_cast<T*>(component)) {
                return true;
            }
        }

        return false;
    }

    template <typename T>
    T* getComponent() {
        for (Component* component : components) {
            if (dynamic_cast<T*>(component)) {
                return dynamic_cast<T*>(component);
            }
        }

        return nullptr;
    }

    template<typename T>
    bool removeComponent() {
        size_t index = 0;
        for (Component* component : components) {
            if (dynamic_cast<T*>(component)) {
                component->onDestroy();
                delete component;
                components.erase(components.begin() + index);
                return true;
            }

            index++;
        }

        return false;
    }

    virtual void broadcast(Event* event) {
        for (Component* component : components) {
            component->receiveEvent(event);
        }
    }

    ~Entity() {
        for (Component* component : components) {
            component->onDestroy();
            delete component;
        }
    }
};

class Transform : public Component {
    bool isDirty = false;
    Mat4 model = Mat4(1.0f);
    Vec3 position = Vec3(0.0f), rotation = Vec3(0.0f), scale = Vec3(1.0f);
public:
    Mat4 getModel() {
        if (isDirty) {
            model = Mat4(1.0f);
            model = glm::translate(model, position);

            model = glm::rotate(model, -glm::radians(rotation.x), Vec3(1.0f, 0.0f, 0.0f));
            model = glm::rotate(model, -glm::radians(rotation.y), Vec3(0.0f, 1.0f, 0.0f));
            model = glm::rotate(model, -glm::radians(rotation.z), Vec3(0.0f, 0.0f, 1.0f));

            model = glm::scale(model, scale);

            isDirty = false;
        }

        return model;
    }

    Vec3 getPosition() {
        return position;
    }
    void setPosition(Vec3 newPosition) {
        position = newPosition;
        isDirty = true;
    }

    void translate(Vec3 translation) {
        getModel(); // be sure the model matrix is not modified
        model = glm::translate(model, translation);
        position.x = model[3][0];
        position.y = model[3][1];
        position.z = model[3][2];
    }

    Vec3 getRotation() {
        return rotation;
    }
    void setRotation(Vec3 newRotation) {
        rotation = newRotation;
        isDirty = true;
    }

    Vec3 getScale() {
        return scale;
    }
    void setScale(Vec3 newScale) {
        scale = newScale;
        isDirty = true;
    }

    Vec3 getForward() {
        Mat4 inverted = glm::inverse(getModel());
        return normalize(Vec3(inverted[2]));
    }

    Vec3 getRight() {
        return glm::normalize(glm::cross(getForward(), Vec3(0.0f, 1.0f, 0.0f)));
    }

    Vec3 getUp() {
        return glm::normalize(glm::cross(getRight(), getForward()));
    }
};

class RenderPipeline {
protected:
    friend Engine;
    friend Camera;
    Engine* engine;
    std::vector<Camera*> cameras;

    CameraProjection defaultProjection = CameraProjection::perspective;

    virtual void configure() {}

    virtual void addCamera(Camera* camera) {
        if (camera->projection == CameraProjection::systemDefault) camera->projection = defaultProjection;

        cameras.emplace_back(camera);
    }

    virtual void removeCamera(Camera* camera) {
        size_t i = 0;
        for (Camera* cam : cameras) {
            if (cam == camera) {
                cameras.erase(cameras.begin() + i);
                break;
            }

            i++;
        }
    }

    virtual void render() {};

    virtual void destruct() {};
};

class PhysicsPipeline {
protected:
    friend Engine;
    Engine* engine;

    virtual void configure() {}
    virtual void step(float timeStep) {}
    virtual void destruct() {}
};

class Engine {
    PhysicsPipeline* physicsPipeline = nullptr;
    RenderPipeline* renderPipeline = nullptr;

public:
    std::vector<Entity*> entities;
    std::unordered_map<std::string, std::shared_ptr<Texture2D>> textures;
    std::vector<std::string> sortingLayers;

    void init() {
        unsigned char data[3] = { 255, 255, 255 };
        textures["blank"] = std::shared_ptr<Texture2D>(new Texture2D(1, 1, 3, data));
    }

    inline void addSortingLayer(std::string name) {
        sortingLayers.emplace_back(name);
    }

    // utility function for loading a 2D texture from file
    std::shared_ptr<Texture2D> loadTexture(const char* path, bool gamma = true, bool mipmap = true,
        int wrapS = GL_REPEAT, int wrapT = GL_REPEAT, int minFilter = GL_LINEAR_MIPMAP_LINEAR, int magFilter = GL_NEAREST) {
        int width, height, nrChannels;
        unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);

        for (auto pair : textures) {
            if (pair.first == path) {
                return pair.second;
            }
        }

        std::shared_ptr<Texture2D> texture;

        if (data) {
            texture = std::make_shared<Texture2D>(width, height, nrChannels, data, gamma, mipmap, wrapS, wrapT, minFilter, magFilter);
        }

        else {
            texture = std::make_shared<Texture2D>();
            std::cout << "Texture failed to load at path: " << path << std::endl;
        }

        textures[path] = texture;

        stbi_image_free(data);
        return texture;
    }

    PhysicsPipeline* getPhysicsPipeline() {
        return physicsPipeline;
    }

    RenderPipeline* getRenderPipeline() {
        return renderPipeline;
    }

    void setPhysicsPipeline(PhysicsPipeline* pipeline) {
        if (physicsPipeline != nullptr) {
            delete renderPipeline;
        }
        physicsPipeline = pipeline;
        pipeline->engine = this;
        pipeline->configure();
    }

    void setRenderPipeline(RenderPipeline* pipeline) {
        if (renderPipeline != nullptr) {
            delete renderPipeline;
        }
        renderPipeline = pipeline;
        pipeline->engine = this;
        pipeline->configure();
    }

    Entity* createEntity() {
        Entity* entity = new Entity();
        entity->engine = this;
        entity->transform = entity->addComponent<Transform>();

        entities.emplace_back(entity);
        return entity;
    }

    void destroyEntity(Entity* entity) {
        size_t i = 0;
        for (Entity* ent : entities) {
            if (ent == entity) {
                ent->pendingDestroy = true;
            }

            i++;
        }
    }

    void instantDestroy(Entity* entity) {
        size_t i = 0;
        for (Entity* ent : entities) {
            if (ent == entity) {
                delete entity;
                entities.erase(entities.begin() + i);
            }

            i++;
        }
    }

    void fixedUpdate() {
        for (Entity* entity : entities) {
            if (entity->enabled) {
                for (Component* component : entity->components) {
                    if (component->enabled) {
                        component->fixedUpdate();
                    }
                }
            }
        }
    }

    void update() {
        for (Entity* entity : entities) {
            if (entity->enabled) {
                for (Component* component : entity->components) {
                    if (component->enabled) {
                        component->update();
                    }
                }
            }
        }
    }

    void lateUpdate() {
        for (Entity* entity : entities) {
            if (entity->enabled) {
                for (Component* component : entity->components) {
                    if (component->enabled) {
                        component->lateUpdate();
                    }
                }
            }
        }
    }

    void stepPhysics(double fixedDt) {
        if (physicsPipeline != nullptr) physicsPipeline->step(fixedDt);
    }

    void render() {
        if (renderPipeline != nullptr) renderPipeline->render();
    }

    void clear() {
        size_t i = 0;
        for (Entity* ent : entities) {
            if (ent->pendingDestroy) {
                delete ent;
                entities.erase(entities.begin() + i);
            }

            i++;
        }
    }

    ~Engine() {
        for (Entity* entity : entities) {
            delete entity;
        }

        if (physicsPipeline != nullptr) {
            physicsPipeline->destruct();
            delete physicsPipeline;
        }

        if (renderPipeline != nullptr) {
            renderPipeline->destruct();
            delete renderPipeline;
        }
    }
};

void Camera::configure(Vec4 ambientLight) {
    this->ambientLight = ambientLight;

    RenderPipeline* rp = engine->getRenderPipeline();
    if (rp != nullptr) {
        rp->addCamera(this);
    }
}

void Camera::onDestroy() {
    RenderPipeline* rp = engine->getRenderPipeline();
    if (rp != nullptr) {
        rp->removeCamera(this);
    }
}
