// Copyright (c) 2018 The FydeOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// Author: Simon Tsao(yang@fydeos.io)

#include "fydeos/extensions/browser/api/shell_client/shell_client_api.h"

#include <stddef.h>
#include "base/functional/bind.h"
#include "base/logging.h"
#include "base/lazy_instance.h"
#include "fydeos/extensions/common/api/shell_client.h"
#include "chromeos/ash/components/dbus/dbus_thread_manager.h"
#include "chromeos/dbus/common/dbus_callback.h"
#include "fydeos/chromeos/ash/components/dbus/fydeos_shell_client/shell_state.h"
#include "fydeos/chromeos/ash/components/dbus/fydeos_shell_client/fydeos_shell_client.h"

using DBusThreadManager = ash::DBusThreadManager;
using FydeOSShellClient = fydeos::ash::FydeOSShellClient;
using ShellState = fydeos::ash::ShellState;
namespace shell = extensions::api::shell_client;

namespace extensions {

namespace {
  const char kShellCode[] = "code";
  const char kShellResult[] = "result";

  FydeOSShellClient* GetShellClient() {
    return FydeOSShellClient::Get();
  }

  base::Value CreateStateValueFromState(std::optional<ShellState> state) {
    base::Value::Dict value;
    VLOG(1) << "get state and create value";
    if (!state) {
      value.Set(kShellCode, -1);
      value.Set(kShellResult, "no state return");
    } else {
      value.Set(kShellCode, state->code);
      value.Set(kShellResult, state->result);
    }
    return base::Value(std::move(value));
  }


}  // namespace

static base::LazyInstance<BrowserContextKeyedAPIFactory<ShellClientAPI>>::
    DestructorAtExit g_shell_client_api_factory =
        LAZY_INSTANCE_INITIALIZER;

// static
BrowserContextKeyedAPIFactory<ShellClientAPI>*
ShellClientAPI::GetFactoryInstance() {
  return g_shell_client_api_factory.Pointer();
}

ShellClientAPI* ShellClientAPI::Get(
    content::BrowserContext* context) {
  return GetFactoryInstance()->Get(context);
}

ShellClientAPI::ShellClientAPI(content::BrowserContext* context)
  : profile_(Profile::FromBrowserContext(context)), weak_factory_(this) {
  RegisterNotifications();
}

ShellClientEventRouter::ShellClientEventRouter(Profile* profile)
  : fydeos_client_observer_(this),
    profile_(profile),
    event_router_(EventRouter::Get(profile_)),
    weak_factory_(this) {
  fydeos_client_observer_.Observe(GetShellClient());
}

ShellClientEventRouter::~ShellClientEventRouter() {}

void ShellClientEventRouter::AddExtensionId(const std::string& extension_id,
    const std::string& event_name) {
  if (event_name == shell::OnSystemNotifying::kEventName)
    system_extension_ids_.insert(extension_id);
  else if (event_name == shell::OnShellCommandNotifying::kEventName)
    command_extension_ids_.insert(extension_id);
  else
    custom_extension_ids_.insert(extension_id);
}

void ShellClientEventRouter::RemoveExtensionId(const std::string& extension_id,
    const std::string& event_name) {
  if (event_name == shell::OnSystemNotifying::kEventName)
    system_extension_ids_.erase(extension_id);
  else if (event_name == shell::OnShellCommandNotifying::kEventName)
    command_extension_ids_.erase(extension_id);
  else
    custom_extension_ids_.erase(extension_id);
}

void ShellClientEventRouter::OnSystemNotificationReceived(int32_t level,
    const std::string& msg) {
  VLOG(1) << "Received system notification level:" << level;
  if (!system_extension_ids_.size())
    return;
  base::Value::List args;
  args.Append(level);
  args.Append(msg);
  auto event = std::make_unique<Event>(
      events::FYDEOS_SHELL_CLIENT_SYSTEM_NOTIFICATION,
      shell::OnSystemNotifying::kEventName, std::move(args));
  event_router_->BroadcastEvent(std::move(event));
}

void ShellClientEventRouter::OnCommandNotificationReceived(int32_t handler,
    int32_t state, const std::string& msg) {
  VLOG(1) << "Received command notification handler:" << handler;
  if (!command_extension_ids_.size())
    return;
  base::Value::List args;
  args.Append(handler);
  args.Append(state);
  args.Append(msg);
  auto event = std::make_unique<Event>(
      events::FYDEOS_SHELL_CLIENT_COMMAND_NOTIFICATION,
      shell::OnShellCommandNotifying::kEventName, std::move(args));
  event_router_->BroadcastEvent(std::move(event));
}

void ShellClientEventRouter::OnCustomNotificationReceived(int32_t data,
    int32_t exdata, const std::string& extra) {
  VLOG(1) << "Received custom notification data:" << data;
  if (!custom_extension_ids_.size())
    return;
  base::Value::List args;
  args.Append(data);
  args.Append(exdata);
  args.Append(extra);
  auto event = std::make_unique<Event>(
      events::FYDEOS_SHELL_CLIENT_CUSTOM_NOTIFICATION,
      shell::OnShellCustomNotifying::kEventName, std::move(args));
  event_router_->BroadcastEvent(std::move(event));
}

void ShellClientAPI::RegisterNotifications() {
  VLOG(1) << "Init shell client API: register notifications";
  EventRouter* router = EventRouter::Get(profile_);
  router->RegisterObserver(
      this, shell::OnSystemNotifying::kEventName);
  router->RegisterObserver(
      this, shell::OnShellCommandNotifying::kEventName);
  router->RegisterObserver(
      this, shell::OnShellCustomNotifying::kEventName);
}

ShellClientAPI::~ShellClientAPI() {}

void ShellClientAPI::Shutdown() {
  EventRouter::Get(profile_)->UnregisterObserver(this);
  shell_client_event_router_.reset(nullptr);
}

void ShellClientAPI::OnListenerAdded(const EventListenerInfo& details) {
  VLOG(1) << "Add extension:" << details.extension_id
    << " on listener:" << details.event_name;
  if (!shell_client_event_router_) {
    shell_client_event_router_ =
      std::make_unique<ShellClientEventRouter>(profile_);
  }
  shell_client_event_router_->AddExtensionId(details.extension_id,
    details.event_name);
}

void ShellClientAPI::OnListenerRemoved(const EventListenerInfo& details) {
  VLOG(1) << "Remove extension:" << details.extension_id
    << " on listener:" << details.event_name;
  if (shell_client_event_router_)
    shell_client_event_router_->RemoveExtensionId(details.extension_id,
      details.event_name);
}

ExtensionFunction::ResponseAction ShellClientExecSyncFunction::Run() {
  std::optional<shell::ExecSync::Params> params(
      shell::ExecSync::Params::Create(args()));
  EXTENSION_FUNCTION_VALIDATE(params);
  VLOG(1) << "shell extension get command:" << params->command;
  GetShellClient()->SyncExec(
      params->command,
      base::BindOnce(&ShellClientExecSyncFunction::StateCallback, this));
  return RespondLater();
}

void ShellClientExecSyncFunction::StateCallback(
    std::optional<ShellState> state) {
  Respond(WithArguments(CreateStateValueFromState(state)));
}

ExtensionFunction::ResponseAction ShellClientExecAsyncFunction::Run() {
  std::optional<shell::ExecAsync::Params> params(
      shell::ExecAsync::Params::Create(args()));
  EXTENSION_FUNCTION_VALIDATE(params);
  GetShellClient()->AsyncExec(
      params->command,
      base::BindOnce(&ShellClientExecAsyncFunction::StateCallback, this));
  return RespondLater();
}

void ShellClientExecAsyncFunction::StateCallback(
    std::optional<ShellState> state) {
  Respond(WithArguments(CreateStateValueFromState(state)));
}

ExtensionFunction::ResponseAction ShellClientGetTaskStateFunction::Run() {
  std::optional<shell::GetTaskState::Params> params(
      shell::GetTaskState::Params::Create(args()));
  EXTENSION_FUNCTION_VALIDATE(params);
  GetShellClient()->GetTaskState(
      params->key,
      base::BindOnce(&ShellClientGetTaskStateFunction::StateCallback, this));
  return RespondLater();
}

void ShellClientGetTaskStateFunction::StateCallback(
    std::optional<ShellState> state) {
  Respond(WithArguments(CreateStateValueFromState(state)));
}

ExtensionFunction::ResponseAction ShellClientForceCloseTaskFunction::Run() {
  std::optional<shell::ForceCloseTask::Params> params(
      shell::ForceCloseTask::Params::Create(args()));
  EXTENSION_FUNCTION_VALIDATE(params);
  GetShellClient()->ForceCloseTask(
      params->key,
      base::BindOnce(&ShellClientForceCloseTaskFunction::StateCallback, this));
  return RespondLater();
}

void ShellClientForceCloseTaskFunction::StateCallback(
    std::optional<ShellState> state) {
  Respond(WithArguments(CreateStateValueFromState(state)));
}

ExtensionFunction::ResponseAction ShellClientGetTaskOutputFunction::Run() {
  std::optional<shell::GetTaskOutput::Params> params(
      shell::GetTaskOutput::Params::Create(args()));
  EXTENSION_FUNCTION_VALIDATE(params);
  GetShellClient()->GetTaskOutput(
      params->key,
      params->lines,
      base::BindOnce(&ShellClientGetTaskOutputFunction::StateCallback, this));
  return RespondLater();
}

void ShellClientGetTaskOutputFunction::StateCallback(
    std::optional<ShellState> state) {
  Respond(WithArguments(CreateStateValueFromState(state)));
}

ExtensionFunction::ResponseAction ShellClientGetDaemonStateFunction::Run() {
  GetShellClient()->GetDaemonState(
      base::BindOnce(&ShellClientGetDaemonStateFunction::StateCallback, this));
  return RespondLater();
}

void ShellClientGetDaemonStateFunction::StateCallback(
    std::optional<ShellState> state) {
  Respond(WithArguments(CreateStateValueFromState(state)));
}
} //exit extensions
