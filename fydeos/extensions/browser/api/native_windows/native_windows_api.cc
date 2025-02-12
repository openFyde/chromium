// Copyright (c) 2018 The FlintOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// Author: Simon Tsao(yang@flintos.io)

#include "fydeos/extensions/browser/api/native_windows/native_windows_api.h"

#include <stddef.h>
#include <algorithm>
#include <limits>
#include <utility>
#include <vector>
#include "chrome/browser/extensions/api/tabs/tabs_constants.h"
#include "chrome/browser/extensions/extension_tab_util.h"
#include "fydeos/extensions/common/api/native_windows.h"
#include "chrome/common/extensions/extension_constants.h"
#include "chromeos/ui/base/window_pin_type.h"
#include "ash/public/cpp/window_properties.h"
#include "chrome/browser/ui/browser_command_controller.h"
#include "ui/aura/window.h"
#include "ui/display/display.h"
#include "ui/display/screen.h"
#include "ash/shell.h"
#include "ash/public/cpp/shelf_types.h"
#include "ash/public/cpp/shell_window_ids.h"
#include "ash/public/cpp/app_types_util.h"
#include "ash/wm/window_cycle/window_cycle_controller.h"
#include "ash/wm/window_cycle/window_cycle_list.h"
#include "ash/wm/window_state.h"
#include "ash/wm/window_util.h"
#include "ash/wm/window_state_util.h"
#include "ash/wm/mru_window_tracker.h"
#include "ash/wm/wm_event.h"
#include "ash/components/arc/arc_util.h"
#include "chrome/browser/ash/app_list/arc/arc_app_utils.h"
#include "chrome/browser/ui/ash/shelf/chrome_shelf_controller.h"
#include "chrome/browser/profiles/profile.h"
#include "ash/public/cpp/shelf_model.h"
#include "chrome/browser/apps/app_service/app_service_proxy_factory.h"
#include "chrome/browser/apps/app_service/app_service_proxy_ash.h"
#include "chrome/browser/apps/app_service/launch_utils.h"
#include "chrome/browser/ash/app_list/app_list_client_impl.h"

namespace extensions {
  using WindowList = ash::WindowCycleList::WindowList;
  namespace windows = api::native_windows;
  using Window = aura::Window;
  using WindowState = ash::WindowState;
  namespace keys = tabs_constants;
  using ShelfID = ash::ShelfID;

  // copied from chrome/browser/extensions/browser_extension_window_controller.cc
  // consider move them to header file, as in r126 or ealier version
  constexpr char kFocusedKey[] = "focused";
  constexpr char kHeightKey[] = "height";
  constexpr char kLeftKey[] = "left";
  constexpr char kShowStateKey[] = "state";
  constexpr char kTopKey[] = "top";
  constexpr char kWidthKey[] = "width";
  constexpr char kWindowTypeKey[] = "type";
  constexpr char kShowStateValueNormal[] = "normal";
  constexpr char kShowStateValueMinimized[] = "minimized";
  constexpr char kShowStateValueMaximized[] = "maximized";
  constexpr char kShowStateValueFullscreen[] = "fullscreen";
  constexpr char kShowStateValueLockedFullscreen[] = "locked-fullscreen";

  const char kWindowTypeValueApp[] = "app";

  const char kAppIdKey[] = "appId";
  const char kLaunchIdKey[] = "launchId";
  const char kAppIdIsEmptyError[] = "appId is empty.";

  namespace {
    int baseWindowId = ash::kShellWindowId_PhantomWindow + 2;
    void ArrangementWindowsIDs(WindowList window_list) {
      std::vector<Window*> need_id_list;
      for (Window * aura_window : window_list) {
        if (aura_window->GetId() == ash::kShellWindowId_Invalid)
          need_id_list.push_back(aura_window);
        else if (aura_window->GetId() >= baseWindowId)
          baseWindowId = aura_window->GetId() + 1;
      }
      for (Window * aura_window: need_id_list)
        aura_window->SetId(baseWindowId++);
    }

