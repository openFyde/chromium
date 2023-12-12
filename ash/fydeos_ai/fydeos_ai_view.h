#ifndef ASH_FYDEOS_AI_FYDEOS_AI_VIEW_H_
#define ASH_FYDEOS_AI_FYDEOS_AI_VIEW_H_

#include <memory>
#include "ash/ash_export.h"
#include "ash/clipboard/clipboard_history_item.h"
#include "base/memory/raw_ptr.h"
#include "ash/public/cpp/session/session_observer.h"
#include "base/observer_list.h"
#include "base/observer_list_types.h"
#include "ui/views/widget/widget_observer.h"

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

class ASH_EXPORT FydeAssistantView : public SessionObserver, public views::WidgetObserver {
 public:
  FydeAssistantView();

  FydeAssistantView(const FydeAssistantView&) = delete;
  FydeAssistantView& operator=(const FydeAssistantView&) = delete;

  ~FydeAssistantView() override;

  void AddObserver(FydeAssistantViewObserver* observer) const;
  void RemoveObserver(FydeAssistantViewObserver* observer) const;

  void CreateAssistantWidget(aura::Window* window);

  void ShowBubble();
  void HideBubble();

  void UpdateLastClipboardItem(const ClipboardHistoryItem& item);
  bool CanHandleToggleFydeOSAssistant();

  void OnBubbleReady();

  void SetBubbleRect(int x, int y, int width, int height);

 private:
  void OnSessionStateChanged(session_manager::SessionState state) override;

  void OnWidgetActivationChanged(views::Widget* widget, bool active) override;
  void OnWidgetClosing(views::Widget* widget) override;

  void Show();
  void Hide();

  bool visible_ = false;
  bool can_show_ = false;
  bool should_show_bubble_delay_ = false;
  base::TimeTicks last_clipboard_item_time_ = base::TimeTicks::Min();
  base::TimeTicks last_time_triggered_ = base::TimeTicks::Min();
  FydeAssistantViewObserver::ClipboardItemForAssistant last_clipboard_item_;
  aura::Window* window_ = nullptr;
  raw_ptr<FydeAssistantBubble, ExperimentalAsh> bubble_;

  gfx::Point current_anchor_point_ = gfx::Point();

  mutable base::ObserverList<FydeAssistantViewObserver> observers_;
};

}  // namespace ash

#endif // !ASH_FYDEOS_AI_FYDEOS_AI_VIEW_H_
