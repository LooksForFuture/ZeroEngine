#ifndef ZENGINE_ZORTHORP
#define ZENGINE_ZORTHORP

#include <ZEngine/ZRef.hpp>
#include <ZEngine/ZRenderPipeline.hpp>
#include <ZEngine/ZCamera.hpp>
#include <ZEngine/ZAsset.hpp>
#include <ZEngine/ZTexture.hpp>
#include <ZEngine/ZComponent.hpp>

#include <vector>


class ZOrthoRenderer : public ZComponent {
	friend class ZOrthoRp;
	GENERATET_COMPONENT_BODY(ZOrthoRenderer, ZComponent);

private:
	int16_t renderIndex = -1;
	std::string sortingLayer = "default";
	int sortingLayerIndex = 0;
	int orderInLayer = 0;

protected:
	virtual void onAttach();
	virtual void onDestroy();

	virtual void render(ZRef<ZCamera> camera, const Vec4& viewport, float unit) {}

public:
	int getSortingLayerIndex() { return sortingLayerIndex; }
	void setSortingLayer(std::string newLayer);
};

class ZOrthoRp : public ZRenderPipeline {
	std::vector<ZRef<ZOrthoRenderer>> renderers;
	std::vector<std::string> sortingLayers;

public:
	ZOrthoRp(ZRef<ZWindow> window, ZRef<ZRenderer> renderer) {
		this->window = window;
		this->renderer = renderer;

		addSortingLayer("default");
	}

	void addSortingLayer(std::string newLayer) {
		for (std::string layer : sortingLayers) {
			if (layer == newLayer) return;
		}

		sortingLayers.push_back(newLayer);
	}

	int getLayerIndex(std::string layer) {
		size_t layersCount = sortingLayers.size();
		for (int i = 0; i < layersCount; i++) {
			if (sortingLayers[i] == layer) return i;
		}

		return -1;
	}

	void addRenderer(ZRef<ZOrthoRenderer> renderer) {
		if (renderer->renderIndex > 0) return;

		renderer->renderIndex = renderers.size();
		renderers.push_back(renderer);
	}

	void removeRenderer(ZRef<ZOrthoRenderer> renderer) {
		int16_t index = renderer->renderIndex;
		if (index < 0) return;

		if (renderers[index] == renderer) {
			renderers.erase(renderers.begin() + index);
		}
	}

	void render() {
		IVec2 intWindowSize = window->getSize(); //the window width and height in int
		Vec2 windowSize = Vec2(intWindowSize.x, intWindowSize.y);

		std::vector<ZRef<ZCamera>> renderCameras;
		for (ZRef<ZCamera> camera : cameras) {
			if (!camera->isEnabled() || !camera->getOwner()->isEnabled()) continue;

			bool camAdded = false;
			unsigned int index = 0;

			for (ZRef<ZCamera> cam : renderCameras) {
				if (camera->priority < cam->priority) {
					renderCameras.insert(renderCameras.begin() + index, camera);
					camAdded = true;
					break;
				}
			}

			if (!camAdded) renderCameras.emplace_back(camera);
		}

		renderer->setRenderDrawColor(U8Vec4(0, 0, 0, 0));
		renderer->clearScreen();

		for (ZRef<ZCamera> camera : renderCameras) {
			Vec4 viewport = Vec4(camera->viewRect.x * windowSize.x, camera->viewRect.y * windowSize.y,
				camera->viewRect.z * windowSize.x, camera->viewRect.w * windowSize.y);

			switch (camera->clearFlag) {
				case CameraClearFlag::solidColor: {
					renderer->setRenderDrawColor(U8Vec4(camera->background.x, camera->background.y, camera->background.z, 1.0f));
					renderer->fillRectF(viewport);
				}
				default:
					NULL;
			}

			std::vector<ZRef<ZOrthoRenderer>> entries;

			for (ZRef<ZOrthoRenderer> newEntry : renderers) {
				if (!newEntry->isEnabled() || newEntry->getOwner()->isMarkedForDelete() || !newEntry->getOwner()->isEnabled()) continue;

				if (camera->sortingLayers.size() != 0) {
					bool found = false;
					for (std::string sortingLayer : camera->sortingLayers) {
						if (sortingLayer == newEntry->sortingLayer) {
							found = true;
							break;
                        }
                    }

                    if (found) {
						if (camera->cullingMask == CullingMask::exclude) continue;
                    }
					
					else {
						if (camera->cullingMask == CullingMask::include) continue;
					}
				}

				bool entryAdded = false;
				unsigned int index = 0;
				for (ZRef<ZOrthoRenderer> entry : entries) {
					if (newEntry->sortingLayerIndex < entry->sortingLayerIndex) {
						entries.insert(entries.begin() + index, newEntry);
						entryAdded = true;
						break;
					}

					else if (newEntry->sortingLayerIndex == entry->sortingLayerIndex) {
						if (newEntry->orderInLayer < entry->orderInLayer) {
							entries.insert(entries.begin() + index, newEntry);
							entryAdded = true;
							break;
						}
					}

					index++;
				}

				if (!entryAdded) entries.emplace_back(newEntry);
			}

			float unit = static_cast<float>(viewport.w) / camera->size;
			for (ZRef<ZOrthoRenderer> entry : entries) {
				entry->render(camera, viewport, unit);
			}
		}
	}
};

