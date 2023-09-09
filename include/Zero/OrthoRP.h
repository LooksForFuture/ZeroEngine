struct OrthoRenderEntry {
    std::vector<Mat4> models;
    std::vector<Vec4> colors;
    std::shared_ptr<Texture2D> sprite;
    unsigned int sortingLayer = 0;
    unsigned int orderInLayer;
};

class OrthoRenderer : public Component {
public:
    std::string sortingLayer = "";
    unsigned int orderInLayer = 0;

    virtual void configure();

    virtual std::vector<OrthoRenderEntry> render(Camera* camera) {
        std::vector<OrthoRenderEntry> entries;
        return entries;
    }

    virtual void onDestroy();
};

class OrthoRP : public RenderPipeline {
private:
    friend OrthoRenderer;
    std::vector<OrthoRenderer*> renderers;

    unsigned int lastSprite = NULL; // ID of the last rendered texture

    virtual void addRenderer(OrthoRenderer* renderer) {
        renderers.emplace_back(renderer);
    }

    virtual void removeRenderer(OrthoRenderer* renderer) {
        size_t i = 0;
        for (OrthoRenderer* ren : renderers) {
            if (ren == renderer) {
                renderers.erase(renderers.begin() + i);
                break;
            }

            i++;
        }
    }

public:
    Shader* program;
    Mesh* qaud;

    OrthoRP() {
        glEnable(GL_FRAMEBUFFER_SRGB);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        std::vector<Vertex> vertices(4);
        vertices[0].position = Vec3(0.5f, 0.5f, 0.0f); // top right
        vertices[0].uv = Vec2(1.0f, 1.0f);

        vertices[1].position = Vec3(0.5f, -0.5f, 0.0f); // bottom right
        vertices[1].uv = Vec2(1.0f, 0.0f);

        vertices[2].position = Vec3(-0.5f, -0.5f, 0.0f); // bottom left
        vertices[2].uv = Vec2(0.0f, 0.0f);

        vertices[3].position = Vec3(-0.5f, 0.5f, 0.0f); // top left
        vertices[3].uv = Vec2(0.0f, 1.0f);

        std::vector<unsigned int> indices = { 0, 1, 3, 1, 2, 3 };

        qaud = new Mesh(vertices, indices);

        std::string vertexCode =
            "#version 330 core\n"
            "layout (location = 0) in vec3 aPos;"
            "layout (location = 1) in vec3 aNormal;"
            "layout (location = 2) in vec2 aTexCoords;"
            "out vec2 TexCoords;"
            "out vec3 FragPos;"
            "uniform mat4 model;"
            "uniform mat4 view;"
            "uniform mat4 projection;"
            "void main() {"
            "    TexCoords = aTexCoords;"
            "    FragPos = vec3(model * vec4(aPos, 1.0));"
            "    gl_Position = projection * view * model * vec4(aPos, 1.0);"
            "}";

        std::string fragmentCode =
            "#version 330 core\n"
            "out vec4 FragColor;"
            "in vec2 TexCoords;"
            "uniform vec4 color;"
            "uniform vec4 ambientLight;"
            "uniform sampler2D sprite;"
            "void main() {"
            "    FragColor = ambientLight * color * texture(sprite, TexCoords);"
            "}";

        program = new Shader(vertexCode, fragmentCode);

        defaultProjection = CameraProjection::orthographic;
    }

