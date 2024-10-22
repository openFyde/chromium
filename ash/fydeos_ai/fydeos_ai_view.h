#ifndef ASH_FYDEOS_AI_FYDEOS_AI_VIEW_H_
#define ASH_FYDEOS_AI_FYDEOS_AI_VIEW_H_

#include <memory>
#include "ash/ash_export.h"
#include "ash/clipboard/clipboard_history_item.h"
#include "base/memory/raw_ptr.h"
#include "ash/public/cpp/session/session_observer.h"
#include "base/observer_list.h"
#include "base/observer_list_types.h"
#include "ui/events/event.h"
#include "ui/events/event_handler.h"
#include "ash/public/cpp/assistant/assistant_state.h"

namespace views {
class View;
}

namespace aura {
class Window;
}

namespace ash {

class FydeAssistantViewObserver : public base::CheckedObserver  {
 public:
  struct ClipboardItemForAssistant {
    std::u16string display_text;
    int            display_format;
  };

  FydeAssistantViewObserver(const FydeAssistantViewObserver&) = delete;
  FydeAssistantViewObserver& operator=(const FydeAssistantViewObserver&) = delete;

  virtual void OnBubbleQueryChanged(const ClipboardItemForAssistant& item) {}
  virtual void OnBubbleVisibilityChanged(bool visible) {}
 protected:
  FydeAssistantViewObserver() = default;
  ~FydeAssistantViewObserver() override = default;
};

class FydeAssistantBubble;

class ASH_EXPORT FydeAssistantView : public SessionObserver, public ui::EventHandler,
                                     public AssistantStateObserver {
 public:
  explicit FydeAssistantView(aura::Window* container);

  FydeAssistantView(const FydeAssistantView&) = delete;
  FydeAssistantView& operator=(const FydeAssistantView&) = delete;

  ~FydeAssistantView() override;

  void AddObserver(FydeAssistantViewObserver* observer) const;
  void RemoveObserver(FydeAssistantViewObserver* observer) const;

  bool IsVisible() const;

  void ShowBubble(bool update_anchor_point = true);
  void HideBubble();

  void UpdateLastClipboardItem(const ClipboardHistoryItem& item);
  bool CanHandleToggleFydeOSAssistant();

  bool CanHandleTouchSelectionMenuAction();
  void HandleSendTextToAI(const gfx::Rect& anchor_rect, const std::u16string& text);

  void OnBubbleReady();

  void SetBubbleRect(int x, int y, int width, int height);

  void CenterBubble(int width, int height);

 private:
  void OnSessionStateChanged(session_manager::SessionState state) override;

  void OnMouseEvent(ui::MouseEvent* event) override;
  void OnTouchEvent(ui::TouchEvent* event) override;

  void ProcessPressedEvent(ui::LocatedEvent* event);

  void OnFydeAssistantExtraAcceleratorEnabled(bool enabled) override;

  void Show(bool update_anchor_point);
  void Hide();

  void InitializeBubble();
  void ScheduleInitializeBubble();

  bool init_scheduled_ = false;
  bool enabled_ = false;
  bool ready_to_show_bubble_ = false;
  bool bubble_initialized_ = false;
  bool should_show_bubble_delay_ = false;
  base::TimeTicks last_clipboard_item_time_ = base::TimeTicks::Min();
  base::TimeTicks last_time_triggered_ = base::TimeTicks::Min();
  FydeAssistantViewObserver::ClipboardItemForAssistant last_clipboard_item_;
  raw_ptr<FydeAssistantBubble, DanglingUntriaged> bubble_;

  gfx::Point current_anchor_point_;

  mutable base::ObserverList<FydeAssistantViewObserver> observers_;

base::WeakPtrFactory<FydeAssistantView> weak_factory_{this};
};

}  // namespace ash

#endif // !ASH_FYDEOS_AI_FYDEOS_AI_VIEW_H_
