#include "Model.h"
#include "DirectXUtils.h"
#include "MathOperator.h"
#include "MathUtility.h"
#include "ModelCommon.h"
#include "TextureManager.h"
#include <assert.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <fstream>
#include <sstream>

using namespace Microsoft::WRL;

void Model::Initialize(ModelCommon* modelCommon, TextureManager* textureManager, const std::string& filename) {
	modelCommon_ = modelCommon;
	textureManager_ = textureManager;
	filename_ = filename;

	// モデル読み込み
	modelData_ = LoadModelFile(filename_);

	// 頂点データの初期化
	CreateVertexData();

	// インデックスデータの初期化
	CreateIndexData();

	// テクスチャ読み込み
	textureManager_->LoadTexture(modelData_.material.textureFilePath);

	// テクスチャ番号を取得して、メンバ変数に書き込む
	modelData_.material.textureIndex = textureManager_->GetSrvIndex(modelData_.material.textureFilePath);
}

void Model::Update() {}

Skeleton Model::CreateSkeleton(const Node& rootNode) {
	Skeleton skeleton;
	skeleton.root = CreateJoint(rootNode, {}, skeleton.joints);

	// 名前とindexのマッピングを行いアクセスしやすくする
	for (const Joint& joint : skeleton.joints) {
		skeleton.jointMap.emplace(joint.name, joint.index);
	}

	UpdateSkeleton(skeleton);

	return skeleton;
}

int32_t Model::CreateJoint(const Node& node, const std::optional<int32_t>& parent, std::vector<Joint>& joints) {
	Joint joint;
	joint.name = node.name;
	joint.localMatrix = node.localMatrix;
	joint.skeletonSpaceMatrix = MathUtility::MakeIdentity4x4();
	joint.transform = node.transform;
	joint.index = int32_t(joints.size()); // 現在登録されてる数をIndexに
	joint.parent = parent;

	joints.push_back(joint); // SkeletonのJoint列に追加

	for (const Node& child : node.children) {
		// 子Jointを作成し、そのIndexを登録
		int32_t childIndex = CreateJoint(child, joint.index, joints);

		// joints.push_backによるメモリ再確保で参照が切れるのを防ぐため、
		// 毎回現在のjoints[joint.index]からchildrenにアクセスして追加する
		joints[joint.index].children.push_back(childIndex);
	}

	// 自身のIndexを返す
	return joint.index;
}

void Model::UpdateSkeleton(Skeleton& skeleton) {
	for (Joint& joint : skeleton.joints) {
		// 現在のTransformからローカル行列を計算
		joint.localMatrix = MathUtility::MakeAffineMatrix(joint.transform.scale, joint.transform.rotate, joint.transform.translate);

		// 親がある場合は親の行列を掛け合わせ、無い場合は自身のローカル行列をそのまま代入
		if (joint.parent) {
			joint.skeletonSpaceMatrix = joint.localMatrix * skeleton.joints[*joint.parent].skeletonSpaceMatrix;
		} else {
			joint.skeletonSpaceMatrix = joint.localMatrix;
		}
	}
}

void Model::ApplyAnimation(Skeleton& skeleton, const Animation& animation, float animationTime) {
	for (Joint& joint : skeleton.joints) {
		// アニメーションデータの中に、このJointの名前と一致するNodeAnimationがあるか探す
		if (auto it = animation.nodeAnimations.find(joint.name); it != animation.nodeAnimations.end()) {
			const NodeAnimation& rootNodeAnimation = it->second;

			// Translate, Rotate, Scale の各キーフレームの補間値を計算して、JointのTransformを上書きする
			joint.transform.translate = KeyframeAnimation::CalculateValue<Vector3, KeyframeVector3>(rootNodeAnimation.translate, animationTime);
			joint.transform.rotate = KeyframeAnimation::CalculateValue<Quaternion, KeyframeQuaternion>(rootNodeAnimation.rotate, animationTime);
			joint.transform.scale = KeyframeAnimation::CalculateValue<Vector3, KeyframeVector3>(rootNodeAnimation.scale, animationTime);
		}
	}
}

