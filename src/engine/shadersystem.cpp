#include "shadersystem.h"
#include <cassert>

ShaderSystem::ShaderSystem()
{

}

ShaderSystem::~ShaderSystem()
{

}

void ShaderSystem::AddShader( IShader *pShader )
{
	m_pShaders.push_back( pShader );
}

void ShaderSystem::RemoveShader( IShader *pShader )
{
	assert( pShader != nullptr );

	for ( unsigned int i = 0; i < m_pShaders.size(); i++ )
	{
		if ( pShader == m_pShaders[i] )
		{
			m_pShaders.erase( m_pShaders.begin() + i );
			return;
		}
	}
}

static ShaderSystem s_ShaderSystem;
ShaderSystem *GetShaderSystem() { return &s_ShaderSystem; }