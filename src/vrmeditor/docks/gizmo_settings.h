#pragma once

class GizmoSettings
{
  struct GizmoSettingsImpl* m_impl;
public:
  GizmoSettings();
  ~GizmoSettings();
  void ShowGui();
};
