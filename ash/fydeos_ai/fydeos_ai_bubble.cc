// Copyright 2023 The Fyde Innovations Limited. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ash/fydeos_ai/fydeos_ai_bubble.h"
#include "ui/views/layout/fill_layout.h"
#include "ui/aura/window.h"
#include "ash/public/cpp/shell_window_ids.h"
#include "ui/views/highlight_border.h"
#include "ui/chromeos/styles/cros_tokens_color_mappings.h"

namespace ash {

FydeAssistantBubble::~FydeAssistantBubble() = default;

gfx::Size FydeAssistantBubble::CalculatePreferredSize(const views::SizeBounds& available_size) const {
  return gfx::Size(320, 100);
}

void FydeAssistantBubble::OnThemeChanged() {
  views::BubbleDialogDelegateView::OnThemeChanged();
  set_color(
      GetColorProvider()->GetColor(cros_tokens::kCrosSysSystemBaseElevated));
}

FydeAssistantBubble::FydeAssistantBubble(const gfx::Rect& anchor_rect) {
  SetAnchorRect(anchor_rect);
  SetButtons(ui::DIALOG_BUTTON_NONE);
  set_margins(gfx::Insets());
  set_force_create_contents_background(true);
  // control hide/show on  my own
  set_close_on_deactivate(false);
  SetBorder(std::make_unique<views::HighlightBorder>(
      12,
      views::HighlightBorder::Type::kHighlightBorderOnShadow));
  SetLayoutManager(std::make_unique<views::FillLayout>());
  auto label = std::make_unique<views::Label>();
  label->SetHorizontalAlignment(gfx::ALIGN_LEFT);
  label->SetMultiLine(true);
  label->SetText(u"Hello World");
  label_ = AddChildView(std::move(label));

  CreateBubble(this);
  // views::DialogDelegate::CreateDialogWidget(this, nullptr, window);
}

void FydeAssistantBubble::UpdateContent(std::u16string text) {
  label_->SetText(text);
}


}
