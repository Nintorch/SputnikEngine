#ifndef __AUDIO_H__
#define __AUDIO_H__

#include "Resource.hpp"

#include "soloud.h"
#include "soloud_biquadresonantfilter.h"

#include <memory>

namespace Sputnik
{
	/*
		Sputnik Engine's audio system.

		It uses SoLoud audio library with different platform-specific backends
		to provide real-time audio filters, volume changing, overall
		decent sound for games and more.

		SoLoud can be found here: http://soloud-audio.com/
	*/
	namespace Audio
	{
		SoLoud::Soloud& get_engine();

		void init();
		void quit();

		void stop_all();
		void pause_all();
		void unpause_all();

		void set_global_filter(unsigned int id, SoLoud::Filter& filter);
		void clear_global_filter(unsigned int id);

		// Set global volume using a multiplier (1.0f is default volume and 0.0f is no audio).
		void set_global_volume(float volume);
		// Get global volume (1.0f is default volume and 0.0f is no audio).
		float get_global_volume();

		namespace Filters
		{
			SoLoud::BiquadResonantFilter& get_underwater_filter();
		}

		/*
			Functions related to playing and manipulating background music
		*/
		namespace Music
		{
			// Play wave stream.
			// Format can be .ogg, .mp3, flac or .wav

			void play(const char* filename, double loop_point = -1);
			void play(unsigned char* buffer, int size, double loop_point = -1);
			void play(Resource::Handle resource, double loop_point = -1);
			
			// Play a custom audio source
			void play(std::unique_ptr<SoLoud::AudioSource> source);

			void stop(double fade_seconds = 0.0);
			void stop_after(double seconds);
			void pause();
			void unpause();

			void set_filter(unsigned int id, SoLoud::Filter& filter);
			void clear_filter(unsigned int id);
			void clear_filters();

			void set_volume(float volume, double fade_seconds = 0.0);
			void set_speed(float speed, double fade_seconds = 0.0);

			void fade_out(double seconds, bool stop_after = false);
			void fade_in(double seconds, bool stop_after = false);

			SoLoud::handle get_handle();
			const SoLoud::AudioSource& get_audio_source();
		}

		/*
			In-game sound effects
		*/
		namespace Sound
		{
			/*
				Class that manages loaded SFX data.
				The data is loaded by the load function below and is automatically
				disposed when the object of this class is destroyed.

				It uses std::shared_ptr internally so an already loaded data
				is used instead of allocating new data if the sound effect was
				already loaded and not yet deallocated.
			*/
			class SFX
			{
			public:
				static int get_allocated_count();

				~SFX();

				SoLoud::handle play();
				void unload();

			private:
				std::shared_ptr<SoLoud::AudioSource> ptr;
				unsigned char* buffer;

				SFX(std::shared_ptr<SoLoud::AudioSource> ptr, unsigned char* buffer);
				friend SFX load(Resource::Handle resource);
			};

			SFX load(Resource::Handle resource);
		}
	}
}

#endif // __AUDIO_H__