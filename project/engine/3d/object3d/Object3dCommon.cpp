#include "Object3dCommon.h"
#include "DirectXCommon.h"
#include "StringUtility.h"
#include "DirectXUtils.h"
#include <DirectXMath.h>
#include <d3dx12.h>
#include <format>

using namespace Microsoft::WRL;
using namespace DirectX;

void Object3dCommon::DrawSettingCommon() {
	// RootSignatureを設定。PSOに設定しているけど別途設定が必要
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(rootSignature_.Get());
	dxCommon_->GetCommandList()->SetPipelineState(graphicsPipelineState_.Get()); // PSOを設定
	// 形状を設定。PSOに設定しているものとはまた別。同じものを設定すると考えておけば良い
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// ライティングCBufferの場所を指定
	dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(3, globalDirectionalLightResource_->GetGPUVirtualAddress());
	// ポイントライトCBufferの場所を指定
	dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(7, globalPointLightResource_->GetGPUVirtualAddress());
	// スポットライトCBufferの場所を指定
	dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(8, globalSpotLightResource_->GetGPUVirtualAddress());
}

void Object3dCommon::DrawSettingOutline() {
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(rootSignature_.Get());
	dxCommon_->GetCommandList()->SetPipelineState(outlinePipelineState_.Get()); // アウトライン用PSO
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Object3dCommon::Initialize(DirectXCommon* dxCommon) {
	dxCommon_ = dxCommon;

	InitializeShaderCompiler();

	// グラフィックスパイプラインの生成
	CreateGraphicsPipeline();

	// グローバルライティングリソースの生成
	CreateGlobalDirectionalLightData();
	CreateGlobalPointLightData();
	CreateGlobalSpotLightData();

	// アウトラインリソースの作成
	CreateOutlinePipeline();
}

void Object3dCommon::Update() {
	// グローバルライティングバッファ更新
	UpdateGlobalLightingBuffers();
}

#ifdef USE_IMGUI
void Object3dCommon::DrawImGuiLighting() {
	ImGui::Begin("DirectionalLight");
	ImGui::DragFloat3("Direction", &globalDirectionalLight_.direction.x, 0.01f);
	ImGui::DragFloat("Intensity", &globalDirectionalLight_.intensity, 0.01f);
	ImGui::ColorEdit4("Color", &globalDirectionalLight_.color.x);
	ImGui::End();

	ImGui::Begin("PointLight");
	ImGui::DragFloat3("Direction", &globalPointLight_.position.x, 0.01f);
	ImGui::DragFloat("Intensity", &globalPointLight_.intensity, 0.01f);
	ImGui::DragFloat("Radius", &globalPointLight_.radius, 0.01f);
	ImGui::DragFloat("Decay", &globalPointLight_.decay, 0.01f);
	ImGui::ColorEdit4("Color", &globalPointLight_.color.x);
	ImGui::End();

	ImGui::Begin("SpotLight");
	ImGui::DragFloat3("Direction", &globalSpotLight_.direction.x, 0.01f);
	ImGui::DragFloat3("Direction", &globalSpotLight_.position.x, 0.01f);
	ImGui::DragFloat("Intensity", &globalSpotLight_.intensity, 0.01f);
	ImGui::DragFloat("Distance", &globalSpotLight_.distance, 0.01f);
	ImGui::DragFloat("CosAngle", &globalSpotLight_.cosAngle, 0.01f);
	ImGui::DragFloat("Decay", &globalSpotLight_.decay, 0.01f);
	ImGui::ColorEdit4("Color", &globalSpotLight_.color.x);
	ImGui::End();
}
#endif // USE_IMGUI

void Object3dCommon::CreateRootSignature() {
	HRESULT hr;

	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = 0;                                                   // 0から始まる
	descriptorRange[0].NumDescriptors = 1;                                                       // 数は1つ
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;                              // SRVを使う
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND; // Offsetを自動計算

	// RootParameter作成。複数設定できるので配列。
	D3D12_ROOT_PARAMETER rootParameters[10] = {};
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;                   // CBVを使う
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;                // PixelShaderで使う
	rootParameters[0].Descriptor.ShaderRegister = 0;                                   // レジスタ番号0とバインド
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;                   // CBVを使う
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;               // VertexShaderで使う
	rootParameters[1].Descriptor.ShaderRegister = 0;                                   // レジスタ番号0を使う
	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;      // DescriptorTableを使う
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;                // PixelShaderで使う
	rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRange;             // Tableの中身の配列を指定
	rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange); // Tableで利用する数
	rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;                   // CBVを使う
	rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;                // PixelShaderで使う
	rootParameters[3].Descriptor.ShaderRegister = 1;                                   // レジスタ番号1を使う
	rootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;                   // CBVを使う
	rootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;                // PixelShaderで使う
	rootParameters[4].Descriptor.ShaderRegister = 2;                                   // レジスタ番号2を使う
	rootParameters[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;                   // CBVを使う
	rootParameters[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;                // PixelShaderで使う
	rootParameters[5].Descriptor.ShaderRegister = 3;                                   // レジスタ番号3を使う
	rootParameters[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;                   // CBVを使う
	rootParameters[6].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;                // PixelShaderで使う
	rootParameters[6].Descriptor.ShaderRegister = 4;                                   // レジスタ番号4を使う
	rootParameters[7].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;                   // CBVを使う
	rootParameters[7].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;                // PixelShaderで使う
	rootParameters[7].Descriptor.ShaderRegister = 5;                                   // レジスタ番号5を使う（PointLight）
	rootParameters[8].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;                   // CBVを使う
	rootParameters[8].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;                // PixelShaderで使う
	rootParameters[8].Descriptor.ShaderRegister = 6;                                   // レジスタ番号6を使う（SpotLight）
	rootParameters[9].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;                   // 定数バッファ
	rootParameters[9].Descriptor.ShaderRegister = 7;                                   // レジスタ番号7を使う（Outline）
	rootParameters[9].Descriptor.RegisterSpace = 0;                                    // space0
	rootParameters[9].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;                  // どちらからも見えるようにする

	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;   // バイリニアフィルタ
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP; // 0~1の範囲をリピート
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;     // 比較しない
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;                       // ありったけのMinmapを使う
	staticSamplers[0].ShaderRegister = 0;                               // レジスタ番号0を使う
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderで使う

	// RootSignature作成
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	descriptionRootSignature.pParameters = rootParameters;             // ルートパラメータ配列へのポインタ
	descriptionRootSignature.NumParameters = _countof(rootParameters); // 配列の長さ
	descriptionRootSignature.pStaticSamplers = staticSamplers;
	descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);

	// シリアライズしてバイナリにする
	ID3DBlob* signatureBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;
	hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr)) {
		Logger::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()), LogLevel::Error);
		assert(false);
	}
	// バイナリを元に生成
	rootSignature_ = nullptr;
	hr = dxCommon_->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature_));
	assert(SUCCEEDED(hr));
}

