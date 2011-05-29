#ifndef COLOR_H_
#define COLOR_H_

namespace Rainbow
{
	/// Structure for storing a color (RGBA).
	///
	/// Copyright 2010-11 Bifrost Games. All rights reserved.
	/// \author Tommy Nguyen
	template<typename T>
	struct _Color
	{
		T r, g, b, a;
	};

	/// Structure for storing a color (RGBA) using unsigned bytes.
	///
	/// Copyright 2010-11 Bifrost Games. All rights reserved.
	/// \author Tommy Nguyen
	template<>
	struct _Color<unsigned char>
	{
		unsigned char r, g, b, a;

		_Color() : r(0xff), g(0xff), b(0xff), a(0xff) { }
		_Color(const _Color<unsigned char> &c) : r(c.r), g(c.g), b(c.b), a(c.a) { }
		_Color(const unsigned char r, const unsigned char g, const unsigned char b, const unsigned char a = 0xff) :
			r(r), g(g), b(b), a(a)
		{ }

		_Color<unsigned char>& operator=(const unsigned int c)
		{
			this->r = 0xff & (c >> 24);
			this->g = 0xff & (c >> 16);
			this->b = 0xff & (c >> 8);
			this->a = 0xff & c;
			return *this;
		}
	};

	/// Structure for storing a color (RGBA) using floats.
	///
	/// Copyright 2010-11 Bifrost Games. All rights reserved.
	/// \author Tommy Nguyen
	template<>
	struct _Color<float>
	{
		float r, g, b, a;

		_Color() : r(1.0f), g(1.0f), b(1.0f), a(1.0f) { }

		_Color(const float r, const float g, const float b, const float a = 1.0f) :
			r(r), g(g), b(b), a(a)
		{ }

		_Color<float>& operator=(const unsigned int c)
		{
			static const float white = 1.0f / 255.0f;
			this->r = (0xff & (c >> 24)) * white;
			this->g = (0xff & (c >> 16)) * white;
			this->b = (0xff & (c >> 8)) * white;
			this->a = (0xff & c) * white;
			return *this;
		}
	};
}

typedef Rainbow::_Color<unsigned char> Colorb;
typedef Rainbow::_Color<float> Colorf;

#endif
