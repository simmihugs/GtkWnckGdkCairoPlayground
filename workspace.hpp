#pragma once

#include <vector>
#include "AppButton.hpp"

class WorkspaceButtons {
public:
  std::vector<AppButton> appbuttons;

  std::vector<AppButton> &get_app_buttons(WnckWorkspace);
  
  WorkspaceButtons(WnckWorkspace wnckWorkspace)
  {
    this->appbuttons = get_app_buttons(wnckWorkspace);
  }
  
};
