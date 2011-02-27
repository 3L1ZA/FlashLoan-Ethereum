/// Font glyph structure for storing advance and a textured sprite.

/// Copyright 2010 Bifrost Games. All rights reserved.
/// \author Tommy Nguyen

#ifndef FONTGLYPH_H_
#define FONTGLYPH_H_

#include "Types.h"

struct FontGlyph
{
	short int advance;
	SpriteVertex quad[4];
};

#endif
