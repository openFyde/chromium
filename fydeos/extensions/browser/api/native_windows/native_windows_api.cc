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
#include "ash/constants/app_types.h"
#include "ash/public/cpp/app_types_util.h"
#include "ash/wm/window_cycle/window_cycle_controller.h"
#include "ash/wm/window_cycle/window_cycle_list.h"
#include "ash/wm/window_state.h"
#include "ash/wm/window_util.h"
#include "ash/wm/window_state_util.h"
#include "ash/wm/mru_window_tracker.h"
#include "ash/wm/wm_event.h"
#include "ash/components/arc/arc_util.h"
#include "chrome/browser/ui/app_list/arc/arc_app_utils.h"
#include "chrome/browser/ui/ash/shelf/chrome_shelf_controller.h"
#include "chrome/browser/profiles/profile.h"
#include "ash/public/cpp/shelf_model.h"
#include "chrome/browser/apps/app_service/app_service_proxy_factory.h"
#include "chrome/browser/apps/app_service/app_service_proxy_ash.h"
#include "chrome/browser/apps/app_service/launch_utils.h"
#include "chrome/browser/ui/app_list/app_list_client_impl.h"

namespace extensions{
  using WindowList = ash::WindowCycleList::WindowList;
  namespace windows = api::native_windows;
  using Window = aura::Window;
  using WindowState = ash::WindowState;
  namespace keys = tabs_constants;
  using ShelfID = ash::ShelfID;

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
      base::Value result(base::Value::Type::DICTIONARY);
      result.SetIntKey(keys::kIdKey, aura_window->GetId());
      result.SetStringKey(keys::kWindowTypeKey, keys::kWindowTypeValueApp);
      ShelfID shelfId = ShelfID::Deserialize(aura_window->GetProperty(ash::kShelfIDKey));
      if (!shelfId.IsNull()) {
        result.SetStringKey(kAppIdKey, shelfId.app_id);
        result.SetStringKey(kLaunchIdKey, shelfId.launch_id);
      }
      std::string window_state;
      WindowState* aura_window_state = WindowState::Get(aura_window);
      if (aura_window_state->IsMinimized()) {
        window_state = keys::kShowStateValueMinimized;
      } else if (aura_window_state->IsFullscreen()) {
        window_state = keys::kShowStateValueFullscreen;
        if (aura_window_state->IsPinned() || aura_window_state->IsTrustedPinned())
          window_state = keys::kShowStateValueLockedFullscreen;
      } else if (aura_window_state->IsMaximized()) {
        window_state = keys::kShowStateValueMaximized;
      } else {
        window_state = keys::kShowStateValueNormal;
      }
      result.SetStringKey(keys::kShowStateKey, window_state);
      result.SetBoolKey(keys::kFocusedKey, aura_window_state->IsActive());
      gfx::Rect bounds = aura_window->GetBoundsInScreen();
      result.SetIntKey(keys::kLeftKey, bounds.x());
      result.SetIntKey(keys::kTopKey, bounds.y());
      result.SetIntKey(keys::kWidthKey, bounds.width());
      result.SetIntKey(keys::kHeightKey, bounds.height());

      return result;
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
    std::unique_ptr<windows::Get::Params> params(
        windows::Get::Params::Create(args()));
    EXTENSION_FUNCTION_VALIDATE(params.get());
    Window* window = NULL;
    if (!GetWindowFromWindowID(params->window_id,
                                              &window)) {
      return RespondNow(Error(keys::kNoCurrentWindowError));
    }
    return RespondNow(OneArgument(CreateWindowValueFromNativeWindow(window)));
  }

  ExtensionFunction::ResponseAction NativeWindowsGetAllFunction::Run() {

    base::Value window_list(base::Value::Type::LIST);

    for(Window* tmp_window: BuildWindowsForCycleList())
      window_list.Append(CreateWindowValueFromNativeWindow(tmp_window));
    return RespondNow(OneArgument(std::move(window_list)));
  }


  ExtensionFunction::ResponseAction NativeWindowsUpdateFunction::Run() {
    std::unique_ptr<windows::Update::Params> params(
        windows::Update::Params::Create(args()));
    EXTENSION_FUNCTION_VALIDATE(params);

    Window* target_window = NULL;
    if (!GetWindowFromWindowID(params->window_id, &target_window))
      return RespondNow(Error(keys::kNoCurrentWindowError));
    WindowState* target_window_state = WindowState::Get(target_window);
    if (params->update_info.state != windows::WINDOW_STATE_LOCKED_FULLSCREEN &&
        params->update_info.state != windows::WINDOW_STATE_NONE) {
      target_window_state->Restore();
    } else if (params->update_info.state ==
                  windows::WINDOW_STATE_LOCKED_FULLSCREEN) {
      if (!target_window_state->IsMaximizedOrFullscreenOrPinned()){
        const ash::WMEvent event(ash::WM_EVENT_TOGGLE_FULLSCREEN);
        target_window_state->OnWMEvent(&event);
      }
      ash::window_util::PinWindow(target_window, true);
    }
    return RespondNow(OneArgument(CreateWindowValueFromNativeWindow(target_window)));
  }

  ExtensionFunction::ResponseAction NativeWindowsRemoveFunction::Run() {
    std::unique_ptr<windows::Remove::Params> params(
      windows::Remove::Params::Create(args()));
    EXTENSION_FUNCTION_VALIDATE(params);
    Window* target_window = NULL;
    if (!GetWindowFromWindowID(params->window_id, &target_window))
      return RespondNow(Error(keys::kNoCurrentWindowError));

    if (ash::IsArcWindow(target_window)){
      auto task_id = arc::GetWindowTaskId(target_window);
      if (!task_id.has_value())
        return RespondNow(Error(keys::kNoCurrentWindowError));
      arc::CloseTask(*task_id);
    }else{
      ash::window_util::CloseWidgetForWindow(target_window);
    }
    return RespondNow(NoArguments());
  }

  ExtensionFunction::ResponseAction NativeWindowsCreateFunction::Run() {
    std::unique_ptr<windows::Create::Params> params(
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
        proxy->Launch(params->app_id, event_flags, apps::mojom::LaunchSource::kFromAppListGrid,
                      apps::MakeWindowInfo(client->GetAppListDisplayId()));
      }
    }
    return RespondNow(NoArguments());
  }

} // exit namesapce extensions
