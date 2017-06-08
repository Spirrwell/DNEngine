#include "baseshader.h"

BaseShader::BaseShader( const std::string &name ) : m_szShaderName( name )
{
	GetShaderSystem()->AddShader( this );
}

BaseShader::~BaseShader()
{
	GetShaderSystem()->RemoveShader( this );
}