SkinCluster Model::CreateSkinCluster(
    const Microsoft::WRL::ComPtr<ID3D12Device>& device, const Skeleton& skeleton, const ModelData& modelData, const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap,
    uint32_t descriptorSize, uint32_t paletteSrvIndex, uint32_t inputVertexSrvIndex, uint32_t influenceSrvIndex, uint32_t outputUavIndex) {

	SkinCluster skinCluster;
	skinCluster.vertexCount = static_cast<uint32_t>(modelData.vertices.size());

	skinCluster.paletteResource = DirectXUtils::CreateBufferResource(device, sizeof(WellForGPU) * skeleton.joints.size());
	WellForGPU* mappedPalette = nullptr;
	skinCluster.paletteResource->Map(0, nullptr, reinterpret_cast<void**>(&mappedPalette));
	skinCluster.mappedPalete = {mappedPalette, skeleton.joints.size()};

	skinCluster.paletteSrvHandle.first = modelCommon_->GetDxCommon()->GetCPUDescriptorHandle(descriptorHeap, descriptorSize, paletteSrvIndex);
	skinCluster.paletteSrvHandle.second = modelCommon_->GetDxCommon()->GetGPUDescriptorHandle(descriptorHeap, descriptorSize, paletteSrvIndex);

	D3D12_SHADER_RESOURCE_VIEW_DESC paletteSrvDesc{};
	paletteSrvDesc.Format = DXGI_FORMAT_UNKNOWN;
	paletteSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	paletteSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	paletteSrvDesc.Buffer.FirstElement = 0;
	paletteSrvDesc.Buffer.NumElements = UINT(skeleton.joints.size());
	paletteSrvDesc.Buffer.StructureByteStride = sizeof(WellForGPU);
	device->CreateShaderResourceView(skinCluster.paletteResource.Get(), &paletteSrvDesc, skinCluster.paletteSrvHandle.first);

	skinCluster.influenceResource = DirectXUtils::CreateBufferResource(device, sizeof(VertexInfluence) * modelData.vertices.size());
	VertexInfluence* mappedInfluence = nullptr;
	skinCluster.influenceResource->Map(0, nullptr, reinterpret_cast<void**>(&mappedInfluence));
	std::memset(mappedInfluence, 0, sizeof(VertexInfluence) * modelData.vertices.size());
	skinCluster.mappedInfluence = {mappedInfluence, modelData.vertices.size()};

	skinCluster.influenceSrvHandle.first = modelCommon_->GetDxCommon()->GetCPUDescriptorHandle(descriptorHeap, descriptorSize, influenceSrvIndex);
	skinCluster.influenceSrvHandle.second = modelCommon_->GetDxCommon()->GetGPUDescriptorHandle(descriptorHeap, descriptorSize, influenceSrvIndex);

	D3D12_SHADER_RESOURCE_VIEW_DESC influenceSrvDesc{};
	influenceSrvDesc.Format = DXGI_FORMAT_UNKNOWN;
	influenceSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	influenceSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	influenceSrvDesc.Buffer.FirstElement = 0;
	influenceSrvDesc.Buffer.NumElements = UINT(modelData.vertices.size());
	influenceSrvDesc.Buffer.StructureByteStride = sizeof(VertexInfluence);
	device->CreateShaderResourceView(skinCluster.influenceResource.Get(), &influenceSrvDesc, skinCluster.influenceSrvHandle.first);

	skinCluster.inputVertexSrvHandle.first = modelCommon_->GetDxCommon()->GetCPUDescriptorHandle(descriptorHeap, descriptorSize, inputVertexSrvIndex);
	skinCluster.inputVertexSrvHandle.second = modelCommon_->GetDxCommon()->GetGPUDescriptorHandle(descriptorHeap, descriptorSize, inputVertexSrvIndex);

	D3D12_SHADER_RESOURCE_VIEW_DESC inputVertexSrvDesc{};
	inputVertexSrvDesc.Format = DXGI_FORMAT_UNKNOWN;
	inputVertexSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	inputVertexSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	inputVertexSrvDesc.Buffer.FirstElement = 0;
	inputVertexSrvDesc.Buffer.NumElements = UINT(modelData.vertices.size());
	inputVertexSrvDesc.Buffer.StructureByteStride = sizeof(VertexData);
	device->CreateShaderResourceView(vertexResource_.Get(), &inputVertexSrvDesc, skinCluster.inputVertexSrvHandle.first);

	{
		UINT64 bufferSize = sizeof(VertexData) * modelData.vertices.size();

		D3D12_HEAP_PROPERTIES heapProps{};
		heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

		D3D12_RESOURCE_DESC resourceDesc{};
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resourceDesc.Width = bufferSize;
		resourceDesc.Height = 1;
		resourceDesc.DepthOrArraySize = 1;
		resourceDesc.MipLevels = 1;
		resourceDesc.SampleDesc.Count = 1;
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

		HRESULT hr = device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&skinCluster.outputVertexResource));
		assert(SUCCEEDED(hr));
	}

	skinCluster.outputUavHandle.first = modelCommon_->GetDxCommon()->GetCPUDescriptorHandle(descriptorHeap, descriptorSize, outputUavIndex);
	skinCluster.outputUavHandle.second = modelCommon_->GetDxCommon()->GetGPUDescriptorHandle(descriptorHeap, descriptorSize, outputUavIndex);

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.NumElements = UINT(modelData.vertices.size());
	uavDesc.Buffer.StructureByteStride = sizeof(VertexData);
	device->CreateUnorderedAccessView(skinCluster.outputVertexResource.Get(), nullptr, &uavDesc, skinCluster.outputUavHandle.first);

	skinCluster.outputVertexBufferView.BufferLocation = skinCluster.outputVertexResource->GetGPUVirtualAddress();
	skinCluster.outputVertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * modelData.vertices.size());
	skinCluster.outputVertexBufferView.StrideInBytes = sizeof(VertexData);
	skinCluster.inverseBindPoseMatrix.resize(skeleton.joints.size());
	std::generate(skinCluster.inverseBindPoseMatrix.begin(), skinCluster.inverseBindPoseMatrix.end(), MathUtility::MakeIdentity4x4);

	for (const auto& jointWeight : modelData.skinClusterData) {
		auto it = skeleton.jointMap.find(jointWeight.first);
		if (it == skeleton.jointMap.end())
			continue;

		skinCluster.inverseBindPoseMatrix[(*it).second] = jointWeight.second.inverseBindPoseMatrix;

		for (const auto& vertexWeight : jointWeight.second.vertexWeights) {
			auto& currentInfluence = skinCluster.mappedInfluence[vertexWeight.vertexIndex];
			for (uint32_t index = 0; index < kNumMaxInfluence; ++index) {
				if (currentInfluence.weight[index] == 0.0f) {
					currentInfluence.weight[index] = vertexWeight.weight;
					currentInfluence.jointIndices[index] = (*it).second;
					break;
				}
			}
		}
	}

	return skinCluster;
}

