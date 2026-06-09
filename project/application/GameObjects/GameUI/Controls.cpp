#include "Controls.h"

using namespace TinyEngine;

void Controls::Initialize(EngineContext* ctx) {
	holdUI_ = std::make_unique<Sprite>();
	holdUI_->Initialize(ctx, "HoldUI.png");
	holdUI_->SetPosition({0, 570});
	holdUI_->SetColor({1, 1, 1, 1});

	pushUI_ = std::make_unique<Sprite>();
	pushUI_->Initialize(ctx, "PushUI.png");
	pushUI_->SetPosition({880, 570});
	pushUI_->SetColor({1, 1, 1, 1});
}

void Controls::Update() {
	holdUI_->Update();
	pushUI_->Update();
}

void Controls::Draw() {
	holdUI_->Draw();
	pushUI_->Draw();
}
