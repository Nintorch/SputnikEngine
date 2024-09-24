#ifndef __GAMEMAIN_H__
#define __GAMEMAIN_H__

#include <string>
#include "Vector2.hpp"

namespace Sputnik
{
	void game_init();

	void game_update(float delta_time);
	void game_render(float delta_time);

	void game_quit();
}
#endif // __GAMEMAIN_H__