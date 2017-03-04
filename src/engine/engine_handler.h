#ifndef ENGINE_HANDLER_H
#define ENGINE_HANDLER_H

#include "engine.h"

class EngineHandler
{
public:
	EngineHandler();
	~EngineHandler();

	//Will expand this in the future
	enum ERETURNCODE {
		ENGINE_OK = 0,
		ENGINE_INIT_ALREADY_INITIALIZED = 1,
		ENGINE_INIT_FAIL = 2,
	};

	ERETURNCODE InitalizeEngine( IGame *game );

	void RunEngine();

	IEngine *GetEngine() { return m_pEngine; };

private:
	IEngine *m_pEngine;
	bool m_bInitialized;
};

extern EngineHandler g_DNEngineHandler;

#endif