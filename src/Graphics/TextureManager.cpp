// Copyright (c) 2010-14 Bifrost Entertainment AS and Tommy Nguyen
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)

#include "Graphics/OpenGL.h"
#include "Graphics/TextureManager.h"

TextureManager* TextureManager::Instance = nullptr;

TextureManager::TextureManager() :
	mag_filter(GL_LINEAR), min_filter(GL_LINEAR), mem_peak(0.0), mem_used(0.0)
{
	memset(this->active, 0, sizeof(this->active));
}

TextureManager::~TextureManager()
{
	this->purge(this->recycled);
	this->purge(this->textures);
}

void TextureManager::bind(const unsigned int id)
{
	if (id == this->active[0])
		return;

	glBindTexture(GL_TEXTURE_2D, id);
	this->active[0] = id;
}

void TextureManager::bind(const unsigned int id, const unsigned int unit)
{
	R_ASSERT(unit < kNumTextureUnits, "Invalid texture unit");

	if (id == this->active[unit])
		return;

	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_2D, id);
	glActiveTexture(GL_TEXTURE0);
	this->active[unit] = id;
}

unsigned int TextureManager::create(const unsigned int internal_format,
                                    const unsigned int width,
                                    const unsigned int height,
                                    const unsigned int format,
                                    const void *data)
{
	TextureId texture = this->create_texture(width * height);
	glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
	R_ASSERT(glGetError() == GL_NO_ERROR, "Failed to load texture");

	this->textures.push_back(texture);
	this->print_usage();
	return texture.id;
}

unsigned int TextureManager::create_compressed(const unsigned int format,
                                               const unsigned int width,
                                               const unsigned int height,
                                               const size_t size,
                                               const void *data)
{
	TextureId texture = this->create_texture(width * height >> 1);
	glCompressedTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, size, data);
	R_ASSERT(glGetError() == GL_NO_ERROR, "Failed to load compressed texture");

	this->textures.push_back(texture);
	this->print_usage();
	return texture.id;
}

void TextureManager::memory_usage(double &used, double &unused, double &peak) const
{
	unused = this->recycled.size() * 64 * 64 * 1e-6;
	used = this->mem_used * 4e-6 + unused;
	peak = this->mem_peak * 4e-6;
}

void TextureManager::remove(const unsigned int id)
{
	for (size_t i = 0; i < this->textures.size(); ++i)
	{
		if (this->textures[i].id == id)
		{
			this->bind(this->textures[i].id);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, 64, 64, 0,
			             GL_LUMINANCE, GL_UNSIGNED_BYTE, nullptr);
			this->bind();
			this->recycled.push_back(this->textures[i]);
			this->mem_used -= this->textures[i].sz;
			this->textures.qerase(i);
			break;
		}
	}
}

void TextureManager::set_filter(const int filter)
{
	R_ASSERT(filter == GL_NEAREST || filter == GL_LINEAR,
	         "Invalid texture filter");

	this->mag_filter = filter;
	this->min_filter = filter;
}

TextureManager::TextureId TextureManager::create_texture(const unsigned int size)
{
	TextureId texture = { 0, size };
	if (!this->recycled.size())
		glGenTextures(1, &texture.id);
	else
	{
		texture.id = this->recycled[0].id;
		this->recycled.qerase(0);
	}
	this->mem_used += texture.sz;
	if (this->mem_used > this->mem_peak)
		this->mem_peak = this->mem_used;

	this->bind(texture.id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, this->min_filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, this->mag_filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	return texture;
}

void TextureManager::print_usage() const
{
#ifndef NDEBUG
	double used, unused, peak;
	this->memory_usage(used, unused, peak);
	R_DEBUG("[Rainbow] Video: %.2f MBs of textures (%.2f MBs unused)\n", used, unused);
#endif
}

void TextureManager::purge(Vector<TextureId> &t)
{
	if (!t.size())
		return;

	/* Hack to avoid allocating a new array of texture ids.
	 *
	 * Since sizeof(int) < sizeof(TextureId) and we will clear the vector by
	 * the end of this function, we can safely overwrite the array and avoid
	 * allocating more memory.
	 */
	static_assert(sizeof(unsigned int) < sizeof(TextureId),
	              "int cannot be greater than TextureId");
	unsigned int *textures = reinterpret_cast<unsigned int*>(t.begin());
	for (size_t i = 0; i < t.size(); ++i)
		textures[i] = t[i].id;

	glDeleteTextures(t.size(), textures);
	t.clear();

	this->print_usage();
}