void Model::UpdateSkinCluster(SkinCluster& skinCluster, const Skeleton& skeleton){
	for (size_t jointIndex = 0; jointIndex < skeleton.joints.size(); ++jointIndex) {
		assert(jointIndex < skinCluster.inverseBindPoseMatrix.size());

		// SkinningMatrixの計算
		Matrix4x4 skinningMatrix = MathUtility::Multiply(skinCluster.inverseBindPoseMatrix[jointIndex], skeleton.joints[jointIndex].skeletonSpaceMatrix);

		// GPU構造体への書き込み
		skinCluster.mappedPalete[jointIndex].skeltonSpaceMatrix = skinningMatrix;

		// 逆行列を計算
		Matrix4x4 inverseMatrix = MathUtility::Inverse(skinningMatrix);

		// 転置行列を計算して代入
		skinCluster.mappedPalete[jointIndex].skeltonSpaceInverseTransposeMatrix = MathUtility::Transpose(inverseMatrix);
	}
}

void Model::DispatchSkinning(ID3D12GraphicsCommandList* commandList, const SkinCluster& skinCluster) {
	commandList->SetComputeRootDescriptorTable(0, skinCluster.inputVertexSrvHandle.second);
	commandList->SetComputeRootDescriptorTable(1, skinCluster.influenceSrvHandle.second);
	commandList->SetComputeRootDescriptorTable(2, skinCluster.paletteSrvHandle.second);
	commandList->SetComputeRootDescriptorTable(3, skinCluster.outputUavHandle.second);

	UINT threadGroupCount = (skinCluster.vertexCount + 1023) / 1024;
	commandList->Dispatch(threadGroupCount, 1, 1);
}

