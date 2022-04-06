#include "SFMLWrapper.h"

#include <iostream>
#include <cmath>

namespace DOTL
{
	SFMLInstance::SFMLInstance( int windowWidth, int windowHeight, char const* name )
		:
		window_( std::make_shared<sf::RenderWindow>( sf::VideoMode( windowWidth, windowHeight ), name ) )
	{
		if ( !font_.loadFromFile( "Assets/Fonts/arial.ttf" ) )
		{
			std::cerr << "DOTL::SFMLInstance [FILE: SFMLWrapper.cpp] - Unable to load font." << std::endl;
		}
		else
		{
			text_.setFont( font_ );
			text_.setCharacterSize( font_size_ );
		}

		// init background ( the environment )
		background_.SetPosition( 0.0f, 0.0f );
	}

	void SFMLInstance::PollMouseEvents( MouseEventData& med )
	{
		sf::Event sf_event;

		med.pressed_ = false;

		while ( window_->pollEvent( sf_event ) )
		{
			if ( sf_event.type == sf::Event::Closed )
			{
				window_->close();
			}

			if ( sf_event.type == sf::Event::MouseButtonPressed )
			{
				if ( sf_event.mouseButton.button == sf::Mouse::Left )
				{
					med.pressed_ = true;
					med.x_ = static_cast< float >( sf_event.mouseButton.x );
					med.y_ = static_cast< float >( sf_event.mouseButton.y );
				}
			}
		}
	}

	float my_lerp( float x, float y, float val )
	{
		return static_cast< float >( x + ( ( y - x ) * val ) );
	}

