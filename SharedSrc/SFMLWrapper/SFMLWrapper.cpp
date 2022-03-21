#include "SFMLWrapper.h"

#include <iostream>

namespace DOTL
{
	SFMLInstance::SFMLInstance ( int windowWidth , int windowHeight , char const* name )
		:
		window_ ( std::make_shared<sf::RenderWindow> ( sf::VideoMode ( windowWidth , windowHeight ) , name ) )
	{
		if ( !font_.loadFromFile ( "Assets/Fonts/arial.ttf" ) )
		{
			std::cerr << "DOTL::SFMLInstance [FILE: SFMLWrapper.cpp] - Unable to load font." << std::endl;
		}
		else
		{
			text_.setFont ( font_ );
			text_.setCharacterSize ( font_size_ );
		}
	}

	void SFMLInstance::Update ( GameData const& data , std::string const& username )
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

		// draw all entities as circles
		sf::CircleShape shape ( 10 );
		sf::Vector2f position;
		for ( auto const& d : data.entities_ )
		{
			// id 0 is reserved for inactive entities
			if ( d.id_ != 0 )
			{
				position = sf::Vector2f ( d.GetData ( ED::POS_X ) , d.GetData ( ED::POS_Y ) );
				shape.setPosition ( position );
				window_->draw ( shape );
				position.y += font_offset_;
				text_.setPosition ( position );
				switch ( d.type_ )
				{
				case ( ET::MINION ):

					text_.setFillColor ( sf::Color::White );
					text_.setString ( "Minion" );
					break;
				case ( ET::TOWER ):

					text_.setFillColor ( sf::Color::White );
					text_.setString ( "Tower" );
					break;
				case ( ET::PLAYER ):

					text_.setFillColor ( sf::Color::White );
					if ( data.player_names_.find ( d.id_ ) != data.player_names_.end () )
					{
						text_.setString ( data.player_names_.at ( d.id_ ) );
					}
					else
					{
						text_.setString ( "UNKNOWN PLAYER" );
					}
					break;
				}
				window_->draw ( text_ );
			}
		}

		window_->display ();

		return;
	}

	bool SFMLInstance::IsOpen ()
	{
		return window_->isOpen ();
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