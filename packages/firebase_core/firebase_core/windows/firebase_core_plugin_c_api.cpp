// Copyright 2023, the Chromium project authors.  Please see the AUTHORS file
// for details. All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.

#include "include/firebase_core/firebase_core_plugin_c_api.h"

#include <flutter/plugin_registrar_windows.h>

#include <string>
#include <vector>

#include "firebase_core_plugin.h"

void FirebaseCorePluginCApiRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  firebase_core_windows::FirebaseCorePlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrarWindows>(registrar));
}

std::vector<std::string> GetFirebaseApp(std::string appName) {
  return firebase_core_windows::FirebaseCorePlugin::GetFirebaseApp(appName);
}

void* GetFirebaseAuth(std::string appName) {
  return firebase_core_windows::FirebaseCorePlugin::GetFirebaseAuth(appName);
}

void* GetFirebaseRemoteConfig(std::string appName) {
  return firebase_core_windows::FirebaseCorePlugin::GetFirebaseRemoteConfig(
      appName);
}
