// Copyright 2023 The Fyde Innovations Limited. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ash/fydeos_ai/fydeos_ai_view.h"
#include "base/check.h"
#include "base/functional/bind.h"
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
#include "components/prefs/pref_change_registrar.h"
#include "components/prefs/pref_service.h"
#include "fydeos/prefs/fydeos_pref_names.h"
#include "ui/wm/core/coordinate_conversion.h"
#include "base/task/task_traits.h"
#include "base/task/single_thread_task_runner.h"

namespace ash {

namespace {
const int kBubbleInitDelaySeconds = 5;
}

FydeAssistantView::FydeAssistantView(aura::Window* container): window_(container) {
  Shell::Get()->session_controller()->AddObserver(this);
  Shell::Get()->AddPreTargetHandler(this);
  if (PrefService* prefs =
          Shell::Get()->session_controller()->GetActivePrefService()) {
    OnActiveUserPrefServiceChanged(prefs);
  }
}

FydeAssistantView::~FydeAssistantView() {
  if (Shell::Get()) {
    Shell::Get()->RemovePreTargetHandler(this);
    Shell::Get()->session_controller()->RemoveObserver(this);
  }
  pref_change_registrar_.reset();
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

void FydeAssistantView::ScheduleInitializeBubble() {
  if (bubble_initialized_ || init_scheduled_) return;
  if (FydeAssistantBubble::ReadyToInit()) {
    InitializeBubble();
  } else {
    VLOG(2) << "Not ready to init bubble, schedule it, after " << kBubbleInitDelaySeconds << " seconds";
    base::SingleThreadTaskRunner::GetCurrentDefault()->PostDelayedTask(
        FROM_HERE,
        base::BindOnce(&FydeAssistantView::InitializeBubble,
                      weak_factory_.GetWeakPtr()),
        base::Seconds(kBubbleInitDelaySeconds));
    init_scheduled_ = true;
  }
}

void FydeAssistantView::ShowBubble(bool update_anchor_point) {
  if (!enabled_) {
    return;
  }
  if (IsVisible()) {
    return;
  }
  Show(update_anchor_point);
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

void FydeAssistantView::ResetDragStartPoint(ui::LocatedEvent* event) {
  if (is_dragging_) {
    is_dragging_ = false;
    event->SetHandled();
  }
  drag_start_point_ = gfx::Point();
}

void FydeAssistantView::ProcessDraggedEvent(ui::LocatedEvent* event) {
  if (!IsVisible()) return;
  gfx::Point screen_location = event->location();
  ::wm::ConvertPointToScreen(static_cast<aura::Window*>(event->target()),
                             &screen_location);
  if (!bubble_->GetBoundsInScreen().Contains(screen_location)) {
    return;
  }
  gfx::Rect widget_bounds = bubble_->GetWidget()->GetWindowBoundsInScreen();
  if (!drag_start_point_.IsOrigin()) {
    widget_bounds.Offset(screen_location - drag_start_point_);
    bubble_->GetWidget()->SetBounds(widget_bounds);
    is_dragging_ = true;
    event->SetHandled();
  }
  drag_start_point_ = screen_location;
}

void FydeAssistantView::OnTouchEvent(ui::TouchEvent* event) {
  if (event->type() == ui::EventType::kTouchPressed) {
    ProcessPressedEvent(event->AsLocatedEvent());
  } else if (event->type() == ui::EventType::kTouchMoved) {
    ProcessDraggedEvent(event->AsLocatedEvent());
  } else if (event->type() == ui::EventType::kTouchReleased || event->type() == ui::EventType::kTouchCancelled) {
    ResetDragStartPoint(event->AsLocatedEvent());
  }
}

void FydeAssistantView::OnMouseEvent(ui::MouseEvent* event) {
  if (event->type() == ui::EventType::kMousePressed) {
    ProcessPressedEvent(event->AsLocatedEvent());
  } else if (event->type() == ui::EventType::kMouseDragged) {
    ProcessDraggedEvent(event->AsLocatedEvent());
  } else if (event->type() == ui::EventType::kMouseReleased) {
    ResetDragStartPoint(event->AsLocatedEvent());
  }
}

void FydeAssistantView::HideBubble() {
  if (!IsVisible()) {
    return;
  }
  Hide();
}

void FydeAssistantView::Show(bool update_anchor_point) {
  if (!ready_to_show_bubble_) {
    return;
  }
  if (!bubble_) {
    return;
  }
  if (update_anchor_point) {
    current_anchor_point_ = display::Screen::Get()->GetCursorScreenPoint();
  }
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
  if (enabled_ && state == session_manager::SessionState::ACTIVE) {
    ScheduleInitializeBubble();
  }
}

void FydeAssistantView::OnActiveUserPrefServiceChanged(
    PrefService* pref_service) {
  pref_change_registrar_ = std::make_unique<PrefChangeRegistrar>();
  pref_change_registrar_->Init(pref_service);
  pref_change_registrar_->Add(
      fydeos::prefs::kFydeAssistantEnabled,
      base::BindRepeating(&FydeAssistantView::UpdateFromPrefs,
                          base::Unretained(this)));
  pref_change_registrar_->Add(
      fydeos::prefs::kFydeAssistantExtraAcceleratorEnabled,
      base::BindRepeating(&FydeAssistantView::UpdateFromPrefs,
                          base::Unretained(this)));
  UpdateFromPrefs();
}

void FydeAssistantView::OnChromeTerminating() {
  if (bubble_) {
    bubble_->RemoveWebView();
  }
}

void FydeAssistantView::HandleSendTextToAI(const gfx::Rect& anchor_rect, const std::u16string& text) {
  if (!enabled_) {
    return;
  }
  if (text.empty()) {
    return;
  }
  last_clipboard_item_.display_format = static_cast<int>(crosapi::mojom::ClipboardHistoryDisplayFormat::kText);
  last_clipboard_item_.display_text = text;

  current_anchor_point_ = anchor_rect.origin();
  ShowBubble(false);
}

void FydeAssistantView::UpdateLastClipboardItem(const ClipboardHistoryItem& item) {
  if (!enabled_) {
    return;
  }
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

bool FydeAssistantView::CanHandleTouchSelectionMenuAction() {
  if (!enabled_) {
    return false;
  }
  if (!ready_to_show_bubble_) {
    return false;
  }
  return true;
}

bool FydeAssistantView::CanHandleToggleFydeOSAssistant() {
  if (!extra_accelerator_enabled_) {
    return false;
  }
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

void FydeAssistantView::InitializeBubble() {
  if (bubble_initialized_) {
    return;
  }
  bubble_ = new FydeAssistantBubble(
      window_,
      gfx::Rect(display::Screen::Get()->GetCursorScreenPoint(), gfx::Size()));
  bubble_->InitWebView(this);
  bubble_initialized_ = true;
}

void FydeAssistantView::UpdateFromPrefs() {
  if (!pref_change_registrar_) {
    return;
  }
  PrefService* prefs = pref_change_registrar_->prefs();
  if (!prefs ||
      !prefs->FindPreference(fydeos::prefs::kFydeAssistantEnabled) ||
      !prefs->FindPreference(
          fydeos::prefs::kFydeAssistantExtraAcceleratorEnabled)) {
    return;
  }
  if (!Shell::Get()->session_controller()->IsActiveUserSessionStarted()) {
    return;
  }
  enabled_ = prefs->GetBoolean(fydeos::prefs::kFydeAssistantEnabled);
  extra_accelerator_enabled_ =
      enabled_ &&
      prefs->GetBoolean(fydeos::prefs::kFydeAssistantExtraAcceleratorEnabled);
  if (!enabled_ && IsVisible()) {
    Hide();
  }
  if (enabled_ && Shell::Get() &&
      Shell::Get()->session_controller()->GetSessionState() ==
          session_manager::SessionState::ACTIVE) {
    ScheduleInitializeBubble();
  }
}

void FydeAssistantView::CenterBubble(int width, int height) {
  if (bubble_) {
    bubble_->GetWidget()->CenterWindow(gfx::Size(width, height));
  }
}

} // namespace ash