    WindowList BuildWindowsForCycleList() {
      WindowList window_list =
          ash::Shell::Get()->mru_window_tracker()->BuildMruWindowList(ash::kAllDesks);
      auto window_is_ineligible = [](Window* window) {
        WindowState* state = WindowState::Get(window);
        return !state->IsUserPositionable() || state->is_dragged() ||
                window->GetRootWindow()
                    ->GetChildById(ash::kShellWindowId_AppListContainer)
                    ->Contains(window) ||
                window->GetProperty(ash::kHideInOverviewKey);
      };
      window_list.erase(std::remove_if(window_list.begin(), window_list.end(),
                                        window_is_ineligible),
                        window_list.end());
      ArrangementWindowsIDs(window_list);
      return window_list;
    }

    base::Value CreateWindowValueFromNativeWindow(Window* aura_window)
    {
      base::Value::Dict result;
      result.Set(extension_misc::kId, aura_window->GetId());
      result.Set(kWindowTypeKey, kWindowTypeValueApp);
      ShelfID shelfId = ShelfID::Deserialize(aura_window->GetProperty(ash::kShelfIDKey));
      if (!shelfId.IsNull()) {
        result.Set(kAppIdKey, shelfId.app_id);
        result.Set(kLaunchIdKey, shelfId.launch_id);
      }
      std::string window_state;
      WindowState* aura_window_state = WindowState::Get(aura_window);
      if (aura_window_state->IsMinimized()) {
        window_state = kShowStateValueMinimized;
      } else if (aura_window_state->IsFullscreen()) {
        window_state = kShowStateValueFullscreen;
        if (aura_window_state->IsPinned() || aura_window_state->IsTrustedPinned())
          window_state = kShowStateValueLockedFullscreen;
      } else if (aura_window_state->IsMaximized()) {
        window_state = kShowStateValueMaximized;
      } else {
        window_state = kShowStateValueNormal;
      }
      result.Set(kShowStateKey, window_state);
      result.Set(kFocusedKey, aura_window_state->IsActive());
      gfx::Rect bounds = aura_window->GetBoundsInScreen();
      result.Set(kLeftKey, bounds.x());
      result.Set(kTopKey, bounds.y());
      result.Set(kWidthKey, bounds.width());
      result.Set(kHeightKey, bounds.height());

      return base::Value(std::move(result));
    }

    bool GetWindowFromWindowID(int window_id, Window** window) {
      WindowList window_list = BuildWindowsForCycleList();
      for (Window * tmp_window : window_list){
        if (tmp_window->GetId() == window_id){
          *window = tmp_window;
          break;
        }
      }
      return *window;
    }
/*
    int64_t GetDisplayIdForCurrentProfile() {
      // Settings in secondary profile cannot access ARC.
      return display::Screen::GetScreen()
          ->GetDisplayNearestView(web_ui()->GetWebContents()->GetNativeView())
          .id();
    }
*/
  } // exit internel namespace

  ExtensionFunction::ResponseAction NativeWindowsGetFunction::Run() {
    std::optional<windows::Get::Params> params(
        windows::Get::Params::Create(args()));
    EXTENSION_FUNCTION_VALIDATE(params);
    Window* window = nullptr;
    if (!GetWindowFromWindowID(params->window_id,
                                              &window)) {
      return RespondNow(Error(ExtensionTabUtil::kNoCurrentWindowError));
    }
    return RespondNow(WithArguments(CreateWindowValueFromNativeWindow(window)));
  }

  ExtensionFunction::ResponseAction NativeWindowsGetAllFunction::Run() {

    base::Value::List window_list;

    for(Window* tmp_window: BuildWindowsForCycleList())
      window_list.Append(CreateWindowValueFromNativeWindow(tmp_window));
    return RespondNow(WithArguments(std::move(window_list)));
  }


