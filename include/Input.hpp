#ifndef __INPUT_H__
#define __INPUT_H__

#include "Vector2.hpp"
#include "Platform.hpp"

#if SE_SDL2
#include "SDL.h"
#endif

#include <functional>

namespace Sputnik
{
	namespace Input
	{
// A type used to represent an input action or button in the game engine.
// Only 256 input actions are available.
		using InputAction = uint8_t;

#if SE_WINDOWS
		using PhysicalKey = SDL_Scancode;
		constexpr int PhysicalKeyCount = SDL_NUM_SCANCODES;
#endif

	// An enumeration for basic in-game actions
		enum Actions : InputAction
		{
			UP, DOWN, LEFT, RIGHT,

			CUSTOM_ACTION // Start custom actions with this member
		};

		/*--------------------------------
			Joystick API
			TODO: finish joystick api
		--------------------------------*/

		class Joystick
		{
		public:
			using Axis = uint8_t;
			using Button = uint8_t;
			using ID = SDL_JoystickID;
		};

		enum InputAxis : Joystick::Axis
		{
			AXIS_HORIZONTAL,
			AXIS_VERTICAL,
		};

		/*--------------------------------
			Mouse API
		--------------------------------*/

		namespace Mouse
		{
			enum class Button : char
			{
				LEFT, MIDDLE, RIGHT
			};

			bool is_pressed(Button btn);
			bool is_held(Button btn);
			void set_pressed(Button btn, bool flag);

			// Get mouse position in playground coordinates
			Vector2Int get_playfield_position();
			// Get mouse position inside the game window
			Vector2Int get_window_position();

			void reset();
		};

		// TODO: clear key bind and clear hotkey
		// Keybinds from keyboard buttons to game input action

		InputAction get_key_bind(PhysicalKey key);
		void set_key_bind(PhysicalKey key, InputAction action);
		bool has_key_bind(PhysicalKey key);

		// Hotkeys (do actions when keyboard buttons are pressed)

		const std::function<void()>& get_hotkey(PhysicalKey key);
		void set_hotkey(PhysicalKey key, std::function<void()> action);
		bool has_hotkey(PhysicalKey key);

		/* Input actions related functions */

		bool is_pressed(InputAction action);
		bool is_held(InputAction action);
		void set_pressed(InputAction action, bool flag);
		void set_held(InputAction action, bool flag);

		// Resets the pressed flag for all input actions
		void reset_action_pressed();
		// Resets the held flag for all input actions
		void reset_action_held();

		/* Physical keys related functions */

		bool is_key_pressed(PhysicalKey key);
		bool is_key_held(PhysicalKey key);
		// Sets the pressed flag for the specified physical key to the specified value
		void set_key_pressed(PhysicalKey key, bool flag);
		// Sets the held flag for the specified physical key to the specified value
		void set_key_held(PhysicalKey key, bool flag);
		// Resets the pressed flag for all physical keys
		void reset_key_pressed();
		// Resets the held flag for all physical keys
		void reset_key_held();
	}
}

#endif // __INPUT_H__