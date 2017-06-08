#ifndef BASESHADER_H
#define BASESHADER_H

#include "ishader.h"
#include "shadersystem.h"

class BaseShader : public IShader
{
public:
	BaseShader( const std::string &name );
	virtual ~BaseShader();

	inline const std::string &GetShaderName() { return m_szShaderName; };

protected:
	std::string m_szShaderName;
};

#endif