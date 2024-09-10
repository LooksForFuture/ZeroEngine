#ifndef ZENGINE
#define ZENGINE

#include <ZEngine/Rtti.hpp>
#include <ZEngine/ZRef.hpp>

#include <ZEngine/ZCore.hpp>

#include <ZEngine/ZComponent.hpp>
#include <ZEngine/ZEntity.hpp>

#include <ZEngine/ZPhysicsPipeline.hpp>

#include <ZEngine/ZRenderer.hpp>
#include <ZEngine/ZRenderPipeline.hpp>
#include <ZEngine/ZCamera.hpp>

#include <ZEngine/ZAsset.hpp>

#include <string>
#include <vector>
#include <memory>

namespace ZTime {
	double scale = 1.0;
	double unscaledDt = 0.0;
	double dt = 0.0;
	double fixedDt = 1.0 / 120.0;
}

struct ZEngineData {
	bool paused = true;
	std::string appName = "";
	std::vector<std::shared_ptr<ZEntity>> entities;
	std::vector<size_t> killPositions; //indices of entities thatt must be destroyed
	std::vector<size_t> freePositions; //indices of "entities" vector that are free

	std::vector<ZRef<ZEntity>> pendingAwake;

	std::shared_ptr<ZWindow> window;
	std::shared_ptr<ZRenderer> renderer;
	std::shared_ptr<ZRenderPipeline> renderPipeline;
	std::shared_ptr<ZPhysicsPipeline> physicsPipeline;

	std::vector<std::unique_ptr<ZAssetContainer>> assetContainers;

	~ZEngineData() {
		for (const std::unique_ptr<ZAssetContainer>& container : assetContainers) {
			container->clear();
		}

		renderer.reset();
		window.reset();
	}
};

//miliseconds resolution
float getTime() {
	return static_cast<float>(SDL_GetTicks()) / 1000.0f;
}

class ZEngine {
	template <typename T>
	friend ZRef<T> createEntity();
	friend void destroyEntity(ZRef<ZEntity> entity);

	friend bool isGamePaused();
	friend void setGamePaused(bool state);

	friend ZRef<ZRenderer> getRenderer();

	friend ZRef<ZRenderPipeline> getRenderPipeline();
	template <typename T>
	friend void setRenderPipeline();

	friend ZRef<ZPhysicsPipeline> getPhysicsPipeline();
	template <typename T>
	friend void setPhysicsPipeline();

	friend class ZResources;

	static bool constructed;
	static std::unique_ptr<ZEngineData> data;

public:
	ZEngine() {}
	ZEngine(std::string appName, float ratio) {
		if (!constructed) {
			constructed = true;
			data = std::make_unique<ZEngineData>();
			data->appName = appName;

			data->window = std::make_shared<ZWindow>(ratio);

			data->renderer = std::make_shared<ZRenderer>(ZRef<ZWindow>(std::weak_ptr<ZWindow>(data->window)));
			data->physicsPipeline = std::make_shared<ZPhysicsPipeline>();
		}
	}

	std::string getName() {
		return data->appName;
	}

	void setName(std::string appName) {
		if (!constructed) {
			constructed = true;
			data = std::make_unique<ZEngineData>();
		}

		data->appName = appName;
	}

	void addContainer(RTTI::TypeId typeId) {
		data->assetContainers.push_back(std::make_unique<ZAssetContainer>(typeId));
	}

	void cleanup() {
		for (size_t i = 0; i < data->killPositions.size(); i++) {
			size_t index = data->killPositions[i];
			data->entities[index] = nullptr;
			data->freePositions.push_back(index);
		}
		data->killPositions.clear();
	}

