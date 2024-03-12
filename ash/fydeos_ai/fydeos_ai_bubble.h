#ifndef ASH_FYDEOS_AI_FYDEOS_AI_BUBBLE_H_
#define ASH_FYDEOS_AI_FYDEOS_AI_BUBBLE_H_

#include "ash/ash_export.h"
#include "ash/fydeos_ai/fydeos_ai_view.h"
#include "ui/views/bubble/bubble_dialog_delegate_view.h"
#include "ash/public/cpp/ash_web_view.h"

namespace aura {
class Window;
}

namespace ash {

class ASH_EXPORT FydeAssistantBubble : public views::BubbleDialogDelegateView,
                                       public AshWebView::Observer {
 public:
  explicit FydeAssistantBubble(aura::Window* window, const gfx::Rect& anchor_rect);

  FydeAssistantBubble(const FydeAssistantBubble&) = delete;
  FydeAssistantBubble& operator=(const FydeAssistantBubble&) =
      delete;
  ~FydeAssistantBubble() override;

  void OpenWebView(FydeAssistantView* owner);

  // views::BubbleDialogDelegateView:
  void Init() override;
  gfx::Size CalculatePreferredSize() const override;
  void OnThemeChanged() override;

 private:
  // AshWebView::Observer:
  void DidStopLoading() override;

  AshWebView* WebView();
  void OpenUrl(const GURL& url);

  std::unique_ptr<AshWebView> web_view_;
  raw_ptr<AshWebView, ExperimentalAsh> web_view_ptr_ = nullptr;
  raw_ptr<FydeAssistantView, ExperimentalAsh> owner_ = nullptr;
};

}

#endif // !ASH_FYDEOS_AI_FYDEOS_AI_BUBBLE_H_
