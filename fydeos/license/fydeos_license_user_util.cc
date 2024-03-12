// Copyright 2020 The FydeOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#include "fydeos/license/fydeos_license_user_util.h"

#include "url/gurl.h"
#include "components/user_manager/user_manager.h"
#include "net/base/url_util.h"

namespace fydeos {
namespace license {

GURL AppendAccountIdQueryParameter(GURL& url) {
  user_manager::UserManager* user_manager = user_manager::UserManager::Get();
  if (!user_manager) return url;
  const user_manager::User* user = user_manager->GetPrimaryUser();
  if (user) {
    AccountId account_id = user->GetAccountId();
    const std::string email = account_id.GetUserEmail();
    url = net::AppendQueryParameter(url, "email", email);
    AccountType account_type = account_id.GetAccountType();
    std::string id;
    if (account_type == AccountType::GOOGLE) {
      id = account_id.GetGaiaId();
    } else if (account_type == AccountType::FYDE_ACCOUNT) {
      id = account_id.GetFydeId();
    }
    std::string account_type_str = AccountId::AccountTypeToString(account_type);
    url = net::AppendQueryParameter(url, "account_type", account_type_str);
    url = net::AppendQueryParameter(url, "account_id", id);
  }
  return url;
}

} // license
} // fydeos
