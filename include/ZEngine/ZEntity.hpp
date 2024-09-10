#ifndef ZENGINE_ZENTITY
#define ZENGINE_ZENTITY

#include <ZEngine/ZCore.hpp>
#include <ZEngine/Rtti.hpp>
#include <ZEngine/ZRef.hpp>
#include <ZEngine/ZComponent.hpp>

#include <memory>
#include <string>
#include <vector>

struct ZEntity : virtual RTTI::Enable {
	friend class ZEngine;

	template <typename T>
	friend ZRef<T> createEntity();
	friend void destroyEntity(ZRef<ZEntity> entity);

	RTTI_DECLARE_TYPEINFO(ZEntity);

private:
	size_t poolIndex;
	bool markedForDelete = false;
	std::string name = "entity";
	ZRef<ZEntity> parent;
	std::vector<std::shared_ptr<ZComponent>> components;
	std::vector<ZRef<ZEntity>> children;

	// transformation related
	ZTransform localTransform;
	ZTransform orignTransform;

	// update loop related
	bool enabled = true;

protected:
	ZRef<ZEntity> self;

	virtual void onCreate() {}; //called after being created by the game engine

	virtual void preAwake() {} //called before awake() of componentts
	virtual void preStart() {} //called after awake() of components and before their start()

	virtual void fixedUpdate() {}
	virtual void update() {}
	virtual void lateUpdate() {}

	virtual void onDestroy() {}; //is called when being destroyed or when the engine is about to shutdown

	virtual void receiveEvent(ZEvent* event) {};

public:
	bool updateEnabled = true;
	bool pauseEnabled = false;
	bool componentUpdateEnabled = true;

	ZEntity() {}

	size_t getPoolIndex() { return poolIndex; }

	bool isMarkedForDelete() {
		return markedForDelete;
	}

	bool isEnabled() {
		return enabled;
	}

	std::string getName() {
		return name;
	}

	void setName(std::string newName) {
		name = newName;
	}

	Vec2 getPosition() {
		return orignTransform.position;
	}

	void setPosition(Vec2 newPosition) {
		Vec2 deltaPosition = newPosition - orignTransform.position;
		orignTransform.position = newPosition;

		moveChildren(deltaPosition);
	}

	void move(Vec2 deltaPosition) {
		orignTransform.position += deltaPosition;
		moveChildren(deltaPosition);
	}

	void moveChildren(Vec2 deltaPosition) {
		for (ZRef<ZEntity> child : children) {
			child->orignTransform.position += deltaPosition;
			child->moveChildren(deltaPosition);
		}
	}

	float getRotation() {
		return orignTransform.rotation;
	}

	void setRotation(float newRotation) {
		orignTransform.rotation = newRotation;
	}

	Vec2 getScale() {
		return orignTransform.scale;
	}

	void setScale(Vec2 newScale) {
		Vec2 deltaScale = newScale - orignTransform.scale;
		orignTransform.scale = newScale;
	}

	void scaleChildren(Vec2 deltaScale) {
		for (ZRef<ZEntity> child : children) {
			child->orignTransform.scale += deltaScale;
			child->scaleChildren(deltaScale);
		}
	}

	ZRef<ZEntity> getParent() {
		return parent;
	}

	void setParent(ZRef<ZEntity> newParent) {
		if (newParent == parent || newParent->markedForDelete || markedForDelete) return;

		if (parent.isValid()) {
			for (size_t i = 0; i < parent->children.size(); i++) {
				ZRef<ZEntity> child = parent->children[i];
				if (child == self) {
					parent->children.erase(parent->children.begin() + i);
				}
			}
		}

		parent.reset();
		if (newParent.isValid()) {
			newParent->children.push_back(self);
			parent = newParent;
		};
	}

	template <typename T>
	ZRef<T> addComponent() {
		static_assert(std::is_base_of<ZComponent, T>::value, "Components must derive from ZComponent");

		std::shared_ptr<T> component = std::make_shared<T>();
		component->owner = self;
		std::weak_ptr<T> weakComponent = component;
		component->self = ZRef<ZComponent>(weakComponent);
		components.push_back(component);

		component->onAttach();

		return ZRef<T>(weakComponent);
	}

	template <typename T>
	ZRef<T> getComponent() {
		static_assert(std::is_base_of<ZComponent, T>::value, "Components must derive from ZComponent");

		for (std::shared_ptr<ZComponent> compoent : components) {
			if (compoent->is<T>()) {
				std::shared_ptr<T> castedComponent = std::reinterpret_pointer_cast<T>(compoent);
				std::weak_ptr<T> weakComponent = castedComponent;
				return weakComponent;
			}
		}

		return ZRef<T>();
	}

	template <typename T>
	void removeComponent() {
		static_assert(std::is_base_of<ZComponent, T>::value, "Components must derive from ZComponent");

		size_t length = components.size();
		for (size_t i = 0; i < length; i++) {
			if (components[i]->is<T>()) {
				components[i]->onDestroy();
				components.erase(components.begin() + i);
				break;
			}
		}
	}

	void removeComponent(ZRef<ZComponent> component) {
		if (!component.isValid() || component->owner.get() != this) return;

		size_t length = components.size();
		ZComponent* realComponent = component.get();
		for (size_t i = 0; i < length; i++) {
			if (components[i].get() == realComponent) {
				realComponent->onDestroy();
				components.erase(components.begin() + i);
				break;
			}
		}
	}

	virtual void broadcast(ZEvent* event) {
		receiveEvent(event);
		for (std::shared_ptr<ZComponent> component : components) {
			component->receiveEvent(event);
		}
	}
};

#define GENERATET_ENTITY_BODY(name, parent) friend class ZEngine; \
template <typename T> \
friend ZRef<T> createEntity(); \
friend void destroyEntity(ZRef<ZEntity> entity); \
RTTI_DECLARE_TYPEINFO(##name, ##parent);

#endif // !ZENGINE_ZENTITY
