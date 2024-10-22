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
  static bool ReadyToInit();

  explicit FydeAssistantBubble(const gfx::Rect& anchor_rect);

  FydeAssistantBubble(const FydeAssistantBubble&) = delete;
  FydeAssistantBubble& operator=(const FydeAssistantBubble&) =
      delete;
  ~FydeAssistantBubble() override;

  bool InitWebView(FydeAssistantView* owner);

  // views::BubbleDialogDelegateView:
  gfx::Size CalculatePreferredSize(const views::SizeBounds& available_size) const override;

 private:
  void OnThemeChanged() override;
  // AshWebView::Observer:
  void DidStopLoading() override;

  bool OpenUrl(const GURL& url);

  std::unique_ptr<AshWebView> web_view_;
  raw_ptr<AshWebView, DanglingUntriaged> web_view_ptr_ = nullptr;
  raw_ptr<FydeAssistantView> owner_ = nullptr;
};

}

#endif // !ASH_FYDEOS_AI_FYDEOS_AI_BUBBLE_H_
