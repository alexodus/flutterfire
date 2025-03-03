// Copyright 2023, the Chromium project authors.  Please see the AUTHORS file
// for details. All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.

#include "firebase_core_plugin.h"

// This must be included before many other Windows headers.
#include <windows.h>

#include "firebase/app.h"
#include "firebase/auth.h"
#include "firebase/remote_config.h"
#include "messages.g.h"

// For getPlatformVersion; remove unless needed for your plugin implementation.
#include <VersionHelpers.h>
#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <flutter/standard_method_codec.h>

#include <future>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using ::firebase::App;
using ::firebase::auth::Auth;
using ::firebase::remote_config::RemoteConfig;

namespace firebase_core_windows {

// static
void FirebaseCorePlugin::RegisterWithRegistrar(
    flutter::PluginRegistrarWindows *registrar) {
  auto plugin = std::make_unique<FirebaseCorePlugin>();

  FirebaseCoreHostApi::SetUp(registrar->messenger(), plugin.get());
  FirebaseAppHostApi::SetUp(registrar->messenger(), plugin.get());

  registrar->AddPlugin(std::move(plugin));
}

std::map<std::string, std::vector<std::string>> apps;

std::vector<std::string> FirebaseCorePlugin::GetFirebaseApp(
    std::string appName) {
  auto app_it = apps.find(appName);

  // If the app is already in the map, return the stored shared_ptr
  if (app_it != apps.end()) {
    return app_it->second;
  }

  std::vector<std::string> app_vector;
  return app_vector;
}

void *FirebaseCorePlugin::GetFirebaseAuth(std::string appName) {
  App *app = App::GetInstance(appName.c_str());
  if (app == nullptr) {
    return nullptr;
  }
  return Auth::GetAuth(app);
}

void *FirebaseCorePlugin::GetFirebaseRemoteConfig(std::string appName) {
  App *app = App::GetInstance(appName.c_str());
  if (app == nullptr) {
    return nullptr;
  }
  return RemoteConfig::GetInstance(app);
}

FirebaseCorePlugin::FirebaseCorePlugin() {}

FirebaseCorePlugin::~FirebaseCorePlugin() = default;

// Convert a Pigeon FirebaseOptions to a Firebase Options.
firebase::AppOptions PigeonFirebaseOptionsToAppOptions(
    const PigeonFirebaseOptions &pigeon_options) {
  firebase::AppOptions options;
  options.set_api_key(pigeon_options.api_key().c_str());
  options.set_app_id(pigeon_options.app_id().c_str());
  if (pigeon_options.database_u_r_l() != nullptr) {
    options.set_database_url(pigeon_options.database_u_r_l()->c_str());
  }
  if (pigeon_options.tracking_id() != nullptr) {
    options.set_ga_tracking_id(pigeon_options.tracking_id()->c_str());
  }
  options.set_messaging_sender_id(pigeon_options.messaging_sender_id().c_str());

  options.set_project_id(pigeon_options.project_id().c_str());

  if (pigeon_options.storage_bucket() != nullptr) {
    options.set_storage_bucket(pigeon_options.storage_bucket()->c_str());
  }
  return options;
}

// Convert a AppOptions to PigeonInitializeOption
PigeonFirebaseOptions optionsFromFIROptions(
    const firebase::AppOptions &options) {
  PigeonFirebaseOptions pigeon_options = PigeonFirebaseOptions();
  pigeon_options.set_api_key(options.api_key());
  pigeon_options.set_app_id(options.app_id());
  if (options.database_url() != nullptr) {
    pigeon_options.set_database_u_r_l(options.database_url());
  }
  pigeon_options.set_tracking_id(nullptr);
  pigeon_options.set_messaging_sender_id(options.messaging_sender_id());
  pigeon_options.set_project_id(options.project_id());
  if (options.storage_bucket() != nullptr) {
    pigeon_options.set_storage_bucket(options.storage_bucket());
  }
  return pigeon_options;
}

// Convert a firebase::App to PigeonInitializeResponse
PigeonInitializeResponse AppToPigeonInitializeResponse(const App &app) {
  PigeonInitializeResponse response = PigeonInitializeResponse();
  response.set_name(app.name());
  response.set_options(optionsFromFIROptions(app.options()));
  return response;
}

void FirebaseCorePlugin::InitializeApp(
    const std::string &app_name,
    const PigeonFirebaseOptions &initialize_app_request,
    std::function<void(ErrorOr<PigeonInitializeResponse> reply)> result) {
  // Create an app
  App *app =
      App::Create(PigeonFirebaseOptionsToAppOptions(initialize_app_request),
                  app_name.c_str());

  auto app_it = apps.find(app_name);

  // If the app is already in the map, return the stored shared_ptr
  if (app_it == apps.end()) {
    std::vector<std::string> app_vector;
    app_vector.push_back(app_name);
    app_vector.push_back(initialize_app_request.api_key());
    app_vector.push_back(initialize_app_request.app_id());
    app_vector.push_back(*initialize_app_request.database_u_r_l());
    app_vector.push_back(initialize_app_request.project_id());

    apps[app_name] = app_vector;
  }

  // Send back the result to Flutter
  result(AppToPigeonInitializeResponse(*app));
}

void FirebaseCorePlugin::InitializeCore(
    std::function<void(ErrorOr<flutter::EncodableList> reply)> result) {
  // TODO: Missing function to get the list of currently initialized apps
  std::vector<PigeonInitializeResponse> initializedApps;
  std::vector<App *> all_apps = App::GetApps();
  for (const App *app : all_apps) {
    initializedApps.push_back(AppToPigeonInitializeResponse(*app));
  }

  flutter::EncodableList encodableList;

  for (const auto &item : initializedApps) {
    encodableList.push_back(flutter::CustomEncodableValue(item));
  }
  result(encodableList);
}

void FirebaseCorePlugin::OptionsFromResource(
    std::function<void(ErrorOr<PigeonFirebaseOptions> reply)> result) {}

void FirebaseCorePlugin::SetAutomaticDataCollectionEnabled(
    const std::string &app_name, bool enabled,
    std::function<void(std::optional<FlutterError> reply)> result) {
  App *firebaseApp = App::GetInstance(app_name.c_str());
  if (firebaseApp != nullptr) {
    // TODO: Missing method
  }
  result(std::nullopt);
}

void FirebaseCorePlugin::SetAutomaticResourceManagementEnabled(
    const std::string &app_name, bool enabled,
    std::function<void(std::optional<FlutterError> reply)> result) {
  App *firebaseApp = App::GetInstance(app_name.c_str());
  if (firebaseApp != nullptr) {
    // TODO: Missing method
  }

  result(std::nullopt);
}

void FirebaseCorePlugin::Delete(
    const std::string &app_name,
    std::function<void(std::optional<FlutterError> reply)> result) {
  App *firebaseApp = App::GetInstance(app_name.c_str());
  if (firebaseApp != nullptr) {
    // TODO: Missing method
  }

  result(std::nullopt);
}

}  // namespace firebase_core_windows