    void render() {
        glUseProgram(program->getId());
        glBindVertexArray(qaud->getVAO());

        std::vector<Camera*> renderCameras;
        for (Camera* camera : cameras) {
            if (!camera->isEnabled()) continue;

            bool camAdded = false;
            unsigned int index = 0;

            for (Camera* cam : renderCameras) {
                if (camera->priority < cam->priority) {
                    renderCameras.insert(renderCameras.begin() + index, camera);
                    camAdded = true;
                    break;
                }
            }

            if (!camAdded) renderCameras.emplace_back(camera);
        }

        for (Camera* camera : renderCameras) {
            switch (camera->clearFlag)
            {
            case CameraClearFlag::solidColor: {
                glClearColor(camera->background.x, camera->background.y, camera->background.z, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT);
            }
            default:
                NULL;
            }

            Mat4 projection = camera->getProjectionMatrix();
            Mat4 view = camera->getView();
            glUniform4f(program->getUniformLocation("ambientLight"), camera->ambientLight.r, camera->ambientLight.g, camera->ambientLight.b, camera->ambientLight.a);
            glUniformMatrix4fv(program->getUniformLocation("projection"), 1, GL_FALSE, glm::value_ptr(projection));
            glUniformMatrix4fv(program->getUniformLocation("view"), 1, GL_FALSE, glm::value_ptr(view));

            std::vector<OrthoRenderEntry> entries;
            for (OrthoRenderer* renderer : renderers) {
                if (renderer != nullptr && renderer->isEnabled()) {
                    if (camera->sortingLayers.size() != 0) {
                        bool found = false;
                        for (std::string sortingLayer : camera->sortingLayers) {
                            if (sortingLayer == renderer->sortingLayer) {
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

                    std::vector<OrthoRenderEntry> newEntries = renderer->render(camera);
                    for (OrthoRenderEntry newEntry : newEntries) {
                        bool entryAdded = false;
                        unsigned int index = 0;
                        for (OrthoRenderEntry const& entry : entries) {
                            if (newEntry.sortingLayer < entry.sortingLayer) {
                                entries.insert(entries.begin() + index, newEntry);
                                entryAdded = true;
                                break;
                            }

                            else if (newEntry.sortingLayer == entry.sortingLayer) {
                                if (newEntry.orderInLayer < entry.orderInLayer) {
                                    entries.insert(entries.begin() + index, newEntry);
                                    entryAdded = true;
                                    break;
                                }

                                else if (entry.sprite != nullptr && newEntry.sprite != nullptr && newEntry.orderInLayer == entry.orderInLayer) {
                                    if (newEntry.sprite->getId() == entry.sprite->getId()) {
                                        entries.insert(entries.begin() + index, newEntry);
                                        entryAdded = true;
                                        break;
                                    }
                                }
                            }

                            index++;
                        }

                        if (!entryAdded) entries.emplace_back(newEntry);
                    }
                }
            }

            unsigned int spriteId;
            for (OrthoRenderEntry const &entry : entries) {
                if (entry.sprite != nullptr && entry.sprite->isCreated()) {
                    spriteId = entry.sprite->getId();
                }

                else {
                    spriteId = engine->textures["blank"]->getId();
                }

                if (spriteId != lastSprite) {
                    glActiveTexture(GL_TEXTURE0); // activate the texture unit first before binding texture
                    glBindTexture(GL_TEXTURE_2D, spriteId);
                }

                lastSprite = spriteId;

                for (size_t i = 0; i < entry.models.size(); i++) {
                    Mat4 model = entry.models[i];
                    Vec4 color = entry.colors[i];

                    glUniformMatrix4fv(program->getUniformLocation("model"), 1, GL_FALSE, glm::value_ptr(model));
                    glUniform4f(program->getUniformLocation("color"), color.x, color.y, color.z, color.w);
                    glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(qaud->indices.size()), GL_UNSIGNED_INT, 0);
                }
            }
        }
    }

    void destruct() {
        qaud->destroy();
        delete qaud;
        delete program;
    }
};

void OrthoRenderer::configure() {
    OrthoRP* rp = (OrthoRP*)engine->getRenderPipeline();
    if (rp != nullptr) {
        rp->addRenderer(this);
    }
}

void OrthoRenderer::onDestroy() {
    OrthoRP* rp = (OrthoRP*)engine->getRenderPipeline();
    if (rp != nullptr) {
        rp->removeRenderer(this);
    }
}

class RenderSprite : public OrthoRenderer {
public:
    Vec4 color;
    std::shared_ptr<Texture2D> sprite;

    void configure(Vec4 color = Vec4(1.0f, 1.0f, 1.0f, 1.0f)) {
        OrthoRenderer::configure();
        this->color = color;
    }

    std::vector<OrthoRenderEntry> render(Camera* camera) {
        std::vector<OrthoRenderEntry> entries;
        std::vector<Mat4> models = { owner->transform->getModel() };
        std::vector<Vec4> colors = { color };
        OrthoRenderEntry entry = OrthoRenderEntry();
        entry.models = models;
        entry.sprite = sprite;
        entry.colors = colors;

        bool found = false;
        if (engine->sortingLayers.size() > 0) {
            for (unsigned int index = 0; index <= engine->sortingLayers.size(); index++) {
                if (engine->sortingLayers[index] == sortingLayer) {
                    entry.sortingLayer = index;
                    found = true;
                    break;
                }
            }
        }

        if (!found) {
            entry.sortingLayer = 0;
        }

        entry.orderInLayer = orderInLayer;
        entries.emplace_back(entry);
        return entries;
    }
};

class TileMap : public OrthoRenderer {
    IVec2 size;
    unsigned int area;

public:
    std::vector<std::shared_ptr<Texture2D>> tiles;
    std::vector<Vec4> colors;
    Vec2 spacing;

    void configure(IVec2 size = IVec2(1, 1)) {
        OrthoRenderer::configure();
        
        this->size = size;
        area = size.x * size.y;
        colors.reserve(area);

        std::shared_ptr<Texture2D> blank = engine->textures["blank"];
        for (unsigned int i = 0; i < area; i++) {
            tiles.emplace_back(blank);
            colors[i] = Vec4(1.0f);
        }
    }

    std::vector<OrthoRenderEntry> render(Camera* camera) {
        unsigned int layerIndex = 0;
        bool found = false;
        if (engine->sortingLayers.size() > 0) {
            for (unsigned int index = 0; index <= engine->sortingLayers.size(); index++) {
                if (engine->sortingLayers[index] == sortingLayer) {
                    layerIndex = index;
                    found = true;
                    break;
                }
            }
        }

        if (!found) {
            layerIndex = 0;
        }

        Vec3 position = transform->getPosition();
        Vec3 scale = transform->getScale();

        std::vector<OrthoRenderEntry> entries;
        for (unsigned int y = 0; y < size.y; y++) {
            for (unsigned int x = 0; x < size.x; x++) {
                Transform t;
                t.setPosition(Vec3(position.x + (x * scale.x + spacing.x * scale.x), position.y - (y * scale.y + spacing.y * scale.y), 0.0f));
                t.setScale(scale);

                OrthoRenderEntry entry;
                entry.colors.emplace_back(colors[y + x]);
                entry.orderInLayer = orderInLayer;
                entry.models.emplace_back(t.getModel());
                entry.sortingLayer = layerIndex;
                entry.sprite = tiles[y + x];

                entries.emplace_back(entry);
            }
        }

        return entries;
    }
};
