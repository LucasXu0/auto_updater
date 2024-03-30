#include "include/auto_updater/auto_updater_plugin.h"

// This must be included before many other Windows headers.
#include <windows.h>

// For getPlatformVersion; remove unless needed for your plugin implementation.
#include <VersionHelpers.h>

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <flutter/standard_method_codec.h>

#include <map>
#include <memory>
#include <sstream>

#include "auto_updater.cpp"

namespace {

class AutoUpdaterPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrarWindows* registrar);
  void SetChannel(std::unique_ptr<flutter::MethodChannel<flutter::EncodableValue>> ptr) {
     _channel = std::move(ptr);
  }

  AutoUpdaterPlugin();

  virtual ~AutoUpdaterPlugin();

 private:
  AutoUpdater auto_updater = AutoUpdater();
  std::unique_ptr<flutter::MethodChannel<flutter::EncodableValue>> _channel;

  // Called when a method is called on this plugin's channel from Dart.
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue>& method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
};

// static
void AutoUpdaterPlugin::RegisterWithRegistrar(
    flutter::PluginRegistrarWindows* registrar) {
  auto channel =
      std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
          registrar->messenger(), "auto_updater",
          &flutter::StandardMethodCodec::GetInstance());

  auto plugin = std::make_unique<AutoUpdaterPlugin>();

  channel->SetMethodCallHandler(
      [plugin_pointer = plugin.get()](const auto& call, auto result) {
        plugin_pointer->HandleMethodCall(call, std::move(result));
      });

  plugin->SetChannel(std::move(channel));

  registrar->AddPlugin(std::move(plugin));
}

AutoUpdaterPlugin::AutoUpdaterPlugin() {}

AutoUpdaterPlugin::~AutoUpdaterPlugin() {}

void AutoUpdaterPlugin::HandleMethodCall(
    const flutter::MethodCall<flutter::EncodableValue>& method_call,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  std::string method_name = method_call.method_name();

  if (method_name.compare("setFeedURL") == 0) {
    const flutter::EncodableMap& args =
        std::get<flutter::EncodableMap>(*method_call.arguments());
    std::string feedURL =
        std::get<std::string>(args.at(flutter::EncodableValue("feedURL")));
    auto_updater.SetFeedURL(feedURL);
    auto_updater.RegisterCallbacks(std::move(_channel));
    result->Success(flutter::EncodableValue(true));

  } else if (method_name.compare("checkForUpdates") == 0) {
    const flutter::EncodableMap& args =
            std::get<flutter::EncodableMap>(*method_call.arguments());
    bool inBackground =
            std::get<bool>(args.at(flutter::EncodableValue("inBackground")));
    if (inBackground) {
      auto_updater.CheckForUpdatesWithoutUI();
    } else {
      auto_updater.CheckForUpdates();
    }
    result->Success(flutter::EncodableValue(true));

  } else if (method_name.compare("setScheduledCheckInterval") == 0) {
    const flutter::EncodableMap& args =
        std::get<flutter::EncodableMap>(*method_call.arguments());
    int interval =
        std::get<int>(args.at(flutter::EncodableValue("interval")));
    auto_updater.SetScheduledCheckInterval(interval);
    result->Success(flutter::EncodableValue(true));

  } else {
    result->NotImplemented();
  }
}

}  // namespace

void AutoUpdaterPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  AutoUpdaterPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrarWindows>(registrar));
}
