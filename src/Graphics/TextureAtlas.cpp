// Copyright 2010-12 Bifrost Entertainment. All rights reserved.

#include "Algorithm.h"
#include "Common/Data.h"
#include "Graphics/OpenGL.h"
#include "Graphics/TextureAtlas.h"

#if defined(RAINBOW_IOS)
#	include <UIKit/UIKit.h>
#else
#	include <png.h>

/// Structure for reading PNG bitmaps.
///
/// Copyright 2010-12 Bifrost Entertainment. All rights reserved.
/// \author Tommy Nguyen
struct png_read_struct
{
	size_t offset;
	const unsigned char *data;

	png_read_struct(unsigned char *data) : offset(8), data(data) { }
};

static void mem_fread(png_structp png_ptr, png_bytep data, png_size_t length)
{
	png_read_struct *read_struct = static_cast<png_read_struct*>(png_get_io_ptr(png_ptr));
	memcpy(data, read_struct->data + read_struct->offset, length);
	read_struct->offset += length;
}

#endif

TextureAtlas::TextureAtlas(const Data &img) : name(0), width(0), height(0)
{
	R_ASSERT(img, "No data provided");

	GLint internal = GL_RGBA8;
	GLint format = GL_RGBA;

#if defined(RAINBOW_IOS)

	UIImage *image = [[UIImage alloc] initWithData:(NSData*)img];
	R_ASSERT(image, "Failed to load file");

	this->width = CGImageGetWidth(image.CGImage);
	this->height = CGImageGetHeight(image.CGImage);
	R_ASSERT(this->is_valid(this->width) && this->is_valid(this->height),
	         "Texture dimension must be divisible by 4 and greater than 64");
	CGRect bounds = CGRectMake(0, 0, this->width, this->height);

	CGColorSpaceRef color_space = CGColorSpaceCreateDeviceRGB();
	unsigned char *data = new unsigned char[this->height * this->width * 4];

	CGContextRef context = CGBitmapContextCreate(data, this->width, this->height, 8, 4 * this->width, color_space, kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big);
	CGColorSpaceRelease(color_space);

	CGContextClearRect(context, bounds);
	CGContextTranslateCTM(context, 0, 0);
	CGContextDrawImage(context, bounds, image.CGImage);
	CGContextRelease(context);

	image = nil;

#else

	// Prepare for decoding PNG data
	png_read_struct texture(img);

	// Look for PNG signature
#ifndef NDEBUG
	int png_error =
#endif
	png_sig_cmp(texture.data, 0, texture.offset);
	R_ASSERT(!png_error, "File is not PNG");

	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
	if (!png_ptr)
	{
		R_ASSERT(false, "Failed to create PNG reading structure");
		return;
	}

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
	{
		png_destroy_read_struct(&png_ptr, nullptr, nullptr);
		R_ASSERT(false, "Failed to initialize PNG information structure");
		return;
	}

	if (setjmp(png_jmpbuf(png_ptr)))
	{
		png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
		R_ASSERT(false, "Failed to read PNG");
		return;
	}

	// Offset because we've read the signature
	png_set_sig_bytes(png_ptr, texture.offset);

	// Texture can't be greater than what the hardware supports
	png_set_user_limits(png_ptr, GL_MAX_TEXTURE_SIZE, GL_MAX_TEXTURE_SIZE);

	// Set reading function
	png_set_read_fn(png_ptr, &texture, mem_fread);

	// Retrieve PNG info
	png_read_info(png_ptr, info_ptr);
	const png_byte color_type = png_get_color_type(png_ptr, info_ptr);
	if (color_type != PNG_COLOR_TYPE_RGB_ALPHA)
	{
		switch(color_type)
		{
			case PNG_COLOR_TYPE_GRAY:
				if (png_get_bit_depth(png_ptr, info_ptr) < 8)
					png_set_expand_gray_1_2_4_to_8(png_ptr);
				internal = GL_LUMINANCE;
				format = GL_LUMINANCE;
				break;
			case PNG_COLOR_TYPE_PALETTE:
				png_set_palette_to_rgb(png_ptr);
			case PNG_COLOR_TYPE_RGB:
				internal = GL_RGB8;
				format = GL_RGB;
				break;
			case PNG_COLOR_TYPE_GRAY_ALPHA:
				if (png_get_bit_depth(png_ptr, info_ptr) < 8)
					png_set_expand_gray_1_2_4_to_8(png_ptr);
				internal = GL_LUMINANCE_ALPHA;
				format = GL_LUMINANCE_ALPHA;
				break;
			default:
				R_ERROR("Unknown PNG color type: %u\n", color_type);
				break;
		}
	}

	this->width = png_get_image_width(png_ptr, info_ptr);
	this->height = png_get_image_height(png_ptr, info_ptr);
	R_ASSERT(this->width > 0 && this->height > 0, "Invalid texture dimensions");

	// Allocate memory for bitmap
	png_read_update_info(png_ptr, info_ptr);
	const size_t rowbytes = png_get_rowbytes(png_ptr, info_ptr);
	unsigned char *data = new unsigned char[this->height * rowbytes];

	// Allocate row pointer array
	png_bytep *row_pointers = new png_bytep[this->height];

	png_byte *b = static_cast<png_byte*>(data);
	row_pointers[0] = b;
	for (int i = 1; i < this->height; ++i)
		row_pointers[i] = row_pointers[i - 1] + rowbytes;

	png_read_image(png_ptr, row_pointers);
	delete[] row_pointers;

	png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);

#endif

	this->name = TextureManager::Instance().create(internal, this->width, this->height, format, data);
	delete[] data;
}

unsigned int TextureAtlas::define(const int x, const int y, const int w, const int h)
{
	R_ASSERT(x >= 0 && (x + w) <= this->width && y >= 0 && (y + h) <= this->height,
	         "Invalid dimensions");

	const float x0 = x / static_cast<float>(this->width);
	const float x1 = (x + w) / static_cast<float>(this->width);
	const float y0 = y / static_cast<float>(this->height);
	const float y1 = (y + h) / static_cast<float>(this->height);

	const size_t i = this->textures.size();
	this->textures.push_back(Texture(x0, y0, x1, y1));
	this->textures[i].atlas = this->name;
	return i;
}

bool TextureAtlas::is_valid(const unsigned int i)
{
	return (i < 64) ? false : i % 4 == 0;
}
