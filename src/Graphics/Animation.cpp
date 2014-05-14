// Copyright (c) 2010-14 Bifrost Entertainment AS and Tommy Nguyen
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)

#include "Graphics/Animation.h"

Animation::Animation(const Sprite::Ref &sprite,
                     const Frame *const frames,
                     const unsigned int fps,
                     const int delay)
    : TimedEvent(1000.0f / fps), frame_(0), sprite_(sprite), frames_(frames),
      delay_(delay), idled_(0) { }

void Animation::set_frames(const Frame *const frames)
{
	frames_.reset(frames);
	reset();
}

void Animation::tick()
{
	sprite_->set_texture(frames_[frame_]);
	if (frames_[frame_ + 1] == kAnimationEnd)
	{
		if (delay_ < 0)
			stop();
		else if (idled_ < delay_)
			++idled_;
		else
			reset();
	}
	else
		++frame_;
}
