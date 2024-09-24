#include "Level.hpp"
#include "App.hpp"
#include "Object.hpp"
#include "Input.hpp"
#include "Audio.hpp"
#include "Exceptions.hpp"
#include "Resource.hpp"
#include "Fade.hpp"
#include "Collision.hpp"
#include "Utils.hpp"
#include "Language.hpp"

#include "Logo.hpp"

#include "gamefiles/InputActions.hpp"
#include "gamefiles/resources.h"

#include <iostream>

using namespace Sputnik;

namespace
{
	Animation wait_animation = {
		0.5, 0,
		{
			{0,0,48,48},
			{48,0,48,48}
		}
	};

	Animation walk_animation = {
		0.1, 0,
		{
			{48 * 0,48,48,48},
			{48 * 1,48,48,48},
			{48 * 2,48,48,48},
			{48 * 3,48,48,48},
			{48 * 4,48,48,48},
			{48 * 5,48,48,48},
			{48 * 6,48,48,48},
			{48 * 7,48,48,48},
		}
	};

	Animation jump_animation = {
		0.2,
		{
			{0, 48 * 2,48,48},
			{48,48 * 2,48,48}
		}
	};
}

/* Code to test 3D */

/*constexpr const char* shader = "#version 110\n\
\
attribute vec3 gpu_Vertex;\n\
attribute vec2 gpu_TexCoord;\n\
attribute vec4 gpu_Color;\n\
uniform mat4 gpu_ModelViewProjectionMatrix;\n\
\
varying vec4 color;\n\
varying vec2 texCoord;\n\
\
void main(void)\n\
{\n\
	color = gpu_Color;\n\
	texCoord = vec2(gpu_TexCoord);\n\
	gl_Position = gpu_ModelViewProjectionMatrix * vec4(gpu_Vertex, 1.0);\n\
}";

constexpr const char* shader2 = "#version 110\n\
\
varying vec4 color;\n\
varying vec2 texCoord;\n\
\
uniform sampler2D tex;\n\
\
void main(void)\n\
{\n\
    gl_FragColor = texture2D(tex, texCoord) * color;\n\
}";*/

/*auto vertex = GPU_CompileShader(GPU_VERTEX_SHADER, shader);
auto fragment = GPU_CompileShader(GPU_FRAGMENT_SHADER, shader2);
auto program = GPU_LinkShaders(vertex, fragment);
auto block = GPU_LoadShaderBlock(program, "gpu_Vertex", "gpu_TexCoord", "gpu_Color", "gpu_ModelViewProjectionMatrix");

GPU_ActivateShaderProgram(program, &block);

GPU_MatrixMode(App::get_surface_target(), GPU_PROJECTION);
GPU_LoadIdentity();
GPU_Perspective(-90, -320.f / 240, 1, 1000);

GPU_MatrixMode(App::get_surface_target(), GPU_MODEL);
GPU_LoadIdentity();
GPU_Translate(-160, -120, -120);*/

/*		float values[] = {
0,100,0, 0, 0,
0,100,-100, 0, 1,
200,100,0, 1, 0,
200,100,-100, 1, 1,
		};

		unsigned short indices[] = {
			0, 1, 2, 1, 2, 3,
		};

		GPU_TriangleBatch(sprite, App::get_surface_target(), 4, values, 6, indices, GPU_BATCH_XYZ_ST);
		GPU_FlushBlitBuffer();*/

Player::Player()
{
	sprite.load_from_resource(Resource::get("SPRITE"));

	sprite_rect = { 0,0,48,48 };
	velocity = { 0,0 };
	animation.play(wait_animation);
}

