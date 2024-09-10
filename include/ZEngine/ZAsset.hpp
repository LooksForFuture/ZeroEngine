#ifndef ZENGINE_ZASSET
#define ZENGINE_ZASSET

#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

class ZAssetContainer;

struct ZAsset : virtual RTTI::Enable {
	friend class ZResources;
	friend class ZAssetContainer;
	RTTI_DECLARE_TYPEINFO(ZAsset);

private:
	std::string path;
	ZAssetContainer* assetContainer;

protected:
	bool loadedSuccessfully = false;

public:
	virtual void loadFromFile(std::string fileContent) {}
	virtual void release() {}

	std::string getPath() {
		return path;
	}

	bool isLoaded() { return loadedSuccessfully; }
};

class ZAssetContainer {
	friend class ZEngine;
	friend class ZResources;
	std::uint32_t assetId = 0;
	size_t capacity = 0;
	size_t size = 0;
	bool sizeControlled = false;
	std::vector<std::shared_ptr<ZAsset>> assets;

public:
	ZAssetContainer() = default;
	ZAssetContainer(std::uint32_t assetType) {
		assetId = assetType;
	}

	std::uint32_t getAssetsType() {
		return assetId;
	}

	void addAsset(std::shared_ptr<ZAsset> asset) {
		if (asset->isById(assetId)) {
			assets.push_back(asset);
		}
	}

	void clear() {
		for (std::shared_ptr<ZAsset> asset : assets) {
			asset->release();
		}
		assets.clear();
	}
};

struct ZAssetReport {
	bool failed = false;
	std::string content;
};

ZAssetReport readAsset(std::string path) {
	// Open the input file
	std::ifstream inputFile(path, std::ios::in | std::ios::binary);

	ZAssetReport report;

	// Check if the file is successfully opened 
	if (!inputFile.is_open()) {
		std::cerr << "Error opening file: " << path << '\n';
		report.failed = true;
		return report;
	}

	std::stringstream sstr;
	inputFile >> sstr.rdbuf();

	inputFile.close();
	report.content = sstr.str();

	return report;
}

class ZResources {
	static std::vector<std::shared_ptr<ZAsset>> defaults; //a default placeholder for every asset type

public:
	template <typename T>
	static ZRef<T> loadAsset(std::string path) {
		if (path == "") {
			for (std::shared_ptr<ZAsset> asset : defaults) {
				if (asset->is<T>()) {
					std::weak_ptr<T> weakAsset = std::reinterpret_pointer_cast<T>(asset);
					return ZRef<T>(weakAsset);
				}
			}

			return ZRef<T>();
		};

		bool foundContainer = false;
		ZAssetContainer* assetContainer = nullptr;
		for (const std::unique_ptr<ZAssetContainer>& container : ZEngine::data->assetContainers) {
			if (container->assets.size() > 0) {
				if (container->assets[0]->is<T>()) {
					assetContainer = container.get();
					foundContainer = true;
					break;
				}
			}
		}

		if (foundContainer) {
			for (std::shared_ptr<ZAsset> asset : assetContainer->assets) {
				if (asset->path == path) {
					std::weak_ptr<T> weakAsset = std::reinterpret_pointer_cast<T>(asset);
					return ZRef<T>(weakAsset);
				}
			}
		}

		ZAssetReport report = readAsset(path);

		if (report.failed) {
			for (std::shared_ptr<ZAsset> asset : defaults) {
				if (asset->is<T>()) {
					std::weak_ptr<T> weakAsset = std::reinterpret_pointer_cast<T>(asset);
					return ZRef<T>(weakAsset);
				}
			}

			return ZRef<T>();
		};

		std::shared_ptr<T> asset = std::make_shared<T>();
		asset->path = path;
		asset->loadFromFile(report.content);

		if (foundContainer) {
			assetContainer->addAsset(asset);
			asset->assetContainer = assetContainer;
		}

		else {
			for (const std::unique_ptr<ZAssetContainer>& container : ZEngine::data->assetContainers) {
				if (asset->isById(container->getAssetsType())) {
					container->assets.push_back(asset);
					asset->assetContainer = container.get();
					break;
				}
			}
		}

		std::weak_ptr<T> weakAsset = asset;
		return ZRef<T>(weakAsset);
	}

	template <typename T>
	static void registerAsset(std::shared_ptr<T> newAsset, std::string path) {
		if (path == "") return;

		bool foundContainer = false;
		ZAssetContainer* assetContainer = nullptr;
		for (const std::unique_ptr<ZAssetContainer>& container : ZEngine::data->assetContainers) {
			if (container->assets.size() > 0) {
				if (container->assets[0]->is<T>()) {
					assetContainer = container.get();
					foundContainer = true;
					break;
				}
			}
		}

		if (foundContainer) {
			for (std::shared_ptr<ZAsset> asset : assetContainer->assets) {
				if (asset->path == path) {
					return;
				}
			}
		}

		newAsset->path = path;

		if (foundContainer) {
			assetContainer->addAsset(newAsset);
			newAsset->assetContainer = assetContainer;
		}

		else {
			for (const std::unique_ptr<ZAssetContainer>& container : ZEngine::data->assetContainers) {
				if (newAsset->isById(container->getAssetsType())) {
					container->assets.push_back(newAsset);
					newAsset->assetContainer = container.get();
					break;
				}
			}
		}
	}

	static void registerDefault(std::shared_ptr<ZAsset> newAsset) {
		for (std::shared_ptr<ZAsset> asset : defaults) {
			if (asset == newAsset) return;
		}

		defaults.push_back(newAsset);
		newAsset->path = "";
	}

	static void clear() {
		for (std::shared_ptr<ZAsset> asset : defaults) {
			asset->release();
		}
	}
};

std::vector<std::shared_ptr<ZAsset>> ZResources::defaults = {};

#endif // !ZENGINE_ZASSET
