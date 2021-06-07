// Copyright (c) 2018 The FydeOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fydeos/dbus/fydeos_shell_client.h"

#include <stdint.h>

#include <utility>

#include "base/bind.h"
#include "base/logging.h"
#include "base/format_macros.h"
#include "base/strings/stringprintf.h"
#include "dbus/bus.h"
#include "dbus/message.h"
#include "dbus/object_path.h"
#include "dbus/object_proxy.h"

namespace fydeos {
namespace {
  const char kFydeOSShellServiceName[] = "io.fydeos.ShellDaemon";
  const char kFydeOSShellServicePath[] = "/io/fydeos/ShellDaemon";
  const char kFydeOSShellServiceInterface[] = "io.fydeos.ShellInterface";
  const char kShellSyncExec[] = "SyncExec";
  const char kShellAsyncExec[] = "AsyncExec";
  const char kShellGetTaskOutput[] = "GetAsyncTaskOutput";
  const char kShellGetTaskState[] = "GetTaskState";
  const char kShellForceCloseTask[] = "ForceCloseTask";
  const char kShellGetDaemonState[] = "GetDaemonState";
  const char kShellSystemNotify[] = "ShellNotifying";
  enum NotificationType { SYSTEM, COMMAND };
}// end namespace

class FydeOSShellClientImpl : public FydeOSShellClient {
  public:
    FydeOSShellClientImpl() : shell_proxy_(NULL), weak_ptr_factory_(this) {}
    ~FydeOSShellClientImpl() override = default;

    void SyncExec(const std::string& cmd, chromeos::DBusMethodCallback<ShellState> callback) override {
      VLOG(1) << "SyncExec received command:" << cmd;
      dbus::MethodCall method_call(kFydeOSShellServiceInterface, kShellSyncExec);
      dbus::MessageWriter writer(&method_call);
      writer.AppendString(cmd);
      shell_proxy_->CallMethod(&method_call,
          dbus::ObjectProxy::TIMEOUT_USE_DEFAULT,
          base::BindOnce(&FydeOSShellClientImpl::OnShellDaemonReturn,
            weak_ptr_factory_.GetWeakPtr(), std::move(callback)));
    }

    void AsyncExec(const std::string& cmd, chromeos::DBusMethodCallback<ShellState> callback) override {
      VLOG(1) << "AsyncExec reveived command:" << cmd;
      dbus::MethodCall method_call(kFydeOSShellServiceInterface, kShellAsyncExec);
      dbus::MessageWriter writer(&method_call);
      writer.AppendString(cmd);
      shell_proxy_->CallMethod(&method_call,
                      dbus::ObjectProxy::TIMEOUT_USE_DEFAULT,
                      base::BindOnce(&FydeOSShellClientImpl::OnShellDaemonReturn,
                              weak_ptr_factory_.GetWeakPtr(), std::move(callback)));
    }

    void GetTaskOutput(int32_t key, int32_t lines, chromeos::DBusMethodCallback<ShellState> callback) override {
      dbus::MethodCall method_call(kFydeOSShellServiceInterface, kShellGetTaskOutput);
      dbus::MessageWriter writer(&method_call);
      writer.AppendInt32(key);
      writer.AppendInt32(lines);
      shell_proxy_->CallMethod(&method_call,
                      dbus::ObjectProxy::TIMEOUT_USE_DEFAULT,
                      base::BindOnce(&FydeOSShellClientImpl::OnShellDaemonReturn,
                              weak_ptr_factory_.GetWeakPtr(), std::move(callback)));
    }

    void GetTaskState(int32_t key, chromeos::DBusMethodCallback<ShellState> callback) override {
      dbus::MethodCall method_call(kFydeOSShellServiceInterface, kShellGetTaskState);
      dbus::MessageWriter writer(&method_call);
      writer.AppendInt32(key);
      shell_proxy_->CallMethod(&method_call,
                      dbus::ObjectProxy::TIMEOUT_USE_DEFAULT,
                      base::BindOnce(&FydeOSShellClientImpl::OnShellDaemonReturn,
                              weak_ptr_factory_.GetWeakPtr(), std::move(callback)));
    }

    void ForceCloseTask(int32_t key, chromeos::DBusMethodCallback<ShellState> callback) override {
      dbus::MethodCall method_call(kFydeOSShellServiceInterface, kShellForceCloseTask);
      dbus::MessageWriter writer(&method_call);
      writer.AppendInt32(key);
      shell_proxy_->CallMethod(&method_call,
                      dbus::ObjectProxy::TIMEOUT_USE_DEFAULT,
                      base::BindOnce(&FydeOSShellClientImpl::OnShellDaemonReturn,
                              weak_ptr_factory_.GetWeakPtr(), std::move(callback)));
    }

