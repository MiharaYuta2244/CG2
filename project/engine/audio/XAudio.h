#pragma once
#include <array>
#include <assert.h>
#include <fstream>
#include <vector>
#include <wrl.h>
#include <xaudio2.h>

#pragma comment(lib, "xaudio2.lib")

// 音声データ
struct SoundData {
	// 波形フォーマット
	WAVEFORMATEXTENSIBLE wfex;
	// バッファ
	std::vector<BYTE> buffer;
};

class XAudio {
public:
	~XAudio();
	void Initialize();
	void SoundsAllLoad(const std::string& filename);
	void SoundPlayWave();

private:
	void SoundLoadFile(const std::string& filename);
	void SoundUnLoad(SoundData* soundData);

private:
	Microsoft::WRL::ComPtr<IXAudio2> xAudio2_;
	SoundData soundData_;
};