#pragma once
#include <array>
#include <assert.h>
#include <fstream>
#include <list>
#include <string>
#include <unordered_map>
#include <vector>
#include <wrl.h>
#include <xaudio2.h>
#include "FFT.h"

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

	// メインループの更新処理で呼び出す
	void Update();

	// 読み込み
	void LoadWave(const std::string& tag, const std::string& filename);

	// BGM再生
	void PlayBGM(const std::string& tag, float volume = 0.1f);

	// BGM停止
	void StopBGM();

	// SE再生
	void PlaySE(const std::string& tag, float volume = 0.1f);

	// スペクトラムのGetter
	std::vector<float> GetSpectrum() const { return spectrum_; }

private:
	// 内部的な解放処理
	void SoundUnLoad(SoundData* soundData);

private:
	Microsoft::WRL::ComPtr<IXAudio2> xAudio2_;

	// 読み込んだ音声データの格納庫
	std::unordered_map<std::string, SoundData> soundData_;

	// BGM用のボイス
	IXAudio2SourceVoice* bgmVoice_ = nullptr;

	// 再生中のSEボイスリスト
	std::list<IXAudio2SourceVoice*> seVoices_;

	// 現在再生中のBGMのタグ
	std::string currentBgmTag_;

	// FFT
	std::unique_ptr<FFT> fft_;

	// スペクトラム
	std::vector<float> spectrum_;
};