void Object3dCommon::CreateGraphicsPipeline() {
	CreateRootSignature();
	HRESULT hr;

	// InputLayout
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[3] = {};
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[1].SemanticName = "TEXCOORD";
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[2].SemanticName = "NORMAL";
	inputElementDescs[2].SemanticIndex = 0;
	inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);

	// BlendStateの設定
	D3D12_BLEND_DESC blendDesc{};
	// すべての色要素を書き込む
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;

	// RasterizerStateの設定
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	// 裏面(時計回り)を表示しない
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	// 三角形の中を塗りつぶす
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	// Shaderをコンパイルする
	IDxcBlob* vertexShaderBlob = DirectXUtils::CompileShader(L"resources/shaders/Object3d.VS.hlsl", L"vs_6_0", dxcUtils_.Get(), dxcCompiler_.Get(), includeHandler_.Get());
	assert(vertexShaderBlob != nullptr);

	IDxcBlob* pixelShaderBlob = DirectXUtils::CompileShader(L"resources/shaders/Object3d.PS.hlsl", L"ps_6_0", dxcUtils_.Get(), dxcCompiler_.Get(), includeHandler_.Get());
	assert(pixelShaderBlob != nullptr);

	// DepthStencilStateの設定
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	// Depthの機能を有効化する
	depthStencilDesc.DepthEnable = true;
	// 書き込みします
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	// 比較関数はLessEqual。つまり、近ければ描画される
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = rootSignature_.Get();                                          // RootSignature
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;                                                  // InputLayout
	graphicsPipelineStateDesc.VS = {vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize()}; // VertexShader
	graphicsPipelineStateDesc.PS = {pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize()};   // PixelShader
	graphicsPipelineStateDesc.BlendState = blendDesc;                                                         // BlendState
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;                                               // RasterizerState
	// 書き込むRTVの情報
	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	// 利用するトポロジ(形状)のタイプ。三角形
	graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	// どのように画面に色を打ち込むのかの設定(気にしなくていい)
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	// DepthStencilの設定
	graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
	graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	// 実際に生成
	hr = dxCommon_->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&graphicsPipelineState_));
	assert(SUCCEEDED(hr));
}

void Object3dCommon::InitializeShaderCompiler() {

	HRESULT hr;

	// dxcCompilerを初期化
	hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils_));
	assert(SUCCEEDED(hr));
	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler_));
	assert(SUCCEEDED(hr));
	// 現時点でincludeはしないが、includeに対応するための設定を行っていく
	hr = dxcUtils_->CreateDefaultIncludeHandler(&includeHandler_);
	assert(SUCCEEDED(hr));
}

void Object3dCommon::UpdateGlobalLightingBuffers() {
	// DirectionalLightの方向を正規化
	XMVECTOR dirVec = XMVectorSet(globalDirectionalLight_.direction.x, globalDirectionalLight_.direction.y, globalDirectionalLight_.direction.z, 0.0f);
	dirVec = XMVector3Normalize(dirVec);
	XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&globalDirectionalLight_.direction), dirVec);

	// SpotLightの方向を正規化
	XMVECTOR spotDirVec = XMVectorSet(globalSpotLight_.direction.x, globalSpotLight_.direction.y, globalSpotLight_.direction.z, 0.0f);
	spotDirVec = XMVector3Normalize(spotDirVec);
	XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&globalSpotLight_.direction), spotDirVec);

	// バッファを更新
	*globalDirectionalLightData_ = globalDirectionalLight_;
	*globalPointLightData_ = globalPointLight_;
	*globalSpotLightData_ = globalSpotLight_;
}

