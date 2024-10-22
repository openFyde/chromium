// Copyright 2016 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/views/widget/focus_manager_event_handler.h"

#include <string_view>

#include "ui/aura/window.h"
#include "ui/views/focus/focus_manager.h"
#include "ui/views/widget/widget.h"
#if BUILDFLAG(IS_CHROMEOS_ASH)
#include "ash/constants/ash_features.h"
#endif


namespace views {

FocusManagerEventHandler::FocusManagerEventHandler(Widget* widget,
                                                   aura::Window* window)
    : widget_(widget), window_(window) {
  DCHECK(window_);
  window_->AddPreTargetHandler(this);
}

FocusManagerEventHandler::~FocusManagerEventHandler() {
  window_->RemovePreTargetHandler(this);
}

void FocusManagerEventHandler::OnKeyEvent(ui::KeyEvent* event) {
  bool has_focused_view = widget_ && widget_->GetFocusManager() && widget_->GetFocusManager()->GetFocusedView() != nullptr;
#if BUILDFLAG(IS_CHROMEOS_ASH)
  bool special_key = event->IsControlDown() && event->key_code() == ui::VKEY_C;
#else
  bool special_key = false;
#endif
  // Ctrl+C must be handled, even if there's no focused_view, for AI interaction
  if (widget_ && (has_focused_view || special_key) &&
      !widget_->GetFocusManager()->OnKeyEvent(*event)) {
    if (!special_key) {
      // without this if, Ctrl+C won't be handled by subsystem like crostini and android
      event->StopPropagation();
    }
  }
}

std::string_view FocusManagerEventHandler::GetLogContext() const {
  return "FocusManagerEventHandler";
}

}  // namespace views
