#ifndef ASH_FYDEOS_AI_FYDEOS_AI_BUBBLE_H_
#define ASH_FYDEOS_AI_FYDEOS_AI_BUBBLE_H_

#include "ash/ash_export.h"
#include "ash/fydeos_ai/fydeos_ai_view.h"
#include "ui/views/bubble/bubble_dialog_delegate_view.h"

namespace aura {
class Window;
}

namespace ash {

class ASH_EXPORT FydeAssistantBubble : public views::BubbleDialogDelegateView {
 public:
  explicit FydeAssistantBubble(const gfx::Rect& anchor_rect);

  FydeAssistantBubble(const FydeAssistantBubble&) = delete;
  FydeAssistantBubble& operator=(const FydeAssistantBubble&) =
      delete;
  ~FydeAssistantBubble() override;

  void UpdateContent(std::u16string text);

 private:
  // views::BubbleDialogDelegateView:
  gfx::Size CalculatePreferredSize(const views::SizeBounds& available_size) const override;
  void OnThemeChanged() override;

  views::Label* label_ = nullptr;

};

}

#endif // !ASH_FYDEOS_AI_FYDEOS_AI_BUBBLE_H_
