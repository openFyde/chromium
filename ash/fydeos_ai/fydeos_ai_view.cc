// Copyright 2023 The Fyde Innovations Limited. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ash/fydeos_ai/fydeos_ai_view.h"
#include "base/check.h"
#include "base/logging.h"
#include "base/memory/ptr_util.h"
#include "base/time/time.h"
#include "ui/events/types/event_type.h"
#include "ui/views/background.h"
#include "ui/views/controls/label.h"
#include "ash/shell.h"
#include "ash/session/session_controller_impl.h"
#include "ash/public/cpp/ash_web_view_factory.h"
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

void FydeAssistantView::AddObserver(FydeAssistantViewObserver* observer) const {
  observers_.AddObserver(observer);
}
void FydeAssistantView::RemoveObserver(FydeAssistantViewObserver* observer) const {
  observers_.RemoveObserver(observer);
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
  if (event->type() == ui::EventType::kMousePressed) {
    ProcessPressedEvent(event->AsLocatedEvent());
  }
}

void FydeAssistantView::HideBubble() {
  if (!IsVisible()) {
    return;
  }
  Hide();
}

void FydeAssistantView::Show() {
  if (!ready_to_show_bubble_) {
    return;
  }
  if (!bubble_) {
    return;
  }
  current_anchor_point_ = display::Screen::GetScreen()->GetCursorScreenPoint();
  bubble_->SetAnchorRect(gfx::Rect(current_anchor_point_, gfx::Size()));
  bubble_->SetPreferredSize(bubble_->CalculatePreferredSize({}));
  bubble_->GetWidget()->Show();
  for (auto& observer : observers_) {
    observer.OnBubbleVisibilityChanged(IsVisible());
    observer.OnBubbleQueryChanged(last_clipboard_item_);
  }
}

void FydeAssistantView::Hide() {
  if (!bubble_) {
    return;
  }
  bubble_->SetPreferredSize(bubble_->CalculatePreferredSize({}));
  bubble_->GetWidget()->Hide();
  for (auto& observer : observers_) {
    observer.OnBubbleVisibilityChanged(IsVisible());
  }
}

void FydeAssistantView::OnSessionStateChanged(session_manager::SessionState state) {
  if (state == session_manager::SessionState::ACTIVE) {
    bubble_ = new FydeAssistantBubble(gfx::Rect(display::Screen::GetScreen()->GetCursorScreenPoint(), gfx::Size()));
    bubble_->InitWebView(this);
  }
}

void FydeAssistantView::UpdateLastClipboardItem(const ClipboardHistoryItem& item) {
  VLOG(3) << "clipboard update";
  last_clipboard_item_time_ = base::TimeTicks::Now();
  last_clipboard_item_.display_format = static_cast<int>(item.display_format());
  last_clipboard_item_.display_text = item.display_text();
  // copy from subsystem can be slow,  slower than the gap between pressing ctrl+c and c twice
  auto now = base::TimeTicks::Now();
  if (now - last_time_triggered_ < base::Seconds(1) && should_show_bubble_delay_) {
    VLOG(3) << "pressed twice and clipboard updated, show the bubble now";
    ShowBubble();
  }
}

bool FydeAssistantView::CanHandleToggleFydeOSAssistant() {
  if (!ready_to_show_bubble_) {
    return false;
  }
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

void FydeAssistantView::OnBubbleReady() {
  ready_to_show_bubble_ = true;
}

void FydeAssistantView::SetBubbleRect(int x, int y, int width, int height) {
  if (bubble_) {
    VLOG(3) << "set bubble rect: " << x << ", " << y << ", " << width << ", " << height;
    auto current = bubble_->GetWidget()->GetWindowBoundsInScreen();
    auto newRect = gfx::Rect(
      x > 0 ? x : current.x(),
      y > 0 ? y : current.y(),
      width > 0 ? width : current.width(),
      height > 0 ? height : current.height()
    );
    bubble_->SetPreferredSize(gfx::Size(newRect.width(), newRect.height()));
    bubble_->SetAnchorRect(gfx::Rect(current_anchor_point_, gfx::Size()));
  }
}

} // namespace ash
