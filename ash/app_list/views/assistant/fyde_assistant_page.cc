#include "ash/app_list/views/assistant/fyde_assistant_page.h"
#include "ash/public/cpp/assistant/controller/assistant_ui_controller.h"
#include "ash/assistant/model/assistant_ui_model.h"
#include "ui/views/border.h"
#include "ui/views/layout/fill_layout.h"
#include "ash/public/cpp/ash_web_view_factory.h"
#include "ui/base/metadata/metadata_impl_macros.h"
#include "ash/assistant/util/deep_link_util.h"

namespace ash {

namespace {
const char kFydeAssistantExtensionUrl[] = "chrome://fydeos-ai/?source=launcher";
constexpr int kHeightDip = 440;
} // namespace

FydeAssistantPage::FydeAssistantPage() {
  InitLayout();
  assistant_controller_observation_.Observe(AssistantController::Get());
  AssistantUiController::Get()->GetModel()->AddObserver(this);
}

FydeAssistantPage::~FydeAssistantPage() {
  if (AssistantUiController::Get())
    AssistantUiController::Get()->GetModel()->RemoveObserver(this);
}

void FydeAssistantPage::OnBoundsChanged(
    const gfx::Rect& previous_bounds) {
  if (AssistantUiController::Get())
    AssistantUiController::Get()->SetAppListBubbleWidth(size().width());
}

void FydeAssistantPage::RequestFocus() {
  VLOG(2) << "FydeAssistantPage::RequestFocus()";
  if (web_view_ptr_) {
    web_view_ptr_->GetInitiallyFocusedView()->RequestFocus();
  }
}

void FydeAssistantPage::InitLayout() {
  SetLayoutManager(std::make_unique<views::FillLayout>());
}

AshWebView* FydeAssistantPage::WebView() {
  return web_view_ptr_ ? web_view_ptr_.get() : web_view_.get();
}

void FydeAssistantPage::OnDeepLinkReceived(
    assistant::util::DeepLinkType type,
    const std::map<std::string, std::string>& params) {
  if (type == assistant::util::DeepLinkType::kQuery) {
    const absl::optional<std::string>& query =
        GetDeepLinkParam(params, assistant::util::DeepLinkParam::kQuery);

    if (!query.has_value())
      return;
    AssistantUiController::Get()->ShowUi(assistant::AssistantEntryPoint::kDeepLink);
    OpenUrl(GURL(base::StringPrintf("%s&initQuery=%s", kFydeAssistantExtensionUrl, query->c_str())));
  }
}

void FydeAssistantPage::OnUiVisibilityChanged(
      AssistantVisibility new_visibility,
      AssistantVisibility old_visibility,
      absl::optional<AssistantEntryPoint> entry_point,
      absl::optional<AssistantExitPoint> exit_point) {
  if (new_visibility == AssistantVisibility::kVisible && entry_point.has_value() && (entry_point == AssistantEntryPoint::kHotkey || entry_point == AssistantEntryPoint::kLauncherSearchBoxIcon)) {
    OpenUrl(GURL(kFydeAssistantExtensionUrl));
  }
}

void FydeAssistantPage::InitializeUIForBubbleView() {
  // called from AppListBubbleView::InitializeUIForBubbleView
  VLOG(2) << "FydeAssistantPage::InitializeUIForBubbleView()";
}

gfx::Size FydeAssistantPage::CalculatePreferredSize() const {
  return gfx::Size(INT_MAX, kHeightDip);
}

void FydeAssistantPage::OpenUrl(const GURL& url) {
  if (web_view_ptr_ || web_view_) {
    return;
  }
  // RemoveContents();
  web_view_ = AshWebViewFactory::Get()->Create(AshWebView::InitParams());
  WebView()->AddObserver(this);
  WebView()->Navigate(url);
}

void FydeAssistantPage::DidStopLoading() {
  if (!web_view_) {
    return;
  }

  web_view_->SetPreferredSize(GetPreferredSize());
  web_view_ptr_ = AddChildView(std::move(web_view_));
  web_view_ptr_->SetBorder(views::CreateEmptyBorder(0));
  web_view_ptr_->GetInitiallyFocusedView()->RequestFocus();
}

void FydeAssistantPage::RemoveContents() {
  if (!web_view_ptr_) {
    return;
  }
  RemoveChildViewT(web_view_ptr_.get())->RemoveObserver(this);
  web_view_ptr_ = nullptr;
}

BEGIN_METADATA(FydeAssistantPage, views::View)
END_METADATA

} // namespace ash
