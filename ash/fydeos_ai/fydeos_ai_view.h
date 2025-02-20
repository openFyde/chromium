#ifndef ASH_FYDEOS_AI_FYDEOS_AI_VIEW_H_
#define ASH_FYDEOS_AI_FYDEOS_AI_VIEW_H_

#include <memory>
#include "ash/ash_export.h"
#include "ash/clipboard/clipboard_history_item.h"
#include "base/memory/raw_ptr.h"
#include "ash/public/cpp/session/session_observer.h"
#include "ui/events/event.h"
#include "ui/events/event_handler.h"

namespace views {
class View;
}

namespace aura {
class Window;
}

namespace ash {

class FydeAssistantBubble;

class ASH_EXPORT FydeAssistantView : public SessionObserver, public ui::EventHandler {
 public:
  explicit FydeAssistantView(aura::Window* container);

  FydeAssistantView(const FydeAssistantView&) = delete;
  FydeAssistantView& operator=(const FydeAssistantView&) = delete;

  ~FydeAssistantView() override;

  bool IsVisible() const;

  void ShowBubble();

  void UpdateLastClipboardItem(const ClipboardHistoryItem& item);
  bool CanHandleToggleFydeOSAssistant();

 private:
  void OnSessionStateChanged(session_manager::SessionState state) override;

  void OnMouseEvent(ui::MouseEvent* event) override;
  void OnTouchEvent(ui::TouchEvent* event) override;

  void ProcessPressedEvent(ui::LocatedEvent* event);

  void Show();
  void Hide();

  bool can_show_ = false;
  bool should_show_bubble_delay_ = false;
  base::TimeTicks last_clipboard_item_time_ = base::TimeTicks::Min();
  base::TimeTicks last_time_triggered_ = base::TimeTicks::Min();
  std::u16string last_clipboard_display_text_;
  raw_ptr<FydeAssistantBubble, DanglingUntriaged> bubble_;
};

}  // namespace ash

#endif // !ASH_FYDEOS_AI_FYDEOS_AI_VIEW_H_
