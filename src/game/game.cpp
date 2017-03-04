#include "game.h"
#include "engine/enginemain.h"

Game::Game( const std::string &title )
{
	m_szTitle = title;
}

Game::~Game()
{

}

void Game::SetTitle( const std::string &title )
{
	m_szTitle = title;
	GetEngine()->UpdateTitle( title );
}

void Game::Update()
{

}