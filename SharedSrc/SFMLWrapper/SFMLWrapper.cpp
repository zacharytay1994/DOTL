#include "SFMLWrapper.h"

namespace DOTL
{
	SFMLInstance::SFMLInstance ( int windowWidth , int windowHeight , char const* name )
		:
		window_ ( std::make_shared<sf::RenderWindow> ( sf::VideoMode ( windowWidth , windowHeight ) , name ) )
	{
	}

	void SFMLInstance::Update ()
	{
		while ( window_->isOpen () )
		{
			sf::Event sf_event;

			while ( window_->pollEvent ( sf_event ) )
			{
				if ( sf_event.type == sf::Event::Closed )
				{
					window_->close ();
				}
			}

			window_->clear ();
			window_->display ();
		}

		return;
	}

	SFMLSprite::SFMLSprite ( char const* name )
	{
		texture_.loadFromFile ( name );
		texture_.setSmooth ( true );

		sprite_.setTexture ( texture_ );
	}

	void SFMLSprite::SetPosition ( float x , float y )
	{
		sprite_.setPosition ( sf::Vector2f ( x , y ) );
	}

	void SFMLSprite::SetRotation ( float deg )
	{
		sprite_.setRotation ( deg );
	}

	void SFMLSprite::SetScale ( float x , float y )
	{
		sprite_.setScale ( sf::Vector2f ( x , y ) );
	}
}