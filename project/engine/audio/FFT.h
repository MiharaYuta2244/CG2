#pragma once
#include <cmath>
#include <complex>
#include <vector>
#include <unordered_map>
//#include <xaudio2.h>

struct SoundData;
struct IXAudio2SourceVoice;
class XAudio;

class FFT {
public:
	void GetSpectrum(std::vector<float>& output, IXAudio2SourceVoice& bgmVoice, const std::string& currentBgmTag, std::unordered_map<std::string, SoundData>& soundData);
};
