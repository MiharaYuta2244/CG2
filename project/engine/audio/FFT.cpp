#include "FFT.h"
#include "XAudio.h"

void FFT::GetSpectrum(std::vector<float>& output, IXAudio2SourceVoice& bgmVoice, const std::string& currentBgmTag, std::unordered_map<std::string, SoundData>& soundData) {
	// データサイズ定義
	const size_t fftSize = 128;
	output.assign(fftSize / 2, 0.0f); // 出力は半分で良い

	// BGMが再生されていない、またはデータが無い場合は終了
	if (currentBgmTag.empty() || soundData.find(currentBgmTag) == soundData.end()) {
		return;
	}

	SoundData& data = soundData[currentBgmTag];

	// 現在の再生位置を取得
	XAUDIO2_VOICE_STATE state;
	bgmVoice.GetState(&state);

	// バッファ情報
	const WAVEFORMATEXTENSIBLE& wfex = data.wfex;
	const int channels = wfex.Format.nChannels;
	const int bytesPerSample = wfex.Format.wBitsPerSample / 8;
	const int blockAlign = wfex.Format.nBlockAlign; // 1フレームあたりのバイト数

	// 全サンプル数（フレーム数）
	size_t totalSamples = data.buffer.size() / blockAlign;

	// 現在の再生サンプル位置
	size_t currentSampleIndex = state.SamplesPlayed % totalSamples;

	// 波形データの抽出 (FFT用入力バッファ)
	std::vector<std::complex<float>> fftBuffer(fftSize);

	// バッファからコピー
	for (size_t i = 0; i < fftSize; ++i) {
		size_t index = (currentSampleIndex + i) % totalSamples; // ループ対応
		size_t byteOffset = index * blockAlign;

		// データ読み取り
		int16_t* pSample = reinterpret_cast<int16_t*>(&data.buffer[byteOffset]);

		// ステレオの場合はL+Rを平均化してモノラルにする
		float sampleValue = 0.0f;
		if (channels >= 2) {
			sampleValue = (static_cast<float>(pSample[0]) + static_cast<float>(pSample[1])) / 2.0f;
		} else {
			sampleValue = static_cast<float>(pSample[0]);
		}

		// 正規化
		sampleValue /= 32768.0f;

		// 窓関数
		float window = 0.5f * (1.0f - cosf(2.0f * 3.14159265f * i / (fftSize - 1)));

		fftBuffer[i] = sampleValue * window;
	}

	// 簡易FFT (Cooley-Tukey法)
	size_t n = fftSize;
	size_t j = 0;
	// ビット反転
	for (size_t i = 0; i < n; ++i) {
		if (i < j)
			std::swap(fftBuffer[i], fftBuffer[j]);
		size_t m = n >> 1;
		while (j >= m && m > 0) {
			j -= m;
			m >>= 1;
		}
		j += m;
	}

	// バタフライ演算
	for (size_t m = 1; m < n; m <<= 1) {
		float theta = -3.14159265f / m;
		std::complex<float> wm(cosf(theta), sinf(theta));
		for (size_t k = 0; k < n; k += (m << 1)) {
			std::complex<float> w(1.0f, 0.0f);
			for (size_t x = 0; x < m; ++x) {
				std::complex<float> u = fftBuffer[k + x];
				std::complex<float> t = w * fftBuffer[k + x + m];
				fftBuffer[k + x] = u + t;
				fftBuffer[k + x + m] = u - t;
				w *= wm;
			}
		}
	}

	// 結果の格納
	for (size_t i = 0; i < fftSize / 2; ++i) {
		float magnitude = std::abs(fftBuffer[i]);

		// 視覚的にわかりやすくするため対数スケールなどに調整しても良い
		output[i] = magnitude * 0.1f;
	}
}