void ZOrthoRenderer::onAttach() {
	ZRef<ZOrthoRp> rp = getRenderPipeline();
	rp->addRenderer(self);
}

void ZOrthoRenderer::onDestroy() {
	ZRef<ZOrthoRp> rp = getRenderPipeline();
	rp->removeRenderer(self);
}

void ZOrthoRenderer::setSortingLayer(std::string newLayer) {
	if (sortingLayer == newLayer) return;

	sortingLayer = newLayer;
	ZRef<ZOrthoRp> rp = getRenderPipeline();
	sortingLayerIndex = rp->getLayerIndex(newLayer);
}

class ZSpriteRenderer : public ZOrthoRenderer {
	GENERATET_COMPONENT_BODY(ZSpriteRenderer, ZOrthoRenderer);

public:
	ZRef<ZTexture> sprite;

	ZSpriteRenderer() {
		this->updateEnabled = false;
	}

	void render(ZRef<ZCamera> camera, const Vec4& viewport, float unit) {
		ZRef<ZEntity> owner = getOwner();
		Vec2 position = owner->getPosition();
		Vec2 scale = owner->getScale();
		
		Vec4 coords = Vec4(0.0f);
		coords.x = (viewport.z / 2.0f) - unit * (scale.x / 2.0f - position.x) + viewport.x;
		coords.y = (viewport.w / 2.0f) - unit * (scale.y / 2.0f + position.y) + viewport.y;

		coords.z = unit * scale.x;
		coords.w = unit * scale.y;

		getRenderer()->setRenderDrawColor(U8Vec4(255, 255, 255, 255));

		if (sprite.isValid()) {
			IVec2 spriteSize = sprite->getSize();
			sprite->render(IVec4(0, 0, spriteSize.x, spriteSize.y), coords, owner->getRotation());
		}
	}
};

class ZTextRenderer : public ZOrthoRenderer {
	GENERATET_COMPONENT_BODY(ZTextRenderer, ZOrthoRenderer);

public:
	ZRef<ZTexture> font;
	std::string text;
	float spacing = 0.0f;

	ZTextRenderer() {
		this->updateEnabled = false;
	}

	float getSizeH() {
		if (font.isValid() && text != "") {
			ZRef<ZEntity> owner = getOwner();
			Vec2 position = owner->getPosition();
			Vec2 scale = owner->getScale();

			float charSize = scale.x * (spacing + 1);

			return text.length() * charSize - spacing * scale.x;
		}

		return 0.0f;
	}

	void render(ZRef<ZCamera> camera, const Vec4& viewport, float unit) {
		ZRef<ZEntity> owner = getOwner();
		Vec2 position = owner->getPosition();
		Vec2 scale = owner->getScale();

		Vec4 coords = Vec4(0.0f);
		coords.x = (viewport.z / 2.0f) - unit * (scale.x / 2.0f - position.x) + viewport.x;
		coords.y = (viewport.w / 2.0f) - unit * (scale.y / 2.0f + position.y) + viewport.y;

		coords.z = unit * scale.x;
		coords.w = unit * scale.y;

		float fontStep = coords.z * (spacing + 1);

		getRenderer()->setRenderDrawColor(U8Vec4(255, 255, 255, 255));

		if (font.isValid()) {
			IVec2 spriteSize = font->getSize();
			int colSize = spriteSize.x / 16;
			int rowSize = spriteSize.y / 16;

			for (unsigned char c : text) {
				int rowPos = c / 16;
				int colPos = c - (rowPos * 16);
				font->render(IVec4(colPos * colSize, rowPos * rowSize, colSize, rowSize), coords, owner->getRotation());
				coords.x += fontStep;
			}
		}
	}
};

#endif // !ZENGINE_ZORTHORP
