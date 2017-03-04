#ifndef SOUNDSYSTEM_H
#define SOUNDSYSTEM_H

#include "fmod.hpp"

class SoundSystem {
public:
	SoundSystem();
	~SoundSystem();

	FMOD::System *GetSystem() { return m_pSystem; }
	void Update() { m_pSystem->update(); }

	void CheckError( FMOD_RESULT result );

private:
	void Setup();

	FMOD::System *m_pSystem;
	unsigned int m_iVersion;
	int m_iNumDrivers;
};

SoundSystem *GetSoundSystem();

#endif