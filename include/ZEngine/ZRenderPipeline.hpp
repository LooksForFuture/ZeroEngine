#ifndef ZENGINE_ZRENDERPIPELINE
#define ZENGINE_ZRENDERPIPELINE

#include <ZEngine/ZCore.hpp>
#include <ZEngine/ZRef.hpp>
#include <ZEngine/ZComponent.hpp>
#include <ZEngine/ZRenderer.hpp>

#include <vector>

class ZCamera;

struct ZRenderPipeline : virtual RTTI::Enable {
	RTTI_DECLARE_TYPEINFO(ZRenderPipeline);

protected:
	ZRef<ZWindow> window;
	ZRef<ZRenderer> renderer;
	std::vector<ZRef<ZCamera>> cameras;

public:
	ZRenderPipeline() {}
	ZRenderPipeline(ZRef<ZWindow> window, ZRef<ZRenderer> renderer) {
		this->window = window;
		this->renderer = renderer;
	}

	void addCamera(ZRef<ZCamera> camera) {
		bool found = false;
		for (ZRef<ZCamera> cam : cameras) {
			if (cam == camera) {
				found = true;
				break;
			}
		}

		if (!found) cameras.push_back(camera);
	}

	void removeCamera(ZRef<ZCamera> camera) {
		size_t count = cameras.size();
		for (int i = 0; i < count; i++) {
			if (cameras[i] == camera) {
				cameras.erase(cameras.begin() + i);
			}
		}
	}

	virtual void render() {}
};

#endif // !ZENGINE_ZRENDERPIPELINE
