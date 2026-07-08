#include "IGameObject.h"

void IGameObject::UpdateAABBForGizmo() {
	Vector3 pos = transform_.translate;
	Vector3 scale = transform_.scale;

	aabbForGizmo_.max = {pos.x + scale.x, pos.y + scale.y, pos.z + scale.z};
	aabbForGizmo_.min = {pos.x - scale.x, pos.y - scale.y, pos.z - scale.z};
}