void Model::Draw(const std::string& textureFilePath, const SkinCluster* skinCluster) {
	auto commandList = modelCommon_->GetDxCommon()->GetCommandList();

	if (skinCluster) {
		D3D12_RESOURCE_BARRIER barrier{};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Transition.pResource = skinCluster->outputVertexResource.Get();
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		commandList->ResourceBarrier(1, &barrier);

		commandList->IASetVertexBuffers(0, 1, &skinCluster->outputVertexBufferView);
	} else {
		commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);
	}

	commandList->IASetIndexBuffer(&indexBufferView_);

	std::string path = textureFilePath.empty() ? modelData_.material.textureFilePath : textureFilePath;
	commandList->SetGraphicsRootDescriptorTable(2, textureManager_->GetSrvHandleGPU(path));

	commandList->DrawIndexedInstanced(indexCount_, 1, 0, 0, 0);

	if (skinCluster) {
		// 次フレームのCS書き込みに備えてUAV状態に戻す
		D3D12_RESOURCE_BARRIER barrierBack{};
		barrierBack.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrierBack.Transition.pResource = skinCluster->outputVertexResource.Get();
		barrierBack.Transition.StateBefore = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
		barrierBack.Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		barrierBack.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		commandList->ResourceBarrier(1, &barrierBack);
	}
}