void Player::update(float delta_time)
{
	std::shared_ptr<Level> scene = std::dynamic_pointer_cast<Level>(App::get_current_scene());

	acceleration.x = 0;
	if (Input::is_held(Input::LEFT))
	{
		flip_x = true;
		acceleration.x = -0.05 * 60;
	}
	if (Input::is_held(Input::RIGHT))
	{
		flip_x = false;
		acceleration.x = 0.05 * 60;
	}

	if (velocity.x >= 3 * 60)
	{
		acceleration.x = 0;
		velocity.x = 3 * 60;
	}
	if (velocity.x <= -3 * 60)
	{
		acceleration.x = 0;
		velocity.x = -3 * 60;
	}

	if (!Input::is_held(Input::LEFT) &&
		!Input::is_held(Input::RIGHT))
	{
		if (velocity.x > 0.1) acceleration.x = -0.05 * 60;
		else if (velocity.x < -0.1) acceleration.x = 0.05 * 60;
		else
		{
			acceleration.x = 0;
			velocity.x = 0;
		}
	}

	switch (state)
	{
		case State::STAND:
			push_out(scene);
			if (Input::is_held(Input::LEFT))
			{
				animation.play(walk_animation);
			}
			if (Input::is_held(Input::RIGHT))
			{
				animation.play(walk_animation);
			}

			if (abs(velocity.x) < 0.04 * 60 && !animation.is_playing(wait_animation))
				animation.play(wait_animation);

			if (Input::is_pressed(InputActions::JUMP))
			{
				state = State::JUMP;
				velocity.y = -3 * 60;
				acceleration.x = 0;
				animation.play(jump_animation);

				jump_sfx.play();
			}
			if (!scene->tileset.check_collision({ position.x,
				position.y + velocity.y * delta_time + 24 }))
			{
				state = State::JUMP;
				acceleration.x = 0;
			}
			break;
		case State::JUMP:
			acceleration.y = 0.1 * 60;
			if (velocity.y > 0 && scene->tileset.check_collision({ position.x,
				position.y + velocity.y * delta_time + 23 }))
			{
				velocity.y = 0;
				position.y = position.y + velocity.y * delta_time;
				push_out(scene);
				state = State::STAND;
				acceleration.y = 0;
			}
			break;
	}

	SpriteObject::update(delta_time);

	// Log::info(velocity);
}

void Player::render(float delta_time)
{
	SpriteObject::render(delta_time);
}

void Player::push_out(std::shared_ptr<Level>& scene)
{
	while (scene->tileset.check_collision(position + Vector2{ 0, 23 }))
	{
		position.y--;
	}
}

static TileMapObject::Tile level_layout[] = {
	0,0,0,0,0,0,0,0,0,0,
	1,2,0,0,0,0,0,0,0,0,
	0,0,1,1,1,1,1,1,1,1,
};

enum Layers
{
	LAYER_MAIN,
	LAYER_HUD,
};

