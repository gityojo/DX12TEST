#include "Audio.h"

#ifdef _XBOX //Big-Endian
#define fourccRIFF 'RIFF'
#define fourccDATA 'data'
#define fourccFMT 'fmt '
#define fourccWAVE 'WAVE'
#define fourccXWMA 'XWMA'
#define fourccDPDS 'dpds'
#endif

#ifndef _XBOX //Little-Endian
#define fourccRIFF 'FFIR'
#define fourccDATA 'atad'
#define fourccFMT ' tmf'
#define fourccWAVE 'EVAW'
#define fourccXWMA 'AMWX'
#define fourccDPDS 'sdpd'
#endif

HRESULT FindChunk(HANDLE hFile, DWORD fourcc, DWORD& dwChunkSize, DWORD& dwChunkDataPosition)
{
	HRESULT hr = S_OK;
	if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, 0, NULL, FILE_BEGIN))
		return HRESULT_FROM_WIN32(GetLastError());

	DWORD dwChunkType;
	DWORD dwChunkDataSize;
	DWORD dwRIFFDataSize = 0;
	DWORD dwFileType;
	DWORD bytesRead = 0;
	DWORD dwOffset = 0;

	while (hr == S_OK)
	{
		DWORD dwRead;
		if (0 == ReadFile(hFile, &dwChunkType, sizeof(DWORD), &dwRead, NULL))
			hr = HRESULT_FROM_WIN32(GetLastError());

		if (0 == ReadFile(hFile, &dwChunkDataSize, sizeof(DWORD), &dwRead, NULL))
			hr = HRESULT_FROM_WIN32(GetLastError());

		switch (dwChunkType)
		{
		case fourccRIFF:
			dwRIFFDataSize = dwChunkDataSize;
			dwChunkDataSize = 4;
			if (0 == ReadFile(hFile, &dwFileType, sizeof(DWORD), &dwRead, NULL))
				hr = HRESULT_FROM_WIN32(GetLastError());
			break;

		default:
			if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, dwChunkDataSize, NULL, FILE_CURRENT))
				return HRESULT_FROM_WIN32(GetLastError());
		}

		dwOffset += sizeof(DWORD) * 2;

		if (dwChunkType == fourcc)
		{
			dwChunkSize = dwChunkDataSize;
			dwChunkDataPosition = dwOffset;
			return S_OK;
		}

		dwOffset += dwChunkDataSize;

		if (bytesRead >= dwRIFFDataSize) return S_FALSE;

	}

	return S_OK;

}

HRESULT ReadChunkData(HANDLE hFile, void* buffer, DWORD buffersize, DWORD bufferoffset)
{
	HRESULT hr = S_OK;
	if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, bufferoffset, NULL, FILE_BEGIN))
		return HRESULT_FROM_WIN32(GetLastError());
	DWORD dwRead;
	if (0 == ReadFile(hFile, buffer, buffersize, &dwRead, NULL))
		hr = HRESULT_FROM_WIN32(GetLastError());
	return hr;
}

Audio::Audio()
{
	CoInitializeEx(NULL, COINIT_MULTITHREADED);

	IXAudio2* audio2;
	XAudio2Create(&audio2);

	IXAudio2MasteringVoice* masteringVoice;
	audio2->CreateMasteringVoice(&masteringVoice);

	HANDLE file = CreateFile(L"Audio/a.wav", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

	DWORD size, pos;
	FindChunk(file, fourccRIFF, size, pos);

	DWORD type;
	ReadChunkData(file, &type, size, pos);

	if (type != fourccWAVE)
	{
		MessageBox(NULL, L"Not WAVE!", NULL, MB_OK);
		return;
	}

	WAVEFORMATEX wfx = {};
	XAUDIO2_BUFFER buffer = {};

	FindChunk(file, fourccFMT, size, pos);
	ReadChunkData(file, &wfx, size, pos);

	FindChunk(file, fourccDATA, size, pos);
	BYTE* data = new BYTE[size];
	ReadChunkData(file, data, size, pos);

	buffer.Flags = XAUDIO2_END_OF_STREAM;
	buffer.AudioBytes = size;
	buffer.pAudioData = data;
	buffer.LoopCount = XAUDIO2_LOOP_INFINITE;

	audio2->CreateSourceVoice(&mSourceVoice, &wfx);
	mSourceVoice->SubmitSourceBuffer(&buffer);
}

void Audio::Play()
{
	if (mSourceVoice)
	{
		mSourceVoice->Start();
	}
}

void Audio::Pause()
{
	if (mSourceVoice)
	{
		mSourceVoice->Stop();
	}
}
