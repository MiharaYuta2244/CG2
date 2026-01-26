#include "ScreenSpaceUtility.h"
#include "Matrix4x4.h"
#include "MathUtility.h"

Vector2 ScreenSpaceUtility::WorldToScreen(const Vector3& worldPosition, const Vector2& margin) const { 
	// ビューポートマトリックス
	Matrix4x4 viewportMatrix = MathUtility::MakeViewPortMatrix(0, 0, 1280.0f, 720.0f, 0.0f, 1.0f);

	// カメラのViewProjectionとビューポートを合成してワールド→スクリーン変換行列を作る
	Matrix4x4 matVPV = MathUtility::Multiply(camera_->GetViewProjectionMatrix(), viewportMatrix);

	// プレイヤーのワールド座標をスクリーン座標に変換
	Vector3 worldPos = {worldPosition.x + margin.x, worldPosition.y + margin.y, worldPosition.z};
	Vector3 screenPos = MathUtility::Transform(worldPos, matVPV);

	return {screenPos.x, screenPos.y};
}
