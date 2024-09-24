#include "Audio.hpp"

#include "Platform.hpp"
#include "Exceptions.hpp"
#include "config.hpp"
#include "Utils.hpp"

#include "soloud_wavstream.h"
#include "soloud_wav.h"

#include <unordered_map>

namespace Sputnik
{
	namespace Audio
	{
		SoLoud::Soloud engine;

		SoLoud::Soloud& get_engine()
		{
			return engine;
		}

		void init()
		{
			engine.init(SoLoud::Soloud::CLIP_ROUNDOFF, SoLoud::Soloud::AUDIO_BACKEND);

			Filters::get_underwater_filter().setParams(SoLoud::BiquadResonantFilter::LOWPASS, 350, 1);
		}

		void quit()
		{
			// Stop the music and release the music audio source
			Music::stop();

			engine.deinit();
		}

		void stop_all()
		{
			engine.stopAll();
		}

		void pause_all()
		{
			engine.setPauseAll(true);
		}

		void unpause_all()
		{
			engine.setPauseAll(false);
		}

		void set_global_filter(unsigned int id, SoLoud::Filter& filter)
		{
			if (id >= 8)
				return;

			if (!engine.mFilter[id] || engine.mFilter[id] != &filter)
				engine.setGlobalFilter(id, &filter);
		}

		void clear_global_filter(unsigned int id)
		{
			engine.setGlobalFilter(id, nullptr);
		}

		void set_global_volume(float volume)
		{
			engine.setGlobalVolume(volume);
		}

		float get_global_volume()
		{
			return engine.getGlobalVolume();
		}

		namespace Filters
		{
			SoLoud::BiquadResonantFilter& get_underwater_filter()
			{
				static SoLoud::BiquadResonantFilter filter;
				return filter;
			}
		}

		namespace Music
		{
			SoLoud::handle handle;
			std::unique_ptr<SoLoud::AudioSource> music_source;

			// T returns std::unique_ptr<SoLoud::AudioSource>
			template<typename T>
			static void play_base(T load, double loop_point)
			{
				if (engine.isValidVoiceHandle(handle))
					engine.stop(handle);

				auto s = load();
				if (!s)
					return;
				music_source = std::move(s);

				handle = engine.playBackground(*music_source);

				if (loop_point >= 0)
				{
					engine.setLooping(handle, true);
					engine.setLoopPoint(handle, loop_point);
				}
			}
			
			void play(const char* filename, double loop_point)
			{
				play_base([=]() {
					auto ptr = Utils::make_unique<SoLoud::WavStream>();
					if (ptr->load(filename) == SoLoud::SO_NO_ERROR)
						return ptr;
					else
						return std::unique_ptr<SoLoud::WavStream>{};
					}, loop_point);
			}

			void play(unsigned char* buffer, int size, double loop_point)
			{
				play_base([=]() {
					auto ptr = Utils::make_unique<SoLoud::WavStream>();
					if (ptr->loadMem(buffer, size, false, false) == SoLoud::SO_NO_ERROR)
						return ptr;
					else
						return std::unique_ptr<SoLoud::WavStream>{};
					}, loop_point);
			}

			void play(Resource::Handle resource, double loop_point)
			{
				if (resource == nullptr)
				{
					Log::error(SE_FUNCTION, ": resource is null");
					return;
				}

				if (!resource->check_type(Resource::Type::SOUND))
				{
					Log::error(SE_FUNCTION, ": Resource '", resource->get_name(), "' is not a sound resource (type: ", (int)resource->get_type(), ")");
					return;
				}

				play(resource->get_buffer(), resource->get_size(), loop_point);
			}
			
			void play(std::unique_ptr<SoLoud::AudioSource> source)
			{
				play_base([&]() {
					return std::move(source);
					}, -1);
			}

			void stop(double fade_seconds)
			{
				if (fade_seconds > 0)
					fade_out(fade_seconds, true);
				else
					engine.stop(handle);

				music_source.reset();
			}
			
			void stop_after(double seconds)
			{
				engine.scheduleStop(handle, seconds);
			}

			void pause()
			{
				engine.setPause(handle, true);
			}

			void unpause()
			{
				engine.setPause(handle, false);
			}

			void set_filter(unsigned int id, SoLoud::Filter& filter)
			{
				music_source->setFilter(id, &filter);
			}

			void clear_filter(unsigned int id)
			{
				music_source->setFilter(id, nullptr);
			}

			void clear_filters()
			{
				for (int i = 0; i < (sizeof(music_source->mFilter) / sizeof(music_source->mFilter[0])); i++)
					music_source->setFilter(i, nullptr);
			}

			void set_volume(float volume, double fade_seconds)
			{
				engine.fadeVolume(handle, volume, fade_seconds);
			}

			void set_speed(float speed, double fade_seconds)
			{
				engine.fadeRelativePlaySpeed(handle, speed, fade_seconds);
			}

			void fade_out(double seconds, bool stop_after)
			{
				engine.fadeVolume(handle, 0.f, seconds);
				if (stop_after)
					engine.scheduleStop(handle, seconds);
			}

			void fade_in(double seconds, bool stop_after)
			{
				engine.fadeVolume(handle, 1.f, seconds);
				if (stop_after)
					engine.scheduleStop(handle, seconds);
			}

			SoLoud::handle get_handle()
			{
				return handle;
			}

			const SoLoud::AudioSource& get_audio_source()
			{
				return *music_source;
			}
		}

		namespace Sound
		{
#if _DEBUG
			static int sounds_allocated = 0;

			int SFX::get_allocated_count()
			{
				return sounds_allocated;
			}
#else
			int SFX::get_allocated_count()
			{
				return -1;
			}

			SFX::SFX() {}
#endif

			std::unordered_map<unsigned char*, std::shared_ptr<SoLoud::AudioSource>> sfx_map;

			SFX::~SFX()
			{
				unload();
			}

			SoLoud::handle SFX::play()
			{
				return engine.play(*ptr);
			}

			void SFX::unload()
			{
				if (ptr)
				{
					auto it = sfx_map.find(buffer);
					if (it != sfx_map.end())
					{
						if (it->second.use_count() <= 2) // 1 for the SFX object and 1 for the shared_ptr inside of sfx_map
						{
							sfx_map.erase(it);
							Log::info("SOUND: Unloaded SFX at buffer ", (void*)buffer);
						}
					}

#if _DEBUG
					sounds_allocated--;
#endif
					ptr.reset();
				}
			}

			SFX::SFX(std::shared_ptr<SoLoud::AudioSource> ptr, unsigned char* buffer)
				: ptr(ptr), buffer(buffer)
			{
				sounds_allocated++;
				Log::info("SOUND: Loaded SFX from buffer ", (void*)buffer);
			}

			SFX load(Resource::Handle resource)
			{
				if (resource == nullptr)
				{
					Log::error(SE_FUNCTION, ": resource is null");
					return { nullptr, nullptr };
				}

				if (!resource->check_type(Resource::Type::SOUND))
				{
					Log::error(SE_FUNCTION, ": Resource '", resource->get_name(), "' is not a sound resource (type: ", (int)resource->get_type(), ")");
					return { nullptr, nullptr };
				}
				
				auto it = sfx_map.find(resource->get_buffer());
				if (it != sfx_map.end())
					return SFX(it->second, it->first);

				std::shared_ptr<SoLoud::Wav> ptr = Utils::make_shared<SoLoud::Wav>();
				ptr->loadMem(resource->get_buffer(), resource->get_size(), false, false);
				sfx_map[resource->get_buffer()] = ptr;
				return SFX(ptr, resource->get_buffer());
			}
		}
	}
}