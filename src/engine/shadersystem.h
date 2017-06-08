#ifndef SHADERSYSTEM_H
#define SHADERSYSTEM_H

#include "ishader.h"

#include <vector>

class ShaderSystem
{
	friend class BaseShader;

public:
	ShaderSystem();
	virtual ~ShaderSystem();

protected:
	void AddShader( IShader *pShader );
	void RemoveShader( IShader *pShader );

private:
	std::vector< IShader* > m_pShaders;
};

extern ShaderSystem *GetShaderSystem();

#endif