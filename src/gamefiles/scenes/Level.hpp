#ifndef __LEVEL_H__
#define __LEVEL_H__

#include "Scene.hpp"
#include "Object.hpp"
#include "Audio.hpp"

#include <memory>

class Level;

class Player : public Sputnik::SpriteObject
{
public:
	enum class State : char
	{
		STAND,
		JUMP
	};

	State state = State::STAND;

	Player();
	void update(float delta_time) override;
	void render(float delta_time) override;

private:
	Sputnik::Audio::Sound::SFX jump_sfx =
		Sputnik::Audio::Sound::load(Sputnik::Resource::get("JUMP"));

	void push_out(std::shared_ptr<Level>& scene);
};

class Level : public Sputnik::Scene
{
public:
	void init() override;
	void returned() override;
	void update(float delta_time) override;
	void render(float delta_time) override;

	Sputnik::TileMapObject tileset{128, 128};
	Player player;

	float water_level = 380.f;

private:
	float timer;

	Sputnik::Texture bg_texture;
	Sputnik::BackgroundHandler bg;
};
#endif // __LEVEL_H__