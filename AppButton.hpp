#pragma once

#include <libwnck/libwnck.h>
#include <string>

class AppButton {
public:
  WnckWindow *wnckWindow;

  AppButton(WnckWindow *wnckWindow)
  {
    this->wnckWindow = wnckWindow;
  }

  GtkWidget *get_icon();
  GtkWidget *get_miniview();
  std::string *get_name();
  WnckWorkspace *get_workspace();
  
};
