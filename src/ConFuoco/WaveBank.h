#ifndef CONFUOCO_WAVEBANK_H_
#define CONFUOCO_WAVEBANK_H_

#include "Common/NonCopyable.h"

namespace ConFuoco
{
	class Sound;
	class Stream;
	class Wave;

	/// Creates and manages all audio waves.
	///
	/// Copyright 2012 Bifrost Entertainment. All rights reserved.
	/// \author Tommy Nguyen
	class WaveBank : public NonCopyable<WaveBank>
	{
		friend class Mixer;

	public:
		static const unsigned int size = 16;  ///< Maximum number of audio sources.

		inline ~WaveBank();

	private:
		unsigned int sound_count;   ///< Number of sounds in the wave bank.
		unsigned int stream_count;  ///< Number of streams in the wave bank.
		Wave *bank[size << 1];      ///< Wave storage.

		inline WaveBank();

		/// Clear out the wave bank.
		void clear();

		/// Create a sound.
		Sound* create_sound(const unsigned int instances);

		/// Create a stream.
		Stream* create_stream();

		/// Delete sound.
		void remove(const Sound *);

		/// Delete stream.
		void remove(const Stream *);

		/// Update all streams.
		void update();
	};

	WaveBank::WaveBank() : sound_count(0), stream_count(0) { }

	WaveBank::~WaveBank()
	{
		this->clear();
	}
}

#endif