	void SFMLInstance::Update( GameData& data, float dt, uint16_t playerID, uint16_t targetID )
	{
		window_->clear();

		// draw background 
		window_->draw( background_.GetSprite() );

		float bar_x{ 40.0f };
		float bar_y{ 5.0f };

		sf::CircleShape selected_outline( 10 );
		selected_outline.setOrigin( selected_outline.getRadius(), selected_outline.getRadius() );
		selected_outline.setFillColor( sf::Color::Transparent );
		selected_outline.setOutlineThickness( 3 );

		sf::RectangleShape hp_bar_border( sf::Vector2f( bar_x, bar_y ) );
		sf::RectangleShape hp_bar_inner( sf::Vector2f( bar_x, bar_y ) );
		hp_bar_border.setOutlineThickness( 1 );
		hp_bar_border.setFillColor( sf::Color::Transparent );
		hp_bar_inner.setFillColor( sf::Color::Green );

		sf::Vector2f position;
		for ( auto& extended_entity : data.entities_ )
		{
			if ( extended_entity.entity_.id_ == 0 )
			{
				continue;
			}
			// id 0 is reserved for inactive entities
			if ( extended_entity.entity_.active_ )
			{
				if ( extended_entity.entity_.id_ != playerID )
				{
					// perform entity interpolation here
					// ...
					float lerp_val = static_cast< float >( data.sync_delta_time_ > 1.0f ? 1.0f : data.sync_delta_time_ );
					extended_entity.interpolated_x = my_lerp( extended_entity.interpolated_x, extended_entity.entity_.GetData( ED::POS_X ), lerp_val * dt * 50 );
					extended_entity.interpolated_y = my_lerp( extended_entity.interpolated_y, extended_entity.entity_.GetData( ED::POS_Y ), lerp_val * dt * 50 );
					position = sf::Vector2f( extended_entity.interpolated_x, extended_entity.interpolated_y );
				}
				else
				{
					position = sf::Vector2f( extended_entity.entity_.GetData( ED::POS_X ), extended_entity.entity_.GetData( ED::POS_Y ) );
				}

				// draw entity for team 1 
				if ( extended_entity.entity_.team_1_ )
				{
					switch ( extended_entity.entity_.type_ )
					{
					case ( ET::MINION ):
						{
							//draw pink minion
							minion_pink_.SetPosition( position.x, position.y );
							minion_pink_.SetScale( 0.75f, 0.75f );
							minion_pink_.GetSprite().setOrigin( sf::Vector2f(
								static_cast< float >( minion_pink_.GetSprite().getTexture()->getSize().x ) / 2.0f,
								static_cast< float >( minion_pink_.GetSprite().getTexture()->getSize().y ) / 2.0f ) );
							window_->draw( minion_pink_.GetSprite() );
							break;
						}
					case ( ET::TOWER ):
						{
							// draw pink tower
							tower_pink_.SetPosition( position.x, position.y );
							tower_pink_.GetSprite().setOrigin( sf::Vector2f(
								static_cast< float >( tower_pink_.GetSprite().getTexture()->getSize().x ) / 2.0f,
								static_cast< float >( tower_pink_.GetSprite().getTexture()->getSize().y ) / 2.0f ) );
							window_->draw( tower_pink_.GetSprite() );
							break;
						}
					case ( ET::PLAYER ):
						{
							// draw pink player
							player_pink_.SetPosition( position.x, position.y );
							player_pink_.GetSprite().setOrigin( sf::Vector2f( static_cast< float >( player_pink_.GetSprite().getTexture()->getSize().x ) / 2.0f,
																static_cast< float >( player_pink_.GetSprite().getTexture()->getSize().y ) / 2.0f ) );
							window_->draw( player_pink_.GetSprite() );
							break;
						}
					case ( ET::BULLET ):
						{
							// draw pink bullet
							bullet_pink_.SetPosition( position.x, position.y );
							bullet_pink_.SetScale( 0.5f, 0.5f );
							bullet_pink_.GetSprite().setOrigin( sf::Vector2f(
								static_cast< float >( bullet_pink_.GetSprite().getTexture()->getSize().x ) / 2.0f,
								static_cast< float >( bullet_pink_.GetSprite().getTexture()->getSize().y ) / 2.0f ) );
							window_->draw( bullet_pink_.GetSprite() );
							break;
						}
					}
				}
				else // draw entity for team 2
				{
					switch ( extended_entity.entity_.type_ )
					{
					case ( ET::MINION ):
						{
							//draw blue minion
							minion_blue_.SetPosition( position.x, position.y );
							minion_blue_.SetScale( 0.75f, 0.75f );
							minion_blue_.GetSprite().setOrigin( sf::Vector2f(
								static_cast< float >( minion_blue_.GetSprite().getTexture()->getSize().x ) / 2.0f,
								static_cast< float >( minion_blue_.GetSprite().getTexture()->getSize().y ) / 2.0f ) );
							window_->draw( minion_blue_.GetSprite() );
							break;
						}
					case ( ET::TOWER ):
						{
							// draw blue tower
							tower_blue_.SetPosition( position.x, position.y );
							tower_blue_.GetSprite().setOrigin( sf::Vector2f(
								static_cast< float >( tower_blue_.GetSprite().getTexture()->getSize().x ) / 2.0f,
								static_cast< float >( tower_blue_.GetSprite().getTexture()->getSize().y ) / 2.0f ) );
							window_->draw( tower_blue_.GetSprite() );
							break;
						}
					case ( ET::PLAYER ):
						{
							// draw blue player
							player_blue_.SetPosition( position.x, position.y );
							player_blue_.GetSprite().setOrigin( sf::Vector2f(
								static_cast< float >( player_blue_.GetSprite().getTexture()->getSize().x ) / 2.0f,
								static_cast< float >( player_blue_.GetSprite().getTexture()->getSize().y ) / 2.0f ) );
							window_->draw( player_blue_.GetSprite() );
							break;
						}
					case ( ET::BULLET ):
						{
							// draw blue bullet
							bullet_blue_.SetPosition( position.x, position.y );
							bullet_blue_.SetScale( 0.5f, 0.5f );
							bullet_blue_.GetSprite().setOrigin( sf::Vector2f(
								static_cast< float >( bullet_blue_.GetSprite().getTexture()->getSize().x ) / 2.0f,
								static_cast< float >( bullet_blue_.GetSprite().getTexture()->getSize().y ) / 2.0f ) );
							window_->draw( bullet_blue_.GetSprite() );
						}
					}
				}

				// when object is selected , draw an outline
				if ( extended_entity.entity_.id_ == targetID )
				{
					selected_outline.setPosition( position );
					window_->draw( selected_outline );
				}

				// draw name
				position.y += font_offset_;

				switch ( extended_entity.entity_.type_ )
				{
				case ( ET::MINION ):
					{
						text_.setString( "Minion" );
						break;
					}
				case ( ET::TOWER ):
					{
						text_.setString( "Tower" );
						break;
					}
				case ( ET::BULLET ):
					{
						text_.setString( "Bullet" );
						break;
					}
				case ( ET::PLAYER ):
					{
						if ( data.player_names_.find( extended_entity.entity_.id_ ) != data.player_names_.end() )
						{
							text_.setString( data.player_names_.at( extended_entity.entity_.id_ ) );
						}
						else
						{
							text_.setString( "UNKNOWN PLAYER" );
						}
						break;
					}
				}

				// set text colour for different team
				if ( extended_entity.entity_.team_1_ )
				{
					text_.setFillColor( sf::Color( 255, 182, 213, 225 ) ); // pastel pink colour
				}
				else
				{
					text_.setFillColor( sf::Color( 167, 199, 255, 255 ) ); // pastel blue colour
				}

				if ( extended_entity.entity_.type_ != ET::BULLET )
				{
					// setting the string to display, you can use getLocalBounds() to get the bounding rect 
					sf::FloatRect bounds = text_.getLocalBounds();
					text_.setPosition( position.x - ( bounds.width / 2.0f ), position.y + 20.0f );
					window_->draw( text_ );

					// draw bar (JM: added magic number 3.5 to increse the height of the health bar)
					position.y -= 3.5f * font_offset_;
					hp_bar_border.setPosition( position );
					window_->draw( hp_bar_border );

					// calculate inner hp bar
					float hp_ratio = static_cast< float >( extended_entity.entity_.health_ ) / static_cast< float >( extended_entity.entity_.max_health_ );
					hp_bar_inner.setPosition( position );
					hp_bar_inner.setSize( sf::Vector2f( hp_ratio * bar_x, bar_y ) );
					window_->draw( hp_bar_inner );
				}
			}
		}
		window_->display();
		return;
	}

	void SFMLInstance::Draw()
	{}

	bool SFMLInstance::IsOpen()
	{
		return window_->isOpen();
	}

	SFMLSprite::SFMLSprite( char const* name )
	{
		if ( !texture_.loadFromFile( name ) )
		{
			std::cerr << "DOTL::SFMLSprite [FILE: SFMLWrapper.cpp] - Unable to load texture" << name << " ." << std::endl;
		}
		else
		{
			texture_.setSmooth( true );
			sprite_.setTexture( texture_ );
		}
	}

	void SFMLSprite::SetPosition( float x, float y )
	{
		sprite_.setPosition( sf::Vector2f( x, y ) );
	}

	void SFMLSprite::SetScale( float x, float y )
	{
		sprite_.setScale( sf::Vector2f( x, y ) );
	}

	sf::Sprite& SFMLSprite::GetSprite()
	{
		return sprite_;
	}
}