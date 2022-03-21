#pragma once

#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#include "../GameData/GameData.h"

#include <memory>

namespace DOTL
{
	struct SFMLInstance
	{
		SFMLInstance ( int windowWidth , int windowHeight , char const* name );

		void Update ( GameData const& data , std::string const& username );

		bool IsOpen ();

	private:
		std::shared_ptr<sf::RenderWindow> window_;
		sf::Font font_;
		sf::Text text_;

		static constexpr int font_size_ { 18 };
		static constexpr int font_offset_ { 25 };
	};

	struct SFMLSprite
	{
		SFMLSprite ( char const* name );

		void SetPosition ( float x , float y );

		void SetRotation ( float deg );

		void SetScale ( float x , float y );

	private:
		sf::Texture texture_;
		sf::Sprite sprite_;
	};
}