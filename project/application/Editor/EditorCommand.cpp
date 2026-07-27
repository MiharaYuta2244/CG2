#include "EditorCommand.h"

void TransformCommand::Apply(const TransformSnapshot& snap) {
	Transform& t = target_->GetTransform();
	t.translate = snap.translate;
	t.rotate = snap.rotate;
	t.scale = snap.scale;
}