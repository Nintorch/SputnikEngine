#include "Animation.hpp"

namespace Sputnik
{
	void AnimationPlayer::play(const Animation& animation)
	{
		if (this->animation == &animation)
			return;
		
		this->animation = &animation;
		reset();
	}

	bool AnimationPlayer::update(float delta_time, Rect& sprite)
	{
		if (!animation)
			return false;

		timer += delta_time;
		if (timer >= animation->delay)
		{
			do
			{
				frame++;
				timer -= animation->delay;
			} while (timer >= animation->delay);

			if (frame >= animation->frames.size())
			{
				if (animation->loop)
					frame = animation->loop_frame;
				else
				{
					stop();
					return false;
				}
			}

			Animation::AnimationFrame f = animation->frames[frame];

			sprite = { f.x,f.y,f.w,f.h };
			return true;
		}
		return false;
	}

	void AnimationPlayer::reset()
	{
		if (animation)
		{
			timer = animation->delay;
			frame = -1;
		}
		else
		{
			timer = 0;
			frame = 0;
		}
	}

	void AnimationPlayer::stop()
	{
		if (callback)
			callback();
		
		animation = nullptr;
		reset();
	}

	bool AnimationPlayer::is_playing(const Animation& animation)
	{
		return this->animation == &animation;
	}
	
	void AnimationPlayer::set_finish_callback(std::function<void()> callback)
	{
		this->callback = callback;
	}
	
	void AnimationPlayer::reset_finish_callback()
	{
		callback = nullptr;
	}

	Animation::Animation(float delay, std::vector<AnimationFrame> frames)
		: delay(delay), frames(frames) {}

	Animation::Animation(float delay, int loop_frame, std::vector<AnimationFrame> frames)
		: delay(delay), frames(frames), loop(true), loop_frame(loop_frame) {}
}