ModelData Model::LoadModelFile(const std::string& filename) {
	ModelData modelData;            // 構築するModelData
	std::vector<Vector4> positions; // 位置
	std::vector<Vector3> normals;   // 法線
	std::vector<Vector2> texcoords; // テクスチャ座標
	std::string line;               // ファイルから読んだ1行を格納するもの

	// デフォルトのテクスチャパスを事前に設定
	modelData.material.textureFilePath = "resources/models/white.png";

	Assimp::Importer importer;
	std::string filePath = "resources/models/" + filename;
	// 三角形化とUV/向きの調整に加えて、法線がない場合の自動生成フラグ（aiProcess_GenNormals）を追加
	const aiScene* scene = importer.ReadFile(filePath.c_str(), aiProcess_Triangulate | aiProcess_FlipWindingOrder | aiProcess_FlipUVs | aiProcess_GenNormals);
	assert(scene && scene->HasMeshes()); // シーン・メッシュがないのは対応しない

	for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
		aiMesh* mesh = scene->mMeshes[meshIndex];

		const bool hasTexcoords = mesh->HasTextureCoords(0);

		// 複数メッシュが含まれる場合、インデックスがずれないように現在の頂点数をオフセットとして保持
		uint32_t vertexOffset = static_cast<uint32_t>(modelData.vertices.size());

		// 頂点の解析
		for (uint32_t vertexIndex = 0; vertexIndex < mesh->mNumVertices; ++vertexIndex) {
			aiVector3D& position = mesh->mVertices[vertexIndex];
			aiVector3D& normal = mesh->mNormals[vertexIndex];

			aiVector3D texcoord;
			if (hasTexcoords) {
				texcoord = mesh->mTextureCoords[0][vertexIndex];
			} else {
				texcoord = aiVector3D(0.0f, 0.0f, 0.0f);
			}

			VertexData vertex;
			vertex.position = {position.x, position.y, position.z, 1.0f};
			vertex.normal = {normal.x, normal.y, normal.z};
			vertex.texcoord = {texcoord.x, texcoord.y};

			// aiProcess_MaskLeftHandedはz*=-1で、右手->左手に変換するので手動で対処
			vertex.position.x *= -1.0f;
			vertex.normal.x *= -1.0f;

			modelData.vertices.push_back(vertex);
		}

		// インデックスの解析
		for (uint32_t faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex) {
			aiFace& face = mesh->mFaces[faceIndex];
			assert(face.mNumIndices == 3); // 三角形のみサポート

			for (uint32_t element = 0; element < face.mNumIndices; ++element) {
				// インデックスに頂点オフセットを加算して追加
				modelData.indices.push_back(face.mIndices[element] + vertexOffset);
			}
		}

		for (uint32_t boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex) {
			aiBone* bone = mesh->mBones[boneIndex];
			std::string jointName = bone->mName.C_Str();
			JointWeightData& jointWeightData = modelData.skinClusterData[jointName];

			aiMatrix4x4 bindPoseMatrixAssimp = bone->mOffsetMatrix.Inverse();
			aiVector3D scale, translate;
			aiQuaternion rotate;
			bindPoseMatrixAssimp.Decompose(scale, rotate, translate);
			Matrix4x4 bindPoseMatrix = MathUtility::MakeAffineMatrix({scale.x, scale.y, scale.z}, {rotate.x, -rotate.y, -rotate.z, rotate.w}, {-translate.x, translate.y, translate.z});
			jointWeightData.inverseBindPoseMatrix = MathUtility::Inverse(bindPoseMatrix);

			for (uint32_t weightIndex = 0; weightIndex < bone->mNumWeights; ++weightIndex) {
				jointWeightData.vertexWeights.push_back({bone->mWeights[weightIndex].mWeight, bone->mWeights[weightIndex].mVertexId});
			}
		}
	}

	// マテリアルの中身を解析していく
	for (uint32_t materialIndex = 0; materialIndex < scene->mNumMaterials; ++materialIndex) {
		aiMaterial* material = scene->mMaterials[materialIndex];
		if (material->GetTextureCount(aiTextureType_DIFFUSE) != 0) {
			aiString textureFilePath;
			material->GetTexture(aiTextureType_DIFFUSE, 0, &textureFilePath);

			// テクスチャファイルパスが空でない場合は指定したパスを使用
			if (textureFilePath.length > 0) {
				std::string texStr = textureFilePath.C_Str();
				// 埋め込みテクスチャ参照は "*<index>" の形式になる（例: "*0"）
				if (!texStr.empty() && texStr[0] == '*') {
					// "*n" の n を取り出す
					int texIndex = 0;
					try {
						texIndex = std::stoi(texStr.substr(1));
					} catch (...) {
						texIndex = -1;
					}

					if (texIndex >= 0 && scene->mTextures && texIndex < static_cast<int>(scene->mNumTextures)) {
						aiTexture* atex = scene->mTextures[texIndex];
						// 圧縮されたイメージデータ（PNG/JPEG 等）は mHeight == 0, pcData に生のバイナリが入る
						if (atex->mHeight == 0 && atex->pcData) {
							// 出力ファイル名を作る（モデル名を元に一意化）
							std::string baseName = filename;
							auto pos = baseName.find_last_of('.');
							if (pos != std::string::npos)
								baseName = baseName.substr(0, pos);
							std::string outPath = "resources/models/" + baseName + "_embedded" + std::to_string(texIndex) + ".png";

							// バイナリ書き出し
							std::ofstream ofs(outPath, std::ios::binary);
							if (ofs) {
								ofs.write(reinterpret_cast<const char*>(atex->pcData), static_cast<std::streamsize>(atex->mWidth));
								ofs.close();
								modelData.material.textureFilePath = outPath;
							} else {
								// 書き出し失敗時はデフォルトのままにする（white.png）
							}
						} else {
							// mHeight > 0 の場合は非圧縮RGBA等の生データが入っている。
							// ここを対応するには生データをPNG等へ変換する処理が必要
							// 現時点ではデフォルトテクスチャを使用
						}
					}
				} else {
					// 通常のファイルパスをそのまま利用（既存の処理）
					modelData.material.textureFilePath = "resources/models/" + texStr;
				}
			}
		}
	}

	modelData.rootNode = ReadNode(scene->mRootNode);

	return modelData;
}

