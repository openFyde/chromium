// Copyright 2023 The Fyde Innovations Limited. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ash/fydeos_ai/fydeos_ai_view.h"
#include "base/check.h"
#include "base/logging.h"
#include "base/memory/ptr_util.h"
#include "base/time/time.h"
#include "ui/views/background.h"
#include "ui/views/controls/label.h"
#include "ash/shell.h"
#include "ash/session/session_controller_impl.h"
#include "ash/fydeos_ai/fydeos_ai_bubble.h"
#include "ui/wm/core/coordinate_conversion.h"

namespace ash {

FydeAssistantView::FydeAssistantView(aura::Window* container) {
  Shell::Get()->session_controller()->AddObserver(this);
  Shell::Get()->AddPreTargetHandler(this);
}

FydeAssistantView::~FydeAssistantView() {
  Shell::Get()->RemovePreTargetHandler(this);
  Shell::Get()->session_controller()->RemoveObserver(this);
}

bool FydeAssistantView::IsVisible() const {
  return bubble_ && bubble_->GetWidget()->IsVisible();
}

void FydeAssistantView::ShowBubble() {
  if (IsVisible()) {
    return;
  }
  Show();
}

void FydeAssistantView::ProcessPressedEvent(ui::LocatedEvent* event) {
  if (!bubble_) return;
  gfx::Point screen_location = event->location();
  ::wm::ConvertPointToScreen(static_cast<aura::Window*>(event->target()),
                             &screen_location);
  if (bubble_->GetBoundsInScreen().Contains(screen_location)) {
    return;
  }
  Hide();
}

void FydeAssistantView::OnTouchEvent(ui::TouchEvent* event) {
  ProcessPressedEvent(event->AsLocatedEvent());
}

void FydeAssistantView::OnMouseEvent(ui::MouseEvent* event) {
  if (event->type() == ui::ET_MOUSE_PRESSED) {
    ProcessPressedEvent(event->AsLocatedEvent());
  }
}

void FydeAssistantView::Show() {
  if (!can_show_) {
    return;
  }
  if (!bubble_) {
    return;
  }
  bubble_->SetAnchorRect(gfx::Rect(display::Screen::GetScreen()->GetCursorScreenPoint(), gfx::Size()));
  bubble_->UpdateContent(u"Just copied " + last_clipboard_display_text_);
  bubble_->GetWidget()->Show();
}

void FydeAssistantView::Hide() {
  if (!bubble_) {
    return;
  }
  bubble_->GetWidget()->Hide();
}

void FydeAssistantView::OnSessionStateChanged(session_manager::SessionState state) {
  if (state == session_manager::SessionState::ACTIVE) {
    can_show_ = true;
    bubble_ = new FydeAssistantBubble(gfx::Rect(display::Screen::GetScreen()->GetCursorScreenPoint(), gfx::Size()));
  }
}

void FydeAssistantView::UpdateLastClipboardItem(const ClipboardHistoryItem& item) {
  VLOG(3) << "clipboard update";
  last_clipboard_item_time_ = base::TimeTicks::Now();
  last_clipboard_display_text_ = item.display_text();
  // copy from subsystem can be slow,  slower than the gap between pressing ctrl+c and c twice
  auto now = base::TimeTicks::Now();
  if (now - last_time_triggered_ < base::Seconds(1) && should_show_bubble_delay_) {
    VLOG(3) << "pressed twice and clipboard updated, show the bubble now";
    ShowBubble();
  }
}

bool FydeAssistantView::CanHandleToggleFydeOSAssistant() {
  auto now = base::TimeTicks::Now();
  auto delta = now - last_time_triggered_;
  VLOG(3) << "fydeos assistant view accelerator, time delta: " << delta.InMilliseconds();
  bool triggered_before_clipboard_update = last_clipboard_item_time_ < last_time_triggered_;
  last_time_triggered_ = now;
  bool repeated = delta < base::Seconds(1);
  if (repeated && triggered_before_clipboard_update) {
    VLOG(3) << "fydeos assistant accelerator pressed twice, but clipboard not updated, so the bubble should be shown later";
    should_show_bubble_delay_ = true;
    return false;
  }
  should_show_bubble_delay_ = false;
  VLOG(3) << "can handle fydeos assistant accelerator: " << repeated;
  return repeated;
}

} // namespace ash
