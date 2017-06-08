#include "engine_handler.h"

EngineHandler::EngineHandler()
{
	m_pEngine = nullptr;
	m_bInitialized = false;
}

EngineHandler::~EngineHandler()
{
	if ( m_pEngine != nullptr )
		delete m_pEngine;
}

EngineHandler::ERETURNCODE EngineHandler::InitalizeEngine( IGame *game )
{
	if ( m_bInitialized )
		return ENGINE_INIT_ALREADY_INITIALIZED;

	m_pEngine = new Engine;

	// Game is required for initialization and appropriate deletion
	m_pEngine->SetGame( game );
	m_bInitialized = m_pEngine->Initialize();

	if ( !m_bInitialized )
	{
		delete m_pEngine;
		m_pEngine = nullptr;

		return ENGINE_INIT_FAIL;
	}

	return ENGINE_OK;
}

void EngineHandler::RunEngine()
{
	m_pEngine->RunEngine();
}

EngineHandler g_DNEngineHandler;