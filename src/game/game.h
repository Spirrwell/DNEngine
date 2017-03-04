#ifndef GAME_H
#define GAME_H

#include "igame.h"

class Game : public IGame
{
public:
	Game( const std::string &title );
	~Game();

	std::string GetTitle() { return m_szTitle; }
	void SetTitle( const std::string &title );

protected:
	void Update();

private:
	std::string m_szTitle;
};

#endif