void Level::init()
{
	Fade::in(1);
	get_current_renderer().set_bg_color(Colors::GRAY);

	add_layer(LAYER_MAIN);
	add_layer(LAYER_HUD, { false });

	tileset.load_image(Resource::get("TILES"));
	tileset.load_layout(10, 3, level_layout);
	tileset.get_tile_collision(1).add_shape(
		Utils::make_unique<Collision::Polygon>(std::vector<Vector2>{
			{ 0, 32 },
			{ 40, 37 },
			{ 90, 37 },
			{ 128, 32 },
			{ 128, 128 },
			{ 0, 128 },
	}));
	tileset.get_tile_collision(2).add_shape(
		Utils::make_unique<Collision::Polygon>(std::vector<Vector2>{
			{ 0, 33 },
			{ 56, 18 },
			{ 105, 5 },
			{ 128, 5 },
			{ 128, 128 },
			{ 0, 128 },
	}));

	player.position = { 150, 100 };
	Camera::get().position = player.position;

	std::shared_ptr<FontTTF> font = Utils::make_shared<FontTTF>("times.ttf", 14);
	{
		std::shared_ptr<TextObjectUnicode> text = create_object<TextObjectUnicode>(
			LAYER_HUD, font, Language::get_text("test_level"));
		text->position = { text->get_sprite().get_width() / 2.f + 5,
						text->get_sprite().get_height() / 2.f + 5 };
	}

	{
		std::shared_ptr<TextObjectUnicode> text = create_object<TextObjectUnicode>(
			LAYER_HUD, font, Language::get_text("by_nintorch"));
		text->position = { text->get_sprite().get_width() / 2.f + 5,
						get_current_renderer().get_surface_size().y - text->get_sprite().get_height() / 2.f - 5 };
	}

	add_object_unmanaged(tileset, LAYER_MAIN);
	add_object_unmanaged(player, LAYER_MAIN);

	bg_texture.load_from_file("background.png");

	float ycoef = 0.1;
	{
		BackgroundHandler::Part part{ bg_texture };
		part.texture_rect = { 11, 10, 420, 144 };
		part.pos = part.texture_rect.get_size() / 2;
		part.xcoef = 0;
		part.ycoef = ycoef;
		part.xwrap = true;
		bg.add_part(part);
	}
	{
		BackgroundHandler::Part part{ bg_texture };
		part.texture_rect = { 55, 237, 97, 56 };
		part.pos = part.texture_rect.get_size() / 2 + Vector2{0, 80};
		part.xcoef = 0;
		part.ycoef = ycoef;
		bg.add_part(part);
	}
	{
		BackgroundHandler::Part part{ bg_texture };
		part.texture_rect = { 12, 165, 627, 72 };
		part.pos = part.texture_rect.get_size() / 2 + Vector2{0, 20};
		part.xcoef = 0.05f;
		part.ycoef = ycoef;
		part.xwrap = true;
		bg.add_part(part);
	}
	{
		BackgroundHandler::Part part{ bg_texture };
		part.texture_rect = { 11, 316, 512, 48 };
		part.pos = part.texture_rect.get_size() / 2 + Vector2{0, 100};
		part.xcoef = 0.1f;
		part.ycoef = ycoef;
		part.xwrap = true;
		bg.add_part(part);
	}
	{
		BackgroundHandler::Part part{ bg_texture };
		part.texture_rect = { 11, 374, 641, 30 };
		part.pos = part.texture_rect.get_size() / 2 + Vector2{0, 130};
		part.xcoef = 0.15f;
		part.ycoef = ycoef;
		part.xwrap = true;
		bg.add_part(part);
	}

	{
		BackgroundHandler::Part part{ bg_texture };
		part.texture_rect = { 12, 407, 320, 62 };
		part.pos = part.texture_rect.get_size() / 2 + Vector2{300, 120};
		part.xcoef = 0.2f;
		part.ycoef = ycoef;
		part.xwrap = true;
		bg.add_part(part);
	}

	Audio::set_global_volume(0.5f);
	Audio::Music::play(Resource::get("MUSIC"), 0);
}

void Level::returned()
{
	Log::info("Returned to level");
	get_current_renderer().set_bg_color(Colors::GRAY);
}

void Level::update(float delta_time)
{
	if (Input::is_key_held(SDL_SCANCODE_D))
		Camera::get().angle += 2;
	if (Input::is_key_held(SDL_SCANCODE_A))
		Camera::get().angle -= 2;
	if (Input::is_key_held(SDL_SCANCODE_Q))
		Camera::get().zoom += 0.02f;
	if (Input::is_key_held(SDL_SCANCODE_E))
		Camera::get().zoom -= 0.02f;

	if (Input::is_key_pressed(SDL_SCANCODE_F2))
	{
		Audio::Music::fade_out(0.5, true);
		Fade::out(1, [] { App::jump_to<Level>(); });
	}

	if (Input::is_key_held(SDL_SCANCODE_O))
		water_level -= 1;
	else if (Input::is_key_held(SDL_SCANCODE_P))
		water_level += 1;

	Scene::update(delta_time);

	Camera::get().position = player.position;

	if (player.position.y > water_level)
		Audio::set_global_filter(0, Audio::Filters::get_underwater_filter());
	else
		Audio::clear_global_filter(0);

	if (Input::is_key_pressed(SDL_SCANCODE_H))
		get_current_renderer().take_screenshot("test.png");
}

void Level::render(float delta_time)
{
	bg.render(delta_time);
	Scene::render(delta_time);

	auto& render = get_current_renderer();
	auto blend = render.get_primitives_blend_mode();
	render.set_primitives_blend_mode(Renderer::BlendMode::SUBTRACT);
	render.enable_camera(false);
	Vector2 surf_size = render.get_surface_size().convert_to<float>();
	render.rectangle_filled({ 0, water_level - Camera::get().get_up(), surf_size.x, surf_size.y }, { 80, 10, 0, 255 });
	render.enable_camera(true);
	render.set_primitives_blend_mode(blend);
}