#include <glad/glad.h>

class Texture2D {
    bool created = false;

    bool gamma = true, mipmap = true;
    unsigned int textureID = 0;
    int width = 1, height = 1, nrChannels = 3;

    GLenum internalFormat;
    GLenum dataFormat;

    int wrapS, wrapT, minFilter, magFilter;

public:
	Texture2D() {}
	Texture2D(int width, int height, int nrChannels, unsigned char* data, bool gamma=true, bool mipmap=true,
        int wrapS=GL_REPEAT, int wrapT=GL_REPEAT, int minFilter=GL_LINEAR_MIPMAP_LINEAR, int magFilter=GL_NEAREST) : width(width), height(height), nrChannels(nrChannels),
    gamma(gamma), mipmap(mipmap), wrapS(wrapS), wrapT(wrapT), minFilter(minFilter), magFilter(magFilter){
        created = true;
        glGenTextures(1, &textureID);

        if (nrChannels == 1) {
            internalFormat = dataFormat = GL_RED;
        }
        else if (nrChannels == 3) {
            internalFormat = gamma ? GL_SRGB : GL_RGB;
            dataFormat = GL_RGB;
        }
        else if (nrChannels == 4) {
            internalFormat = gamma ? GL_SRGB_ALPHA : GL_RGBA;
            dataFormat = GL_RGBA;
        }

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data);

        if(mipmap) glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
	}

    void create(int width, int height, int nrChannels, unsigned char* data, bool gamma = true, bool mipmap = true,
        int wrapS = GL_REPEAT, int wrapT = GL_REPEAT, int minFilter = GL_LINEAR_MIPMAP_LINEAR, int magFilter = GL_NEAREST) {
        created = true;
        glGenTextures(1, &textureID);

        if (nrChannels == 1) {
            internalFormat = dataFormat = GL_RED;
        }
        else if (nrChannels == 3) {
            internalFormat = gamma ? GL_SRGB : GL_RGB;
            dataFormat = GL_RGB;
        }
        else if (nrChannels == 4) {
            internalFormat = gamma ? GL_SRGB_ALPHA : GL_RGBA;
            dataFormat = GL_RGBA;
        }

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data);

        if (mipmap) glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
    }

    bool isCreated() {
        return created;
    }
    unsigned int getId() { return textureID; }

    ~Texture2D() {
        if(created) glDeleteTextures(1, &textureID);
    }
};
