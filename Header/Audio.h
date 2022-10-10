#pragma once

#include <xaudio2.h>

class Audio
{
public:
	Audio();
	void Play();
	void Pause();

private:
	IXAudio2SourceVoice* mSourceVoice;
};
