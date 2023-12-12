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

namespace ash {

FydeAssistantView::FydeAssistantView() {
  Shell::Get()->session_controller()->AddObserver(this);
}

FydeAssistantView::~FydeAssistantView() {
  Shell::Get()->session_controller()->RemoveObserver(this);
  if (bubble_) {
    bubble_->GetWidget()->RemoveObserver(this);
    bubble_ = nullptr;
  }
}

void FydeAssistantView::AddObserver(FydeAssistantViewObserver* observer) const {
  observers_.AddObserver(observer);
}
void FydeAssistantView::RemoveObserver(FydeAssistantViewObserver* observer) const {
  observers_.RemoveObserver(observer);
}

void FydeAssistantView::ShowBubble() {
  if (visible_) {
    return;
  }
  Show();
}

void FydeAssistantView::HideBubble() {
  if (!visible_) {
    return;
  }
  Hide();
}

void FydeAssistantView::Show() {
  if (!can_show_) {
    return;
  }
  DCHECK(bubble_);
  if (bubble_->GetWidget() == nullptr) {
    NOTREACHED();
    return;
  }
  current_anchor_point_ = display::Screen::GetScreen()->GetCursorScreenPoint();
  bubble_->SetAnchorRect(gfx::Rect(current_anchor_point_, gfx::Size()));
  bubble_->SetPreferredSize(bubble_->CalculatePreferredSize());
  bubble_->GetWidget()->Show();
  visible_ = true;

  for (auto& observer : observers_) {
    observer.OnBubbleVisibilityChanged(visible_);
    observer.OnBubbleQueryChanged(last_clipboard_item_);
  }
}

void FydeAssistantView::Hide() {
  if (!bubble_) {
    return;
  }
  bubble_->SetPreferredSize(bubble_->CalculatePreferredSize());
  bubble_->GetWidget()->Hide();
  visible_ = false;

  for (auto& observer : observers_) {
    observer.OnBubbleVisibilityChanged(visible_);
  }
}

void FydeAssistantView::CreateAssistantWidget(aura::Window* window) {
  window_ = window;
}

void FydeAssistantView::OnWidgetActivationChanged(views::Widget* widget, bool active) {
  DCHECK(widget == bubble_->GetWidget());
  if (!active) {
    Hide();
  }
}

void FydeAssistantView::OnWidgetClosing(views::Widget* widget) {
  //  not reached if set_close_on_deactivate(false)
  NOTREACHED();
  DCHECK(widget == bubble_->GetWidget());
  bubble_->GetWidget()->RemoveObserver(this);
  bubble_ = nullptr;
  visible_ = false;
}

void FydeAssistantView::OnSessionStateChanged(session_manager::SessionState state) {
  if (state == session_manager::SessionState::ACTIVE) {
    if (!bubble_) {
      bubble_ = new FydeAssistantBubble(window_,
                                        gfx::Rect(display::Screen::GetScreen()->GetCursorScreenPoint(), gfx::Size()));
      bubble_->OpenWebView(this);
      bubble_->GetWidget()->AddObserver(this);
    }
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
  if (!can_show_) return false;
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
  can_show_ = true;
}

void FydeAssistantView::SetBubbleRect(int x, int y, int width, int height) {
  if (bubble_) {
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