	void run() {
		data->paused = false;

		float previousTime = getTime();
		double previous = previousTime;
		double lag = 0.0f;

		while (!data->window->isShouldClose()) {
			double currentTime = getTime();
			ZTime::unscaledDt = (currentTime - previous);
			ZTime::dt = ZTime::scale * ZTime::unscaledDt;

			if (!data->paused) {
				lag += ZTime::dt;
				// fixed update
				while (lag >= ZTime::fixedDt) {
					for (std::shared_ptr<ZEntity> entity : data->entities) {
						if (entity->updateEnabled) entity->fixedUpdate();
						for (std::shared_ptr<ZComponent> component : entity->components) {
							if (component->m_enabled && component->updateEnabled) component->fixedUpdate();
						}
					}
					
					//gameEngine.stepPhysics(Time::fixedDt);
					data->physicsPipeline->step(ZTime::fixedDt);
					lag -= ZTime::fixedDt;
				}
			}

			// awake
			if (!data->paused) {
				for (size_t i = 0; i < data->pendingAwake.size(); i++) {
					ZRef<ZEntity> entity = data->pendingAwake[i];
					if (!entity->enabled) continue;

					entity->preAwake();
					for (std::shared_ptr<ZComponent> component : entity->components) {
						component->awake();
					}

					entity->preStart();
					for (std::shared_ptr<ZComponent> component : entity->components) {
						component->start();
					}

					data->pendingAwake.erase(data->pendingAwake.begin() + i);
					i--;
				}
			}


			// update loop
			for (std::shared_ptr<ZEntity> entity : data->entities) {
				if (entity->markedForDelete || !entity->enabled) continue;

				if (data->paused) {
					if (entity->pauseEnabled) {
						if (entity->updateEnabled) entity->update();
						for (std::shared_ptr<ZComponent> component : entity->components) {
							if (component->m_enabled && component->pauseEnabled && component->updateEnabled) component->update();
						}

						if (entity->updateEnabled) entity->lateUpdate();
						for (std::shared_ptr<ZComponent> component : entity->components) {
							if (component->m_enabled && component->pauseEnabled && component->updateEnabled) component->lateUpdate();
						}
					}
				}

				else {
					if (entity->updateEnabled) entity->update();
					for (std::shared_ptr<ZComponent> component : entity->components) {
						if (component->m_enabled && component->updateEnabled) component->update();
					}

					if (entity->updateEnabled) entity->lateUpdate();
					for (std::shared_ptr<ZComponent> component : entity->components) {
						if (component->m_enabled && component->updateEnabled) component->lateUpdate();
					}
				}
			}

			data->renderPipeline->render();

			data->renderer->present();
			data->window->pollEvents();

			previous = currentTime;
		} // end of update loop

		for (std::shared_ptr<ZEntity> entity : data->entities) {
			if (entity->markedForDelete) return;
			
			entity->onDestroy();
			for (std::shared_ptr<ZComponent> component : entity->components) {
				component->onDestroy();
			}
		}
	}
};

template <typename T>
RTTI::TypeId getTypeId() {
	return RTTI::TypeInfo<T>::Id();
}

bool ZEngine::constructed = false;
std::unique_ptr<ZEngineData> ZEngine::data = NULL;

template <typename T>
ZRef<T> createEntity() {
	std::shared_ptr<T> entity = std::make_shared<T>();

	//if there is no free position in the list of entities, append to the end
	if (ZEngine::data->freePositions.size() == 0) {
		entity->poolIndex = ZEngine::data->freePositions.size();
		ZEngine::data->entities.push_back(entity);
	}

	//and if there is, put the new entity in that position
	else {
		size_t index = ZEngine::data->freePositions[0];
		entity->poolIndex = index;
		ZEngine::data->entities[index] = entity;
		ZEngine::data->freePositions.erase(ZEngine::data->freePositions.begin());
	}

	std::weak_ptr<T> weakEntity = entity;
	ZRef<T> ref = weakEntity;
	entity->self = std::weak_ptr(std::reinterpret_pointer_cast<ZEntity>(entity));

	ZEngine::data->pendingAwake.push_back(ref);

	entity->onCreate();

	return  ref;
}

void destroyEntity(ZRef<ZEntity> entity) {
	if (entity.isExpired() || entity->markedForDelete) return;

	ZEngine::data->killPositions.push_back(entity->poolIndex);
	entity->markedForDelete = true;
	entity->onDestroy();

	//onDestroy() components
	for (std::shared_ptr<ZComponent> component : entity->components) {
		component->onDestroy();
	}

	//destroy children
	for (ZRef<ZEntity> child : entity->children) {
		destroyEntity(child);
	}

	//detach from parent
	ZRef<ZEntity> parent = entity->parent;
	if (parent.isValid()) {
		for (size_t i = 0; i < parent->children.size(); i++) {
			ZRef<ZEntity> child = parent->children[i];
			if (child == entity) {
				parent->children.erase(parent->children.begin() + i);
			}
		}
	}
}

#define spawnEntity(name, type) ZRef<##type> ##name = createEntity<##type>()

bool isGamePaused() {
	return ZEngine::data->paused;
}

void setGamePaused(bool state) {
	ZEngine::data->paused = state;
}

ZRef<ZRenderer> getRenderer() {
	return ZRef<ZRenderer>(std::weak_ptr<ZRenderer>(ZEngine::data->renderer));
}

ZRef<ZRenderPipeline> getRenderPipeline() {
	return ZRef<ZRenderPipeline>(std::weak_ptr<ZRenderPipeline>(ZEngine::data->renderPipeline));
}

template <typename T>
void setRenderPipeline() {
	ZEngine::data->renderPipeline = std::make_shared<T>(ZRef(std::weak_ptr(ZEngine::data->window)), ZRef(std::weak_ptr(ZEngine::data->renderer)));
}

ZRef<ZPhysicsPipeline> getPhysicsPipeline() {
	return ZRef<ZPhysicsPipeline>(std::weak_ptr<ZPhysicsPipeline>(ZEngine::data->physicsPipeline));
}

template <typename T>
void setPhysicsPipeline() {
	ZEngine::data->physicsPipeline = std::make_shared<T>();
}

#endif // !ZENGINE