// Copyright (c) 2018 The FlintOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// Author: Simon Tsao(yang@flintos.io)

#include "fydeos/extensions/browser/api/command_line/command_line_api.h"
#include "fydeos/extensions/common/api/command_line.h"

#include <memory>
#include <string>

#include "base/command_line.h"
#include "base/values.h"

namespace extensions {
  namespace command_line = api::command_line;
  const char kEmptySwitchName[] = "Switch name is empty.";

  namespace {
    bool HasSwitch(const std::string& name){
      return base::CommandLine::ForCurrentProcess()->HasSwitch(name);
    }
    std::string GetSwitchValue(const std::string& name){
      return base::CommandLine::ForCurrentProcess()->GetSwitchValueASCII(name);
    }

  } //internal namespace

  ExtensionFunction::ResponseAction CommandLineHasSwitchFunction::Run() {
    std::unique_ptr<command_line::HasSwitch::Params> params(
      command_line::HasSwitch::Params::Create(args()));
    EXTENSION_FUNCTION_VALIDATE(params);
    if (params->name.empty())
      return RespondNow(Error(kEmptySwitchName));
    if (!HasSwitch(params->name))
      return RespondNow(ArgumentList(command_line::HasSwitch::Results::Create(false, std::string())));
    return RespondNow(ArgumentList(command_line::HasSwitch::Results::Create(true, GetSwitchValue(params->name))));
  }

  ExtensionFunction::ResponseAction CommandLineAddSwitchFunction::Run() {
    std::unique_ptr<command_line::AddSwitch::Params> params(
      command_line::AddSwitch::Params::Create(args()));
    EXTENSION_FUNCTION_VALIDATE(params);
    if (params->switch_info.name.empty())
      return RespondNow(Error(kEmptySwitchName));
    if (params->switch_info.value)
      base::CommandLine::ForCurrentProcess()->AppendSwitchASCII(params->switch_info.name, *params->switch_info.value);
    else
      base::CommandLine::ForCurrentProcess()->AppendSwitch(params->switch_info.name);
    return RespondNow(NoArguments());
  }

  ExtensionFunction::ResponseAction CommandLineRemoveSwitchFunction::Run() {
    std::unique_ptr<command_line::RemoveSwitch::Params> params(
      command_line::RemoveSwitch::Params::Create(args()));
    EXTENSION_FUNCTION_VALIDATE(params);
    if (params->name.empty())
      return RespondNow(Error(kEmptySwitchName));
    base::CommandLine::ForCurrentProcess()->RemoveSwitch(params->name);
    return RespondNow(NoArguments());
  }

} //extensions
