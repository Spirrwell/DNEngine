#ifndef ISHADER_H
#define ISHADER_H

#include <string>

class IShader
{
	virtual const std::string &GetShaderName() = 0;
};

#endif