  ExtensionFunction::ResponseAction NativeWindowsUpdateFunction::Run() {
    std::optional<windows::Update::Params> params(
        windows::Update::Params::Create(args()));
    EXTENSION_FUNCTION_VALIDATE(params);

    Window* target_window = nullptr;
    if (!GetWindowFromWindowID(params->window_id, &target_window))
      return RespondNow(Error(ExtensionTabUtil::kNoCurrentWindowError));
    WindowState* target_window_state = WindowState::Get(target_window);
    if (params->update_info.state != windows::WindowState::kLockedFullscreen &&
        params->update_info.state != windows::WindowState::kNone) {
      target_window_state->Restore();
    } else if (params->update_info.state ==
                  windows::WindowState::kLockedFullscreen) {
      if (!target_window_state->IsMaximizedOrFullscreenOrPinned()){
        const ash::WMEvent event(ash::WM_EVENT_TOGGLE_FULLSCREEN);
        target_window_state->OnWMEvent(&event);
      }
      ash::window_util::PinWindow(target_window, true);
    }
    return RespondNow(WithArguments(CreateWindowValueFromNativeWindow(target_window)));
  }

  ExtensionFunction::ResponseAction NativeWindowsRemoveFunction::Run() {
    std::optional<windows::Remove::Params> params(
      windows::Remove::Params::Create(args()));
    EXTENSION_FUNCTION_VALIDATE(params);
    Window* target_window = nullptr;
    if (!GetWindowFromWindowID(params->window_id, &target_window))
      return RespondNow(Error(ExtensionTabUtil::kNoCurrentWindowError));

    if (ash::IsArcWindow(target_window)){
      auto task_id = arc::GetWindowTaskId(target_window);
      if (!task_id.has_value())
        return RespondNow(Error(ExtensionTabUtil::kNoCurrentWindowError));
      arc::CloseTask(*task_id);
    }else{
      ash::window_util::CloseWidgetForWindow(target_window);
    }
    return RespondNow(NoArguments());
  }

  ExtensionFunction::ResponseAction NativeWindowsCreateFunction::Run() {
    std::optional<windows::Create::Params> params(
      windows::Create::Params::Create(args()));
    EXTENSION_FUNCTION_VALIDATE(params);
    if(params->app_id.empty())
      return RespondNow(Error(kAppIdIsEmptyError));
    if (params->app_id == arc::kSettingsAppId) {
      arc::LaunchApp(browser_context(),
                     arc::kSettingsAppId,
                     ui::EF_NONE,
                     arc::UserInteractionType::APP_STARTED_FROM_SETTINGS
                     );
    } else {
      Profile* profile = Profile::FromBrowserContext(browser_context());
      AppListClientImpl* client = AppListClientImpl::GetInstance();
      apps::AppServiceProxyAsh* proxy =
        apps::AppServiceProxyFactory::GetForProfile(profile);
      int event_flags = ui::EF_NONE;
      bool is_active_app = false;
      proxy->AppRegistryCache()
          .ForOneApp(params->app_id, [&is_active_app](const apps::AppUpdate& update) {
            if (update.AppType() == apps::AppType::kCrostini ||
                ((update.AppType() == apps::AppType::kExtension ||
                  update.AppType() == apps::AppType::kSystemWeb ||
                  update.AppType() == apps::AppType::kWeb) &&
                 update.IsPlatformApp().value_or(true))) {
              is_active_app = true;
            }
          });
      if (is_active_app) {
        ash::ShelfID shelf_id(params->app_id);
        ash::ShelfModel* model = ChromeShelfController::instance()->shelf_model();
        ash::ShelfItemDelegate* delegate = model->GetShelfItemDelegate(shelf_id);
        if (delegate) {
          delegate->ItemSelected(
              /*event=*/nullptr, client->GetAppListDisplayId(),
              ash::LAUNCH_FROM_APP_LIST, /*callback=*/base::DoNothing(),
              /*filter_predicate=*/base::NullCallback());
        }
      } else {
        proxy->Launch(params->app_id, event_flags, apps::LaunchSource::kFromAppListGrid,
                  std::make_unique<apps::WindowInfo>(client->GetAppListDisplayId()));
      }
    }
    return RespondNow(NoArguments());
  }

} // exit namesapce extensions
