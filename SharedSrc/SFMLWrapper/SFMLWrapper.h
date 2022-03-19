#pragma once

#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#include <memory>

namespace DOTL
{
	struct SFMLInstance
	{
		SFMLInstance ( int windowWidth , int windowHeight , char const* name );

		void Update ();

	private:
		std::shared_ptr<sf::RenderWindow> window_;
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