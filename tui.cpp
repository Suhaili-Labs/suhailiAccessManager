#include <iostream>
#include <fstream>
#include <algorithm>
#include <filesystem>
#include <functional>
#include <system_error>
#include <sys/ioctl.h>
#include <unistd.h>
#include "json.hpp"
#include "accessman.h"

#include <ftxui/component/component.hpp>  // for Button, Renderer
#include <ftxui/component/component_options.hpp>  // for Catch
#include <ftxui/screen/screen.hpp>  // for Screen
#include <ftxui/dom/elements.hpp>  // for text, vbox
#include <ftxui/component/screen_interactive.hpp>  // for ScreenInteractive

using std::cout;
using nlohmann::json;
using std::string;
using std::endl;
using std::ifstream;
using std::ofstream;
using std::vector;
using namespace ftxui;

namespace {

struct TerminalSize {
  int width;
  int height;
};

TerminalSize getTerminalSize() {
  winsize ws{};
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) != 0) {
    return {0, 0};
  }
  return {static_cast<int>(ws.ws_col), static_cast<int>(ws.ws_row)};
}

}  // namespace

int main() {

  const int minTerminalWidth = 80;
  const int minTerminalHeight = 32;

  json ndiConfig = json::object();
  
// Make Function????
  const std::filesystem::path ndiDir = std::filesystem::path(getHomeDir()) / ".ndi";
  
  std::error_code fsError;
  const bool ndiDirExists = std::filesystem::exists(ndiDir, fsError);
  if (fsError) {
    std::cerr << "Could not check NDI config directory: " << ndiDir << " (" << fsError.message() << ")" << endl;
    return 1;
  }

  if (!ndiDirExists) {
    std::filesystem::create_directories(ndiDir, fsError);
    if (fsError) {
      std::cerr << "Could not create NDI config directory: " << ndiDir << " (" << fsError.message() << ")" << endl;
      return 1;
    }
    cout << ".ndi directory does not exist. Creating." << endl;
  }
// Make Function????
  const std::filesystem::path configPath = ndiDir / "ndi-config.v1.json";
  const std::filesystem::path backupConfigPath = configPath.string() + ".bak";

  cout << "NDI Config Dir: " << configPath << endl;
  
// Need to add a function to generate missing settings if they do not exist
// in the ndi-config, or program will crash if they are not present in the config.

  ifstream inputFile(configPath);
  if (inputFile.is_open()) {
    try {
      inputFile >> ndiConfig;
    } catch (const json::exception& e) {
      std::cerr << "Invalid NDI Config JSON at " << configPath << ": " << e.what() << endl;
      cout << "Using default config values for this run." << endl;
      ndiConfig = json::object();
    }
    inputFile.close();
  } else {
    cout << "NDI config file does not exist yet. A new ndi-config.v1.json will be created." << endl;
  }

// Generate anny "missing" config items so they can be accessed without errors
  try {
    generateMissingConfig(ndiConfig);
  } catch (const json::exception& e) {
    std::cerr << "Could not normalize NDI config, resetting to defaults: " << e.what() << endl;
    ndiConfig = json::object();
    generateMissingConfig(ndiConfig);
  }

// TUI BELOW

  string titleL1 = "█▀▀ █ █ █ █ ▄▀█ █ █   █";
  string titleL2 = "▄▄█ █▄█ █▀█ █▀█ █ █▄▄ █";
  string titleL3 = "";
  string titleL4 = "▄▀█ █▀▀ █▀▀ █▀▀ █▀▀ █▀▀   █▀▄▀█ ▄▀█ █▄ █ ▄▀█ █▀▀ █▀▀ █▀█";
  string titleL5 = "█▀█ █▄▄ █▄▄ ██▄ ▄▄█ ▄▄█   █ ▀ █ █▀█ █ ▀█ █▀█ █▄█ ██▄ █▀▄";




  auto screen = ScreenInteractive::Fullscreen();
  enum class ExitAction {
    Discard,
    Save,
  };

  ExitAction exitAction = ExitAction::Discard;
  bool showDiscardConfirm = false;
  string restoreStatusMessage = "Not loaded";
  bool restoreStatusIsError = false;
  std::function<bool()> hasUnsavedChanges = [] {
    return false;
  };
  auto closeScreen = screen.ExitLoopClosure();
  auto saveAndExitButton = Button("Save & Exit", [&] {
    exitAction = ExitAction::Save;
    closeScreen();
  });
  auto discardAndExitButton = Button("Discard & Exit", [&] {
    if (hasUnsavedChanges()) {
      showDiscardConfirm = true;
      return;
    }
    exitAction = ExitAction::Discard;
    closeScreen();
  });
  Component restoreBackupButton;
  
  vector<string> toggleEntries = {
    "  Disable  ",
    "  Enable  "
  };

  vector<string> ttlEntries = {" 0 ", " 1 ", " 2 ", " 3 ", " 4 ", " 5 "};

  int tcpSendSelected = ndiConfig["ndi"]["tcp"]["send"]["enable"];
  int tcpRecvSelected = ndiConfig["ndi"]["tcp"]["recv"]["enable"];
  int rudpSendSelected = ndiConfig["ndi"]["rudp"]["send"]["enable"];
  int rudpRecvSelected = ndiConfig["ndi"]["rudp"]["recv"]["enable"];
  int unicastSendSelected = ndiConfig["ndi"]["unicast"]["send"]["enable"];
  int unicastRecvSelected = ndiConfig["ndi"]["unicast"]["recv"]["enable"];
  int multicastSendSelected = ndiConfig["ndi"]["multicast"]["send"]["enable"];
  int multicastRecvSelected = ndiConfig["ndi"]["multicast"]["recv"]["enable"]; 
  int multicastSendTTL = ndiConfig["ndi"]["multicast"]["send"]["ttl"];

  auto normalizeToggleSelection = [](int value) {
    return value == 0 ? 0 : 1;
  };

  tcpSendSelected = normalizeToggleSelection(tcpSendSelected);
  tcpRecvSelected = normalizeToggleSelection(tcpRecvSelected);
  rudpSendSelected = normalizeToggleSelection(rudpSendSelected);
  rudpRecvSelected = normalizeToggleSelection(rudpRecvSelected);
  unicastSendSelected = normalizeToggleSelection(unicastSendSelected);
  unicastRecvSelected = normalizeToggleSelection(unicastRecvSelected);
  multicastSendSelected = normalizeToggleSelection(multicastSendSelected);
  multicastRecvSelected = normalizeToggleSelection(multicastRecvSelected);
  multicastSendTTL = std::clamp(multicastSendTTL, 0, static_cast<int>(ttlEntries.size()) - 1);

  string sendGroups = ndiConfig["ndi"]["groups"]["send"];
  string recvGroups = ndiConfig["ndi"]["groups"]["recv"];
  string discoveryServers = ndiConfig["ndi"]["networks"]["discovery"];
  string ips = ndiConfig["ndi"]["networks"]["ips"];
  string machineName = ndiConfig["ndi"]["machinename"];  
  string multicastSendNetmask = ndiConfig["ndi"]["multicast"]["send"]["netmask"];
  string multicastSendNetprefix = ndiConfig["ndi"]["multicast"]["send"]["netprefix"];

  const int initialTcpSendSelected = tcpSendSelected;
  const int initialTcpRecvSelected = tcpRecvSelected;
  const int initialRudpSendSelected = rudpSendSelected;
  const int initialRudpRecvSelected = rudpRecvSelected;
  const int initialUnicastSendSelected = unicastSendSelected;
  const int initialUnicastRecvSelected = unicastRecvSelected;
  const int initialMulticastSendSelected = multicastSendSelected;
  const int initialMulticastRecvSelected = multicastRecvSelected;
  const int initialMulticastSendTTL = multicastSendTTL;
  const string initialSendGroups = sendGroups;
  const string initialRecvGroups = recvGroups;
  const string initialDiscoveryServers = discoveryServers;
  const string initialIps = ips;
  const string initialMachineName = machineName;
  const string initialMulticastSendNetmask = multicastSendNetmask;
  const string initialMulticastSendNetprefix = multicastSendNetprefix;

  hasUnsavedChanges = [&] {
    return tcpSendSelected != initialTcpSendSelected ||
      tcpRecvSelected != initialTcpRecvSelected ||
      rudpSendSelected != initialRudpSendSelected ||
      rudpRecvSelected != initialRudpRecvSelected ||
      unicastSendSelected != initialUnicastSendSelected ||
      unicastRecvSelected != initialUnicastRecvSelected ||
      multicastSendSelected != initialMulticastSendSelected ||
      multicastRecvSelected != initialMulticastRecvSelected ||
      multicastSendTTL != initialMulticastSendTTL ||
      sendGroups != initialSendGroups ||
      recvGroups != initialRecvGroups ||
      discoveryServers != initialDiscoveryServers ||
      ips != initialIps ||
      machineName != initialMachineName ||
      multicastSendNetmask != initialMulticastSendNetmask ||
      multicastSendNetprefix != initialMulticastSendNetprefix;
  };

  restoreBackupButton = Button("Restore Backup", [&] {
    if (!std::filesystem::exists(backupConfigPath)) {
      restoreStatusIsError = true;
      restoreStatusMessage = "No backup file found";
      return;
    }

    ifstream backupInputFile(backupConfigPath);
    if (!backupInputFile.is_open()) {
      restoreStatusIsError = true;
      restoreStatusMessage = "Could not open backup file";
      return;
    }

    json backupConfig;
    try {
      backupInputFile >> backupConfig;
      generateMissingConfig(backupConfig);
    } catch (const json::exception&) {
      restoreStatusIsError = true;
      restoreStatusMessage = "Backup file is invalid JSON";
      return;
    }

    tcpSendSelected = normalizeToggleSelection(backupConfig["ndi"]["tcp"]["send"]["enable"]);
    tcpRecvSelected = normalizeToggleSelection(backupConfig["ndi"]["tcp"]["recv"]["enable"]);
    rudpSendSelected = normalizeToggleSelection(backupConfig["ndi"]["rudp"]["send"]["enable"]);
    rudpRecvSelected = normalizeToggleSelection(backupConfig["ndi"]["rudp"]["recv"]["enable"]);
    unicastSendSelected = normalizeToggleSelection(backupConfig["ndi"]["unicast"]["send"]["enable"]);
    unicastRecvSelected = normalizeToggleSelection(backupConfig["ndi"]["unicast"]["recv"]["enable"]);
    multicastSendSelected = normalizeToggleSelection(backupConfig["ndi"]["multicast"]["send"]["enable"]);
    multicastRecvSelected = normalizeToggleSelection(backupConfig["ndi"]["multicast"]["recv"]["enable"]);
    multicastSendTTL = std::clamp(
      static_cast<int>(backupConfig["ndi"]["multicast"]["send"]["ttl"]),
      0,
      static_cast<int>(ttlEntries.size()) - 1
    );

    sendGroups = backupConfig["ndi"]["groups"]["send"];
    recvGroups = backupConfig["ndi"]["groups"]["recv"];
    discoveryServers = backupConfig["ndi"]["networks"]["discovery"];
    ips = backupConfig["ndi"]["networks"]["ips"];
    machineName = backupConfig["ndi"]["machinename"];
    multicastSendNetmask = backupConfig["ndi"]["multicast"]["send"]["netmask"];
    multicastSendNetprefix = backupConfig["ndi"]["multicast"]["send"]["netprefix"];

    restoreStatusIsError = false;
    restoreStatusMessage = "Loaded backup into form";
  });

  auto confirmDiscardButton = Button("Yes, Discard", [&] {
    exitAction = ExitAction::Discard;
    showDiscardConfirm = false;
    closeScreen();
  });
  auto cancelDiscardButton = Button("Cancel", [&] {
    showDiscardConfirm = false;
  });

  Component discardConfirmContainer = Container::Vertical({
    confirmDiscardButton,
    cancelDiscardButton
  });
  Component discardConfirmDialog = Renderer(discardConfirmContainer, [&] {
    return window(
      text("Discard Unsaved Changes?") | bold | center,
      vbox({
        text("You have unsaved edits."),
        text("Discard and exit anyway?") | dim,
        separator(),
        hbox(confirmDiscardButton->Render(), text("  "), cancelDiscardButton->Render()) | center,
      })
    ) | center;
  });
  
  Component tcpSendToggle = Toggle(&toggleEntries, &tcpSendSelected);
  Component tcpRecvToggle = Toggle(&toggleEntries, &tcpRecvSelected);
  Component rudpSendToggle = Toggle(&toggleEntries, &rudpSendSelected);
  Component rudpRecvToggle = Toggle(&toggleEntries, &rudpRecvSelected);
  Component unicastSendToggle = Toggle(&toggleEntries, &unicastSendSelected);
  Component unicastRecvToggle = Toggle(&toggleEntries, &unicastRecvSelected);
  Component multicastSendToggle = Toggle(&toggleEntries, &multicastSendSelected);
  Component multicastRecvToggle = Toggle(&toggleEntries, &multicastRecvSelected);
  Component multicastSendTTLToggle = Toggle(&ttlEntries, &multicastSendTTL);
  
  Component sendGroupInput = Input(&sendGroups, "Public, Group1, Group2");
  Component recvGroupInput = Input(&recvGroups, "Public, Group1, Group2");
  Component discoveryServersInput = Input(&discoveryServers, "192.168.1.21,192.168.1.22");
  Component ipsInput = Input(&ips, "192.168.1.1,192.168.1.2");
  Component machineNameInput = Input(&machineName, "My Machine Name");
  Component multicastSendNetmaskInput =  Input(&multicastSendNetmask, "   255.255.0.0   ");
  Component multicastSendNetprefixInput = Input(&multicastSendNetprefix, "   239.255.0.0   ");

  Component tcpContainer = Container::Vertical({ 
    tcpSendToggle,
    tcpRecvToggle
  });

  Component rudpContainer = Container::Vertical({ 
    rudpSendToggle,
    rudpRecvToggle
  });

  Component unicastContainer = Container::Vertical({
    unicastSendToggle,
    unicastRecvToggle
  });

  Component modesRowContainer = Container::Vertical({
    tcpContainer,
    rudpContainer,
    unicastContainer
  });

  Component multicastContainer = Container::Vertical({
    multicastSendToggle,
    multicastRecvToggle,
    multicastSendNetmaskInput,
    multicastSendNetprefixInput,
    multicastSendTTLToggle
  });

  Component mainContainer = Container::Vertical({ 
    machineNameInput,
    discoveryServersInput,
    ipsInput,
    sendGroupInput,
    recvGroupInput,
    modesRowContainer,
    multicastContainer,
    saveAndExitButton,
    discardAndExitButton,
    restoreBackupButton
  });

  
  auto renderer = Renderer(mainContainer, [&] {
    TerminalSize termSize = getTerminalSize();
    const bool terminalTooSmall =
      termSize.width > 0 && termSize.height > 0 &&
      (termSize.width < minTerminalWidth || termSize.height < minTerminalHeight);

    if (terminalTooSmall) {
      return vbox({
        filler(),
        text("Terminal too small for Access Manager") | bold | center,
        text("Minimum size: " + std::to_string(minTerminalWidth) + "x" + std::to_string(minTerminalHeight) +
             " (current: " + std::to_string(termSize.width) + "x" + std::to_string(termSize.height) + ")") | center,
        text("Resize your terminal to continue.") | dim | center,
        filler(),
      });
    }

    const bool dirty = hasUnsavedChanges();

    Element changeStatus = dirty
      ? text("Unsaved changes") | bold | color(Color::Yellow) | center
      : text("All changes saved") | color(Color::Green) | center;

    const bool backupExists = std::filesystem::exists(backupConfigPath);
    Element backupStatus = backupExists
      ? text("Backup file found") | color(Color::Green)
      : text("No backup file found") | color(Color::Yellow);

    Element restoreLoadStatus = restoreStatusIsError
      ? text(restoreStatusMessage) | color(Color::Red)
      : text(restoreStatusMessage) | color(Color::Green);

    auto layout = vbox({
      text(""),
      text(titleL1) | center,
      text(titleL2) | center,
      text(titleL3) | center,
      text(titleL4) | center,
      text(titleL5) | center,
      text(""),
      text("A TUI Access Manager for NDI on Linux") | bold | center,
      text("Version 1.0") | bold | center,
  
      hbox(
        border(vbox(
          text("    Machine Name    ") | bold | center,
          separator(),
          machineNameInput->Render()
          ) | center
        )
      ) | center,

      hbox(
        border(vbox(
          text("                   Network                  ") | bold | center,
          separator(),
          hbox(text(" Discovery "), separator(), discoveryServersInput->Render()),
          hbox(text("    IPs    "), separator(), ipsInput->Render())
          ) | center
        ),
      border(vbox(
          text("                    Groups                   ") | bold | center,
          separator(),
          hbox(text(" Send "), separator(), sendGroupInput->Render()),
          hbox(text(" Recv "), separator(), recvGroupInput->Render())
          ) | center
        )
      ) | center,
      
      hbox(
        border(vbox(
          text("Multi-TCP") | bold | center,
          separator(),
          hbox(text(" Send ") ,separator(),tcpSendToggle->Render()),
          hbox(text(" Recv ") ,separator(),tcpRecvToggle->Render())
        ) | center),
        border(vbox(
          text("RUDP") | bold | center,
          separator(), 
          hbox(text(" Send ") ,separator(),rudpSendToggle->Render()),
          hbox(text(" Recv ") ,separator(),rudpRecvToggle->Render())
        ) | center),
        border(vbox(
          text("Unicast") | bold | center, 
          separator(), 
          hbox(text(" Send ") ,separator(),unicastSendToggle->Render()),
          hbox(text(" Recv ") ,separator(),unicastRecvToggle->Render())
        ) | center
      ) | center ) | center,

      hbox(
        border(
          vbox(
            text("     Multicast Enables     ") | bold | center,
            separator(),
            hbox(
              vbox(
                hbox(text(" Send ") | bold | center, separator(),multicastSendToggle->Render()),
                hbox(text(" Recv ") | bold | center, separator(),multicastRecvToggle->Render())
                )
              )
            ) | center
          ) | center,

      border(
        vbox(
          text("    Multicast IP Settings    ") | bold | center,
          separator(),
          hbox(
            vbox( 
                hbox(text("  Netmask  ") | bold | center, separator(),multicastSendNetmaskInput->Render()),
                hbox(text(" NetPrefix ") | bold | center, separator(),multicastSendNetprefixInput->Render())
                )
              )
            )
          ),
      border(
        vbox(
          text("        Multicast        ") | bold | center,
          text("TTL") | bold | center,
          separator(),
          hbox(
            vbox( 
                hbox(text(" TTL ") | bold | center, separator(),multicastSendTTLToggle->Render())
                ) | center
              )
            ) | center
          ) | center

        ) | center,

      separator(),

      border(
        hbox(
          vbox(
            text("Status") | bold | center,
            separator(),
            hbox(text(" Changes "), separator(), text("  "), changeStatus, text("  ")),
            hbox(text("         "), separator(), text("  "), text("  "), text("  "))
          ) | flex,
          separator(),
          vbox(
            text("Backup") | bold | center,
            separator(),
            hbox(text(" Status  "), separator(), text("  "), backupStatus, text("  ")),
            hbox(text(" Restore "), separator(), text("  "), restoreLoadStatus, text("  "))
          ) | flex
        )
      ) | center,

      hbox(
        saveAndExitButton->Render(),
        text("  "),
        discardAndExitButton->Render(),
        text("  "),
        restoreBackupButton->Render()
      ) | center,
      
      separator(),

      text("By Suhaili Labs | NDI® is a registered trademark of Vizrt NDI AB") | dim | center,

    });
    return layout;  
  });

  auto app = Modal(renderer, discardConfirmDialog, &showDiscardConfirm);

  screen.Loop(app);
  
  // End TUI

  if (exitAction == ExitAction::Discard) {
    cout << "Changes discarded. Existing config was not modified." << endl;
    return 0;
  }

  discoveryServerSet(discoveryServers, ndiConfig);
  discoveryIpsSet(ips, ndiConfig);
  groupsSet(sendGroups, recvGroups, ndiConfig);
  tcpSet(tcpSendSelected, tcpRecvSelected, ndiConfig);
  rudpSet(rudpSendSelected, rudpRecvSelected, ndiConfig);
  unicastSet(unicastSendSelected,unicastRecvSelected, ndiConfig);
  machineNameSet(machineName, ndiConfig);
  multicastSendSet(multicastSendSelected, multicastSendNetmask, multicastSendNetprefix, multicastSendTTL, ndiConfig);
  multicastRecvSet(multicastRecvSelected, ndiConfig);

  const std::filesystem::path tempConfigPath = configPath.string() + ".tmp";

  ofstream outputFile(tempConfigPath);
  if (!outputFile.is_open()) {
    std::cerr << "Could not open temp NDI config JSON: " << tempConfigPath << endl;
    return 1;
  }

  outputFile << ndiConfig.dump(2);
  outputFile.flush();
  if (!outputFile.good()) {
    std::cerr << "Failed to write temp NDI config JSON: " << tempConfigPath << endl;
    outputFile.close();
    std::error_code cleanupError;
    std::filesystem::remove(tempConfigPath, cleanupError);
    return 1;
  }
  outputFile.close();

  if (std::filesystem::exists(configPath)) {
    std::error_code backupError;
    std::filesystem::copy_file(
      configPath,
      backupConfigPath,
      std::filesystem::copy_options::overwrite_existing,
      backupError
    );
    if (backupError) {
      std::cerr << "Warning: could not create NDI config backup at " << backupConfigPath
                << ": " << backupError.message() << endl;
    }
  }

  std::error_code renameError;
  std::filesystem::rename(tempConfigPath, configPath, renameError);
  if (renameError) {
    std::cerr << "Could not atomically replace NDI config JSON: " << renameError.message() << endl;
    std::error_code cleanupError;
    std::filesystem::remove(tempConfigPath, cleanupError);
    return 1;
  }

  return 0;

}
