#include "Input.hpp"
#include "Utils.hpp"

#include <unordered_map>
#include <bitset>
#include <algorithm>

namespace Sputnik
{
	namespace Input
	{
		namespace Mouse
		{
			static char mouse_flags;

			bool is_pressed(Button btn)
			{
				switch (btn)
				{
					case Button::LEFT:
						return mouse_flags & 1;
					case Button::MIDDLE:
						return mouse_flags & 2;
					case Button::RIGHT:
						return mouse_flags & 4;
					default:
						return false;
				}
			}

			bool is_held(Button btn)
			{
				if (btn < Button::LEFT || btn > Button::RIGHT)
					return false;

				auto state = SDL_GetMouseState(nullptr, nullptr);
				return state & SDL_BUTTON((int)btn);
			}

			void set_pressed(Button btn, bool flag)
			{
				switch (btn)
				{
					case Button::LEFT:
					case Button::MIDDLE:
					case Button::RIGHT:
						mouse_flags |= 1 << (int)btn;
						break;
				}
			}

			Vector2Int get_playfield_position()
			{
				// TODO: Renderer API for this
				return {};
			}

			Vector2Int get_window_position()
			{
				Vector2Int v;
				SDL_GetMouseState(&v.x, &v.y);
				return v;
			}

			void reset()
			{
				mouse_flags = 0;
			}
		}

		std::unordered_map<PhysicalKey, InputAction> key_binds;

		InputAction get_key_bind(PhysicalKey key)
		{
			return key_binds.at(key);
		}

		void set_key_bind(PhysicalKey key, InputAction action)
		{
			key_binds[key] = action;
		}

		bool has_key_bind(PhysicalKey key)
		{
			auto it = key_binds.find(key);
			return it != key_binds.end();
		}

		std::unordered_map<PhysicalKey, std::function<void()>> hotkeys;

		const std::function<void()>& get_hotkey(PhysicalKey key)
		{
			return hotkeys.at(key);
		}

		void set_hotkey(PhysicalKey key, std::function<void()> action)
		{
			hotkeys[key] = std::move(action);
		}

		bool has_hotkey(PhysicalKey key)
		{
			auto it = hotkeys.find(key);
			return it != hotkeys.end();
		}

		static std::bitset<256> action_held;
		static std::bitset<256> action_pressed;
		static std::bitset<PhysicalKeyCount> key_pressed;
		static std::bitset<PhysicalKeyCount> key_held;

		bool is_pressed(InputAction action)
		{
			return action_pressed[action];
		}

		bool is_held(InputAction action)
		{
			return action_held[action];
		}

		void set_pressed(InputAction action, bool flag)
		{
			action_pressed[action] = flag;
		}

		void set_held(InputAction action, bool flag)
		{
			action_held[action] = flag;
		}

		void reset_action_pressed()
		{
			action_pressed.reset();
		}

		void reset_action_held()
		{
			action_held.reset();
		}

		bool is_key_pressed(PhysicalKey key)
		{
			return key_pressed[key];
		}

		bool is_key_held(PhysicalKey key)
		{
			return SDL_GetKeyboardState(nullptr)[key];
		}

		void set_key_pressed(PhysicalKey key, bool flag)
		{
			key_pressed[key] = flag;
		}

		void set_key_held(PhysicalKey key, bool flag)
		{
			key_held[key] = flag;
		}

		void reset_key_pressed()
		{
			key_pressed.reset();
		}

		void reset_key_held()
		{
			key_held.reset();
		}
	}
}