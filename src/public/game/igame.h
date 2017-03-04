#ifndef IGAME_H
#define IGAME_H

#include <string>

class IGame
{
	friend class Engine;
public:
	//Game Window Title
	virtual std::string GetTitle() = 0;
	virtual void SetTitle( const std::string &title ) = 0;

protected:
	virtual void Update() = 0;
};

#endif