MaterialData Model::LoadMaterialTemplateFile(const std::string& filename) {
	MaterialData materialData;                          // 構築するMaterialData
	std::string line;                                   // ファイルから読んだ1行を格納するもの
	std::ifstream file("resources/models/" + filename); // ファイルを開く
	assert(file.is_open());                             // とりあえず開けなかったら止める

	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;

		// identifierに応じた処理
		if (identifier == "map_Kd") {
			std::string textureFilename;
			s >> textureFilename;
			// 連結してファイルパスにする
			materialData.textureFilePath = "resources/models/" + textureFilename;
		}
	}
	return materialData;
}

void Model::CreateVertexData() {
	// 頂点リソースの作成
	vertexResource_ = DirectXUtils::CreateBufferResource(modelCommon_->GetDxCommon()->GetDevice(), sizeof(VertexData) * modelData_.vertices.size());
	// リソースの先頭のアドレスから使う
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点6つ分のサイズ
	vertexBufferView_.SizeInBytes = static_cast<UINT>(sizeof(VertexData) * modelData_.vertices.size());
	// 1頂点あたりのサイズ
	vertexBufferView_.StrideInBytes = sizeof(VertexData);
	// 書き込むためのアドレスを取得
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));
	std::memcpy(vertexData_, modelData_.vertices.data(), sizeof(VertexData) * modelData_.vertices.size());
	vertexResource_->Unmap(0, nullptr);
}

void Model::CreateIndexData() {
	// modelData_に格納されたインデックス配列のサイズを取得
	indexCount_ = static_cast<uint32_t>(modelData_.indices.size());

	// インデックスリソースの作成
	indexResource_ = DirectXUtils::CreateBufferResource(modelCommon_->GetDxCommon()->GetDevice(), sizeof(uint32_t) * indexCount_);

	// リソースの先頭のアドレスから使う
	indexBufferView_.BufferLocation = indexResource_->GetGPUVirtualAddress();
	// 使用するリソースのサイズ
	indexBufferView_.SizeInBytes = sizeof(uint32_t) * indexCount_;
	// フォーマット
	indexBufferView_.Format = DXGI_FORMAT_R32_UINT;

	// 書き込むためのアドレスを取得
	indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&indexData_));
	std::memcpy(indexData_, modelData_.indices.data(), sizeof(uint32_t) * indexCount_);
	indexResource_->Unmap(0, nullptr);
}

Node Model::ReadNode(aiNode* node) {
	Node result;

	// Assimp行列を取得
	aiMatrix4x4 aiLocalMatrix = node->mTransformation;

	// SRT分解
	aiVector3D scale, translate;
	aiQuaternion rotate;
	aiLocalMatrix.Decompose(scale, rotate, translate);

	// 座標系変換 右手→左手
	result.transform.scale = {scale.x, scale.y, scale.z};

	result.transform.rotate = {-rotate.x, -rotate.y, rotate.z, rotate.w};

	result.transform.translate = {translate.x, translate.y, -translate.z};

	// ローカル行列を再構築
	result.localMatrix = MathUtility::MakeAffineMatrix(result.transform.scale, result.transform.rotate, result.transform.translate);

	// 名前と子ノード
	result.name = node->mName.C_Str();
	result.children.resize(node->mNumChildren);

	for (uint32_t childIndex = 0; childIndex < node->mNumChildren; ++childIndex) {
		result.children[childIndex] = ReadNode(node->mChildren[childIndex]);
	}

	return result;
}