void Object3dCommon::CreateGlobalDirectionalLightData() {
	// 平行光源用のリソースを作る
	globalDirectionalLightResource_ = DirectXUtils::CreateBufferResource(dxCommon_->GetDevice(), sizeof(DirectionalLight));
	// アドレス取得
	globalDirectionalLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&globalDirectionalLightData_));
	globalDirectionalLightResource_->Unmap(0, nullptr);
	// 初期化
	globalDirectionalLight_.color = {1.0f, 1.0f, 1.0f, 1.0f};
	globalDirectionalLight_.direction = {-0.702f, -0.253f, 0.666f};
	globalDirectionalLight_.intensity = 1.2f;

	// Vector3 → XMVECTOR 変換
	XMVECTOR dirVec = XMVectorSet(globalDirectionalLight_.direction.x, globalDirectionalLight_.direction.y, globalDirectionalLight_.direction.z, 0.0f);
	// 正規化
	dirVec = XMVector3Normalize(dirVec);
	// XMVECTOR → Vector3 に戻す
	XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&globalDirectionalLight_.direction), dirVec);

	// globalDirectionalLightDataへの書き込み
	*globalDirectionalLightData_ = globalDirectionalLight_;
}

void Object3dCommon::CreateGlobalPointLightData() {
	// ポイントライト用のリソースを作る
	globalPointLightResource_ = DirectXUtils::CreateBufferResource(dxCommon_->GetDevice(), sizeof(PointLight));
	// アドレス取得
	globalPointLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&globalPointLightData_));
	globalPointLightResource_->Unmap(0, nullptr);
	// 初期化
	globalPointLight_.color = {1.0f, 1.0f, 1.0f, 1.0f};
	globalPointLight_.position = {0.0f, 5.0f, 0.0f};
	globalPointLight_.intensity = 0.0f;
	globalPointLight_.radius = 10.0f;
	globalPointLight_.decay = 1.0f;
	// globalPointLightDataへの書き込み
	*globalPointLightData_ = globalPointLight_;
}

void Object3dCommon::CreateGlobalSpotLightData() {
	// スポットライト用のリソースを作る
	globalSpotLightResource_ = DirectXUtils::CreateBufferResource(dxCommon_->GetDevice(), sizeof(SpotLight));
	// アドレス取得
	globalSpotLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&globalSpotLightData_));
	globalSpotLightResource_->Unmap(0, nullptr);
	// 初期化
	globalSpotLight_.color = {1.0f, 1.0f, 1.0f, 1.0f};
	globalSpotLight_.position = {0.0f, 10.0f, 0.0f};
	globalSpotLight_.intensity = 0.0f;
	globalSpotLight_.direction = {0.0f, -1.0f, 0.0f};
	globalSpotLight_.distance = 20.0f;
	globalSpotLight_.decay = 1.0f;
	globalSpotLight_.cosAngle = 0.707f; // 約45度のコサイン値

	// Vector3 → XMVECTOR 変換
	XMVECTOR dirVec = XMVectorSet(globalSpotLight_.direction.x, globalSpotLight_.direction.y, globalSpotLight_.direction.z, 0.0f);
	// 正規化
	dirVec = XMVector3Normalize(dirVec);
	// XMVECTOR → Vector3 に戻す
	XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&globalSpotLight_.direction), dirVec);

	// globalSpotLightDataへの書き込み
	*globalSpotLightData_ = globalSpotLight_;
}

void Object3dCommon::CreateOutlinePipeline() {
	// 基本的な設定は CreateGraphicsPipeline() と同様に進める
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};

	// シェーダーのセット
	auto vsBlob = DirectXUtils::CompileShader(L"resources/shaders/Outline.VS.hlsl", L"vs_6_0", dxcUtils_.Get(), dxcCompiler_.Get(), includeHandler_.Get());
	auto psBlob = DirectXUtils::CompileShader(L"resources/shaders/Outline.PS.hlsl", L"ps_6_0", dxcUtils_.Get(), dxcCompiler_.Get(), includeHandler_.Get());
	psoDesc.VS = {vsBlob->GetBufferPointer(), vsBlob->GetBufferSize()};
	psoDesc.PS = {psBlob->GetBufferPointer(), psBlob->GetBufferSize()};

	// ルートシグネチャのセット
	psoDesc.pRootSignature = rootSignature_.Get();

	// ラスタライザ設定
	psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	// 前面をカリングし背面のみ描画
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_FRONT;

	// 入力レイアウト
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[] = {
	    {"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	    {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	    {"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	};
	psoDesc.InputLayout = {inputElementDescs, _countof(inputElementDescs)};

	// その他の設定
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	psoDesc.SampleDesc.Count = 1;

	// PSO生成
	HRESULT hr = dxCommon_->GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&outlinePipelineState_));
	assert(SUCCEEDED(hr));
}