    void GetDaemonState(chromeos::DBusMethodCallback<ShellState> callback) override {
      dbus::MethodCall method_call(kFydeOSShellServiceInterface, kShellGetDaemonState);
      shell_proxy_->CallMethod(&method_call,
                      dbus::ObjectProxy::TIMEOUT_USE_DEFAULT,
                      base::BindOnce(&FydeOSShellClientImpl::OnShellDaemonReturn,
                              weak_ptr_factory_.GetWeakPtr(), std::move(callback)));
    }

    void AddObserver(Observer* observer) override {
      DCHECK(observer);  // http://crbug.com/119976
      observers_.AddObserver(observer);
    }

    void RemoveObserver(Observer* observer) override {
      observers_.RemoveObserver(observer);
    }

    bool HasObserver(const Observer* observer) const override {
      return observers_.HasObserver(observer);
    }

  protected:
    void Init(dbus::Bus* bus) override {
      shell_proxy_ = bus->GetObjectProxy(kFydeOSShellServiceName,
          dbus::ObjectPath(kFydeOSShellServicePath));
      shell_proxy_->ConnectToSignal(
        kFydeOSShellServiceInterface, kShellSystemNotify,
        base::BindRepeating(&FydeOSShellClientImpl::OnShellSignalReceived,
          weak_ptr_factory_.GetWeakPtr()),
        base::BindOnce(&FydeOSShellClientImpl::SignalConnected,
          weak_ptr_factory_.GetWeakPtr())
      );
      VLOG(1) << "Init shell_proxy";
    }

  private:
    void SignalConnected(const std::string& interface_name,
                       const std::string& signal_name,
                       bool success) {
      if (!success)
        LOG(ERROR) << "Failed to connect to signal " <<  signal_name << ".";
      else
        VLOG(1) << "Success to connnect to signal " << signal_name;
    }

    void OnShellSignalReceived(dbus::Signal* signal) {
      int32_t type, handler, state;
      std::string msg;
      dbus::MessageReader reader(signal);
      if (!reader.PopInt32(&type) ||
          !reader.PopInt32(&handler) ||
          !reader.PopInt32(&state) ||
          !reader.PopString(&msg)) {
        LOG(ERROR) << "Error reading signal: " << signal->ToString();
        return;
      }
      VLOG(1) << "Get notify type:" << type << " handler:" << handler
        << " state:" << state << " msg:" << msg;
      switch (type) {
        case NotificationType::SYSTEM:
          for (auto& observer : observers_)
            observer.OnSystemNotificationReceived(state, std::move(msg));
          break;
        case NotificationType::COMMAND:
          for (auto& observer : observers_)
            observer.OnCommandNotificationReceived(handler, state, std::move(msg));
          break;
        default:
          for (auto& observer : observers_)
            observer.OnCustomNotificationReceived(handler, state, std::move(msg));
          break;
      }
    }

    void OnShellDaemonReturn(chromeos::DBusMethodCallback<ShellState> callback,
        dbus::Response* response) {
      if (!response) {
        LOG(ERROR) << "Error calling FydeOS shell daemon";
        std::move(callback).Run(absl::nullopt);
        return;
      }
      ShellState shell_state;
      dbus::MessageReader reader(response);
      dbus::MessageReader sub_reader(nullptr);
      if (!reader.PopStruct(&sub_reader) ||!sub_reader.PopInt32(&shell_state.code) ||
          !sub_reader.PopString(&shell_state.result)) {
        LOG(ERROR) << "Error reading response from FydeOS shell daemon: "
                 << response->ToString();
        std::move(callback).Run(absl::nullopt);
        return;
      }
      VLOG(1) << "Return code:" << shell_state.code;
      std::move(callback).Run(std::move(shell_state));
    }

    dbus::ObjectProxy* shell_proxy_ = nullptr;
    base::ObserverList<Observer>::Unchecked observers_;
    base::WeakPtrFactory<FydeOSShellClientImpl> weak_ptr_factory_;
};// end FydeOSShellClientImpl

FydeOSShellClient::FydeOSShellClient() = default;
FydeOSShellClient::~FydeOSShellClient() = default;

FydeOSShellClient* FydeOSShellClient::Create() {
  return new FydeOSShellClientImpl();
}

} //end namespace fydeos
