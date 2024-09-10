#ifndef ZENGINE_ZCAMERA
#define ZENGINE_ZCAMERA

#include <ZEngine/ZCore.hpp>
#include <ZEngine/ZRef.hpp>
#include <ZEngine/ZComponent.hpp>
#include <ZEngine/ZRenderer.hpp>
#include <ZEngine/ZEngine.hpp>

enum class CameraProjection {
	systemDefault, perspective, orthographic
};

enum class CameraClearFlag {
	none, solidColor
};

enum class CullingMask {
	include, exclude
};

class ZCamera : public ZComponent {
public:
	CameraClearFlag clearFlag = CameraClearFlag::solidColor;
	CullingMask cullingMask = CullingMask::include;

	U8Vec3 background = U8Vec3(255);
	Vec4 viewRect = Vec4(0.0f, 0.0f, 1.0f, 1.0f);

	bool depthTest = true;
	unsigned char priority = 0;

	float ratio = 16.0 / 10.0f;
	float size = 5.0f;

	Vec4 ambientLight;

	std::vector<std::string> sortingLayers;

	void onAttach() {
		ZRef<ZRenderPipeline> pipeline = getRenderPipeline();
		pipeline->addCamera(self);
	}

	void onDestroy() {
		ZRef<ZRenderPipeline> pipeline = getRenderPipeline();
		pipeline->removeCamera(self);
	}
};

#endif // !ZENGINE_ZCAMERA
