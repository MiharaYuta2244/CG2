#pragma once
#include "GameObjects/IGameObject.h"
#include "MathUtility.h"

struct TransformSnapshot {
	Vector3 scale;
	Vector3 rotate;
	Vector3 translate;
};

inline TransformSnapshot CaptureTransform(const Transform& t) { return TransformSnapshot{t.scale, t.rotate, t.translate}; }

/// <summary>
/// Undo/Redo可能な操作の基底クラス
/// </summary>
class IEditorCommand {
public:
	virtual ~IEditorCommand() = default;
	virtual void Undo() = 0;
	virtual void Redo() = 0;

	// このコマンドが対象にしているオブジェクトを返す
	virtual IGameObject* GetTarget() const = 0;
};

/// <summary>
/// Transformの変更を1件記録するコマンド
/// </summary>
class TransformCommand : public IEditorCommand {
public:
	TransformCommand(IGameObject* target, const TransformSnapshot& before, const TransformSnapshot& after) : target_(target), before_(before), after_(after) {}

	void Undo() override { Apply(before_); }
	void Redo() override { Apply(after_); }
	IGameObject* GetTarget() const override { return target_; }

private:
	void Apply(const TransformSnapshot& snap);

	IGameObject* target_;
	TransformSnapshot before_;
	TransformSnapshot after_;
};