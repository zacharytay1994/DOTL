#pragma once

#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#include "../GameData/GameData.h"

#include <memory>

namespace DOTL
{
	struct MouseEventData
	{
		bool pressed_{ false };
		float x_, y_;
		float click_radius_{ 40.0f };
	};

	struct SFMLSprite
	{
		SFMLSprite( char const* name );

		void SetPosition( float x, float y );

		void SetScale( float x, float y );

		sf::Sprite& GetSprite();

		//sf::Texture GetTexture();

	private:
		sf::Texture texture_;
		sf::Sprite sprite_;
	};

	struct SFMLInstance
	{
		SFMLInstance( int windowWidth, int windowHeight, char const* name );

		void PollMouseEvents( MouseEventData& med );

		void Update( GameData& data, float dt, uint16_t playerID, uint16_t targetID );

		void Draw();

		bool IsOpen();

	private:
		std::shared_ptr<sf::RenderWindow> window_;
		sf::Font font_;
		sf::Text text_;

		SFMLSprite background_{ "Assets/Arts/Background_v1_20220403.png" };
		SFMLSprite tower_pink_{ "Assets/Arts/Tower_Pink_v1_20220403.png" };
		SFMLSprite tower_blue_{ "Assets/Arts/Tower_Blue_v1_20220403.png" };
		SFMLSprite player_pink_{ "Assets/Arts/Player_Pink_v1_20220403.png" };
		SFMLSprite player_blue_{ "Assets/Arts/Player_Blue_v1_20220403.png" };
		SFMLSprite minion_pink_{ "Assets/Arts/Minion_Pink_v1_20220403.png" };
		SFMLSprite minion_blue_{ "Assets/Arts/Minion_Blue_v1_20220403.png" };
		SFMLSprite bullet_pink_{ "Assets/Arts/Bullet_Pink_v1_20220403.png" };
		SFMLSprite bullet_blue_{ "Assets/Arts/Bullet_Blue_v1_20220403.png" };

		static constexpr int font_size_{ 18 };
		static constexpr int font_offset_{ 25 };
	};
}