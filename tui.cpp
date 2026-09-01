#include <iostream>
#include <algorithm>
#include <filesystem>
#include <functional>
#include <sys/ioctl.h>
#include <unistd.h>
#include "json.hpp"
#include "accessman.hpp"
#include "tui_support.hpp"

#include <ftxui/component/component.hpp>  // for Button, Renderer
#include <ftxui/component/component_options.hpp>  // for Catch
#include <ftxui/screen/screen.hpp>  // for Screen
#include <ftxui/dom/elements.hpp>  // for text, vbox
#include <ftxui/component/screen_interactive.hpp>  // for ScreenInteractive

using std::cout;
using nlohmann::json;
using std::string;
using std::endl;
using std::vector;
using namespace ftxui;
using namespace tui_support;

#ifndef APP_VERSION
#define APP_VERSION "0.0.0-dev"
#endif

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
  const string appVersion = APP_VERSION;

  const ConfigPaths paths = getConfigPaths();
  if (!ensureConfigDirectory(paths.ndiDir)) {
    return 1;
  }

  const std::filesystem::path& configPath = paths.configPath;
  const std::filesystem::path& backupConfigPath = paths.backupConfigPath;

  cout << "NDI Config Dir: " << configPath << endl;

  json ndiConfig = loadConfigWithDefaults(configPath);

// TUI BELOW


  string titleL1 = " ▄▀█ █▀▀ █▀▀ █▀▀ █▀▀ █▀▀   █▀▄▀█ ▄▀█ █▄ █ ▄▀█ █▀▀ █▀▀ █▀█";
  string titleL2 = " █▀█ █▄▄ █▄▄ ██▄ ▄▄█ ▄▄█   █ ▀ █ █▀█ █ ▀█ █▀█ █▄█ ██▄ █▀▄";




  auto screen = ScreenInteractive::Fullscreen();
  enum class ExitAction {
    Discard,
    Save,
  };

  ExitAction exitAction = ExitAction::Discard;
  bool showDiscardConfirm = false;
  bool showRestoreConfirm = false;
  string restoreStatusMessage = "Not loaded";
  bool restoreStatusIsError = false;
  string validationStatusMessage = "Ready";
  bool validationStatusIsError = false;
  bool validationAttempted = false;
  std::function<bool()> hasUnsavedChanges = [] {
    return false;
  };
  std::function<bool(string&)> validateBeforeSave = [](string&) {
    return true;
  };
  auto closeScreen = screen.ExitLoopClosure();
  auto saveAndExitButton = Button("Save & Exit", [&] {
    if (!hasUnsavedChanges()) {
      validationStatusIsError = false;
      validationStatusMessage = "No changes to save";
      return;
    }

    validationAttempted = true;
    string validationError;
    if (!validateBeforeSave(validationError)) {
      validationStatusIsError = true;
      validationStatusMessage = validationError;
      return;
    }
    validationStatusIsError = false;
    validationStatusMessage = "Validation passed";
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

  // Checkboxes (Enter/Space flip, arrows navigate) instead of Toggles
  // (Left/Right flipped the value). TTL is a validating text Input for the
  // same reason.
  bool tcpSendSelected = ndiConfig["ndi"]["tcp"]["send"]["enable"];
  bool tcpRecvSelected = ndiConfig["ndi"]["tcp"]["recv"]["enable"];
  bool rudpSendSelected = ndiConfig["ndi"]["rudp"]["send"]["enable"];
  bool rudpRecvSelected = ndiConfig["ndi"]["rudp"]["recv"]["enable"];
  bool unicastSendSelected = ndiConfig["ndi"]["unicast"]["send"]["enable"];
  bool unicastRecvSelected = ndiConfig["ndi"]["unicast"]["recv"]["enable"];
  bool multicastSendSelected = ndiConfig["ndi"]["multicast"]["send"]["enable"];
  bool multicastRecvSelected = ndiConfig["ndi"]["multicast"]["recv"]["enable"];
  string multicastSendTTL = std::to_string(static_cast<int>(ndiConfig["ndi"]["multicast"]["send"]["ttl"]));

  string sendGroups = ndiConfig["ndi"]["groups"]["send"];
  string recvGroups = ndiConfig["ndi"]["groups"]["recv"];
  string discoveryServers = ndiConfig["ndi"]["networks"]["discovery"];
  string ips = ndiConfig["ndi"]["networks"]["ips"];
  string machineName = ndiConfig["ndi"]["machinename"];
  string multicastRecvSubnets = jsonArrayToCsv(ndiConfig["ndi"]["multicast"]["recv"]["subnets"]);
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
  const string initialMulticastSendTTL = multicastSendTTL;
  const string initialSendGroups = sendGroups;
  const string initialRecvGroups = recvGroups;
  const string initialDiscoveryServers = discoveryServers;
  const string initialIps = ips;
  const string initialMachineName = machineName;
  const string initialMulticastRecvSubnets = multicastRecvSubnets;
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
      multicastRecvSubnets != initialMulticastRecvSubnets ||
      multicastSendNetmask != initialMulticastSendNetmask ||
      multicastSendNetprefix != initialMulticastSendNetprefix;
  };

  validateBeforeSave = [&](string& errorMessage) {
    if (!validateDiscoveryCsv(discoveryServers)) {
      errorMessage = "Discovery list must contain valid IPv4[:port] values";
      return false;
    }
    if (!validateCsvIPv4(ips)) {
      errorMessage = "IPs list must contain valid IPv4 values";
      return false;
    }
    if (!validateCsvCidr(multicastRecvSubnets)) {
      errorMessage = "Multicast recv subnets must be valid CIDR values";
      return false;
    }

    const string ttlTrimmed = trim(multicastSendTTL);
    const bool ttlNumeric = !ttlTrimmed.empty() &&
      std::all_of(ttlTrimmed.begin(), ttlTrimmed.end(),
                  [](char c) { return std::isdigit(static_cast<unsigned char>(c)); });
    if (!ttlNumeric || std::stoi(ttlTrimmed) > 255) {
      errorMessage = "Multicast TTL must be a number 0-255";
      return false;
    }

    const string netmaskForValidation = trim(multicastSendNetmask);
    const string netprefixForValidation = trim(multicastSendNetprefix);

    if (!netmaskForValidation.empty() && !isValidNetmask(netmaskForValidation)) {
      errorMessage = "Multicast netmask is invalid";
      return false;
    }
    if (!netprefixForValidation.empty() && !isValidMulticastPrefix(netprefixForValidation)) {
      errorMessage = "Multicast netprefix must be in 224.0.0.0/4";
      return false;
    }
    return true;
  };

  std::function<void()> loadBackupIntoForm = [] {};

  loadBackupIntoForm = [&] {
    json backupConfig;
    string backupError;
    if (!loadBackupConfig(backupConfigPath, backupConfig, backupError)) {
      restoreStatusIsError = true;
      restoreStatusMessage = backupError;
      return;
    }

    tcpSendSelected = backupConfig["ndi"]["tcp"]["send"]["enable"];
    tcpRecvSelected = backupConfig["ndi"]["tcp"]["recv"]["enable"];
    rudpSendSelected = backupConfig["ndi"]["rudp"]["send"]["enable"];
    rudpRecvSelected = backupConfig["ndi"]["rudp"]["recv"]["enable"];
    unicastSendSelected = backupConfig["ndi"]["unicast"]["send"]["enable"];
    unicastRecvSelected = backupConfig["ndi"]["unicast"]["recv"]["enable"];
    multicastSendSelected = backupConfig["ndi"]["multicast"]["send"]["enable"];
    multicastRecvSelected = backupConfig["ndi"]["multicast"]["recv"]["enable"];
    multicastSendTTL = std::to_string(static_cast<int>(backupConfig["ndi"]["multicast"]["send"]["ttl"]));

    sendGroups = backupConfig["ndi"]["groups"]["send"];
    recvGroups = backupConfig["ndi"]["groups"]["recv"];
    discoveryServers = backupConfig["ndi"]["networks"]["discovery"];
    ips = backupConfig["ndi"]["networks"]["ips"];
    machineName = backupConfig["ndi"]["machinename"];
    multicastRecvSubnets = jsonArrayToCsv(backupConfig["ndi"]["multicast"]["recv"]["subnets"]);
    multicastSendNetmask = backupConfig["ndi"]["multicast"]["send"]["netmask"];
    multicastSendNetprefix = backupConfig["ndi"]["multicast"]["send"]["netprefix"];

    restoreStatusIsError = false;
    restoreStatusMessage = "Loaded backup into form";
    validationAttempted = false;
    validationStatusIsError = false;
    validationStatusMessage = "Ready";
  };

  restoreBackupButton = Button("Restore Previous Config", [&] {
    if (hasUnsavedChanges()) {
      showRestoreConfirm = true;
      return;
    }
    loadBackupIntoForm();
  });

  auto confirmDiscardButton = Button("Yes, Discard", [&] {
    exitAction = ExitAction::Discard;
    showDiscardConfirm = false;
    closeScreen();
  });
  auto cancelDiscardButton = Button("Cancel", [&] {
    showDiscardConfirm = false;
  });

  auto confirmRestoreButton = Button("Yes, Load Backup", [&] {
    showRestoreConfirm = false;
    loadBackupIntoForm();
  });
  auto cancelRestoreButton = Button("Cancel", [&] {
    showRestoreConfirm = false;
  });

  Component discardConfirmContainer = Container::Vertical({
    confirmDiscardButton,
    cancelDiscardButton
  });
  Component discardConfirmDialog = Renderer(discardConfirmContainer, [&] {
    return window(
      text("Discard Unsaved Changes?") | bold | center,
      vbox({
        text(""),
        hbox(text("   "), text("You have unsaved edits."), text("   ")),
        hbox(text("   "), text("Discard and exit anyway?") | dim, text("   ")),
        separator(),
        hbox(text("   "), confirmDiscardButton->Render(), text("  "), cancelDiscardButton->Render(), text("   ")) | center,
        text(""),
      })
    ) | color(Color::White) | bgcolor(Color::RGB(95, 12, 12)) | center;
  });

  Component restoreConfirmContainer = Container::Vertical({
    confirmRestoreButton,
    cancelRestoreButton
  });
  Component restoreConfirmDialog = Renderer(restoreConfirmContainer, [&] {
    return window(
      text("Load Backup Over Unsaved Edits?") | bold | center,
      vbox({
        text(""),
        hbox(text("   "), text("You have unsaved edits."), text("   ")),
        hbox(text("   "), text("Load backup into the form anyway?") | dim, text("   ")),
        separator(),
        hbox(text("   "), confirmRestoreButton->Render(), text("  "), cancelRestoreButton->Render(), text("   ")) | center,
        text(""),
      })
    ) | color(Color::White) | bgcolor(Color::RGB(95, 12, 12)) | center;
  });
  
  Component tcpSendToggle = Checkbox(" Enable", &tcpSendSelected);
  Component tcpRecvToggle = Checkbox(" Enable", &tcpRecvSelected);
  Component rudpSendToggle = Checkbox(" Enable", &rudpSendSelected);
  Component rudpRecvToggle = Checkbox(" Enable", &rudpRecvSelected);
  Component unicastSendToggle = Checkbox(" Enable", &unicastSendSelected);
  Component unicastRecvToggle = Checkbox(" Enable", &unicastRecvSelected);
  Component multicastSendToggle = Checkbox(" Enable", &multicastSendSelected);
  Component multicastRecvToggle = Checkbox(" Enable", &multicastRecvSelected);
  Component multicastSendTTLInput = Input(&multicastSendTTL, "1");
  Component multicastRecvSubnetsInput = Input(&multicastRecvSubnets, "10.28.5.0/24, 10.28.4.0/24");
  
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

  // Horizontal so Left/Right move focus between protocols natively.
  Component multicastEnablesBox = Container::Vertical({
    multicastSendToggle,
    multicastRecvToggle
  });

  // Protocols + multicast enables in one band; wider columns so labels don't
  // wrap. Left/Right moves across the four boxes; Up/Down moves within one.
  Component modesRowContainer = Container::Horizontal({
    tcpContainer,
    rudpContainer,
    unicastContainer,
    multicastEnablesBox
  });

  Component multicastSettingsBox = Container::Vertical({
    multicastSendNetmaskInput,
    multicastSendNetprefixInput
  });

  Component multicastTtlBox = Container::Vertical({
    multicastSendTTLInput
  });

  // Each band is Vertical-of-one-horizontal-child. Up/Down bubbles through
  // the nearest Vertical wrapper (Vertical consumes the event only when it
  // changes focus), so focus walks row-by-row without skipping boxes.
  Component machineNameRow = Container::Vertical({ machineNameInput });

  Component networkInputsBox = Container::Vertical({
    discoveryServersInput,
    ipsInput
  });

  Component groupsInputsBox = Container::Vertical({
    sendGroupInput,
    recvGroupInput
  });

  Component networkGroupsRowInner = Container::Horizontal({
    networkInputsBox,
    groupsInputsBox
  });

  Component networkGroupsRow = Container::Vertical({ networkGroupsRowInner });

  // (modesRowContainer above now includes multicastEnablesBox.)

  Component multicastRowInner = Container::Horizontal({
    multicastSettingsBox,
    multicastTtlBox
  });
  Component multicastRow = Container::Vertical({ multicastRowInner });

  Component bottomButtonsRowInner = Container::Horizontal({
    saveAndExitButton,
    discardAndExitButton,
    restoreBackupButton
  });
  Component bottomButtonsRow = Container::Vertical({ bottomButtonsRowInner });

  Component mainContainer = Container::Vertical({
    machineNameRow,
    networkGroupsRow,
    modesRowContainer,
    multicastRow,
    multicastRecvSubnetsInput,
    bottomButtonsRow
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

    const string netmaskForValidation = trim(multicastSendNetmask);
    const string netprefixForValidation = trim(multicastSendNetprefix);

    const bool changedDiscovery = discoveryServers != initialDiscoveryServers;
    const bool changedIps = ips != initialIps;
    const bool changedMachineName = machineName != initialMachineName;
    const bool changedSendGroups = sendGroups != initialSendGroups;
    const bool changedRecvGroups = recvGroups != initialRecvGroups;
    const bool changedTcpSend = tcpSendSelected != initialTcpSendSelected;
    const bool changedTcpRecv = tcpRecvSelected != initialTcpRecvSelected;
    const bool changedRudpSend = rudpSendSelected != initialRudpSendSelected;
    const bool changedRudpRecv = rudpRecvSelected != initialRudpRecvSelected;
    const bool changedUnicastSend = unicastSendSelected != initialUnicastSendSelected;
    const bool changedUnicastRecv = unicastRecvSelected != initialUnicastRecvSelected;
    const bool changedMulticastSend = multicastSendSelected != initialMulticastSendSelected;
    const bool changedMulticastRecv = multicastRecvSelected != initialMulticastRecvSelected;
    const bool changedMulticastRecvSubnets = multicastRecvSubnets != initialMulticastRecvSubnets;
    const bool changedMulticastTtl = multicastSendTTL != initialMulticastSendTTL;
    const bool changedNetmask = multicastSendNetmask != initialMulticastSendNetmask;
    const bool changedNetprefix = multicastSendNetprefix != initialMulticastSendNetprefix;

    const bool invalidDiscovery = validationAttempted && !validateDiscoveryCsv(discoveryServers);
    const bool invalidIps = validationAttempted && !validateCsvIPv4(ips);
    const bool invalidMulticastRecvSubnets = validationAttempted && !validateCsvCidr(multicastRecvSubnets);
    const string ttlForValidation = trim(multicastSendTTL);
    const bool invalidTtl = validationAttempted &&
      (ttlForValidation.empty() ||
       !std::all_of(ttlForValidation.begin(), ttlForValidation.end(),
                    [](char c) { return std::isdigit(static_cast<unsigned char>(c)); }) ||
       std::stoi(ttlForValidation) > 255);
    const bool invalidNetmask =
      validationAttempted && !netmaskForValidation.empty() && !isValidNetmask(netmaskForValidation);
    const bool invalidNetprefix =
      validationAttempted && !netprefixForValidation.empty() && !isValidMulticastPrefix(netprefixForValidation);

    auto colorizeRow = [](Element row, bool changed, bool invalid) {
      if (invalid) {
        return row | color(Color::Red);
      }
      if (changed) {
        return row | color(Color::Yellow);
      }
      return row;
    };

    Element restoreLoadStatus = restoreStatusIsError
      ? text(restoreStatusMessage) | color(Color::Red)
      : text(restoreStatusMessage) | color(Color::Green);

    Element validationSaveStatus = validationStatusIsError
      ? text(validationStatusMessage) | color(Color::Red)
      : text(validationStatusMessage) | color(Color::Green);

    Element saveButtonElement = saveAndExitButton->Render();
    if (!dirty) {
      saveButtonElement = saveButtonElement | dim;
    }

    auto layout = vbox({
      text(""),
      text(titleL1) | center,
      text(titleL2) | center,
      text(""),
      text("A TUI Access Manager for NDI on Linux") | bold | center,
      text("Version " + appVersion) | bold | center,
  
      hbox(
        border(vbox(
          text("    Machine Name    ") | bold | center,
          separator(),
          colorizeRow(hbox(text(" Name "), separator(), machineNameInput->Render()), changedMachineName, false)
          ) | center
        )
      ) | center,

      hbox(
        border(vbox(
          text("                   Network                  ") | bold | center,
          separator(),
          colorizeRow(
            hbox(text(" Discovery "), separator(), discoveryServersInput->Render()),
            changedDiscovery,
            invalidDiscovery
          ),
          colorizeRow(
            hbox(text("    IPs    "), separator(), ipsInput->Render()),
            changedIps,
            invalidIps
          )
          ) | center
        ),
      border(vbox(
          text("                    Groups                   ") | bold | center,
          separator(),
          colorizeRow(hbox(text(" Send "), separator(), sendGroupInput->Render()), changedSendGroups, false),
          colorizeRow(hbox(text(" Recv "), separator(), recvGroupInput->Render()), changedRecvGroups, false)
          ) | center
        )
      ) | center,
      
      hbox(
        border(vbox(
          text("Multi-TCP") | bold | center,
          separator(),
          colorizeRow(hbox(text(" Send ") ,separator(),tcpSendToggle->Render()) | size(WIDTH, GREATER_THAN, 18), changedTcpSend, false),
          colorizeRow(hbox(text(" Recv ") ,separator(),tcpRecvToggle->Render()) | size(WIDTH, GREATER_THAN, 18), changedTcpRecv, false)
        ) | center | size(WIDTH, GREATER_THAN, 21)),
        border(vbox(
          text("RUDP") | bold | center,
          separator(),
          colorizeRow(hbox(text(" Send ") ,separator(),rudpSendToggle->Render()) | size(WIDTH, GREATER_THAN, 18), changedRudpSend, false),
          colorizeRow(hbox(text(" Recv ") ,separator(),rudpRecvToggle->Render()) | size(WIDTH, GREATER_THAN, 18), changedRudpRecv, false)
        ) | center | size(WIDTH, GREATER_THAN, 21)),
        border(vbox(
          text("Unicast") | bold | center,
          separator(),
          colorizeRow(hbox(text(" Send ") ,separator(),unicastSendToggle->Render()) | size(WIDTH, GREATER_THAN, 18), changedUnicastSend, false),
          colorizeRow(hbox(text(" Recv ") ,separator(),unicastRecvToggle->Render()) | size(WIDTH, GREATER_THAN, 18), changedUnicastRecv, false)
        ) | center | size(WIDTH, GREATER_THAN, 21)),
        border(vbox(
          text(" Mcast ") | bold | center,
          separator(),
          colorizeRow(hbox(text(" Send ") ,separator(),multicastSendToggle->Render()) | size(WIDTH, GREATER_THAN, 18), changedMulticastSend, false),
          colorizeRow(hbox(text(" Recv ") ,separator(),multicastRecvToggle->Render()) | size(WIDTH, GREATER_THAN, 18), changedMulticastRecv, false)
        ) | center | size(WIDTH, GREATER_THAN, 21))
      ) | center,

      hbox(
        border(
          vbox(
            text("    Multicast Send Settings    ") | bold | center,
            separator(),
            hbox(
              vbox(
                colorizeRow(
                  hbox(text("  Netmask  ") | bold | center, separator(),multicastSendNetmaskInput->Render()),
                  changedNetmask,
                  invalidNetmask
                ),
                colorizeRow(
                  hbox(text(" NetPrefix ") | bold | center, separator(),multicastSendNetprefixInput->Render()),
                  changedNetprefix,
                  invalidNetprefix
                )
              ) | center
            )
          )
        ),
        border(
          vbox(
            text("  Multicast  ") | bold | center,
            text("   TTL    ") | bold | center,
            separator(),
            hbox(
              vbox(
                colorizeRow(
                  hbox(text(" TTL ") | bold | center, separator(),multicastSendTTLInput->Render() | size(WIDTH, EQUAL, 5)),
                  changedMulticastTtl,
                  invalidTtl
                )
              ) | center
            )
          )
        ) | center
      ) | center,

      border(
        vbox(
          text("Multicast Recv Subnets") | bold | center,
          separator(),
          colorizeRow(
            hbox(
              text(" Subnets "),
              separator(),
              multicastRecvSubnetsInput->Render() | size(WIDTH, EQUAL, 38)
            ) | center,
            changedMulticastRecvSubnets,
            invalidMulticastRecvSubnets
          )
        )
      ) | center,

      border(
        hbox(
          vbox(
            text("Status") | bold | center,
            separator(),
            hbox(text(" Changes "), separator(), text("  "), changeStatus, text("  ")),
            hbox(text(" Save    "), separator(), text("  "), validationSaveStatus, text("  "))
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
        saveButtonElement,
        text("  "),
        discardAndExitButton->Render(),
        text("  "),
        restoreBackupButton->Render()
      ) | center,

      text(""),
      
      text("Navigation: Up/Down rows | Left/Right groups | Enter/Space toggle checkbox | type in text fields") | dim | center,

      text(""),

      text("By Suhaili Labs | NDI® is a registered trademark of Vizrt NDI AB") | dim | center,

    });
    return layout;  
  });

  auto app = Modal(renderer, discardConfirmDialog, &showDiscardConfirm);
  app = Modal(app, restoreConfirmDialog, &showRestoreConfirm);

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

  const string multicastNetmaskForSave =
    trim(multicastSendNetmask).empty() ? kDefaultMulticastNetmask : trim(multicastSendNetmask);
  const string multicastNetprefixForSave =
    trim(multicastSendNetprefix).empty() ? kDefaultMulticastNetprefix : trim(multicastSendNetprefix);
  const string ttlTrimmed = trim(multicastSendTTL);
  const int multicastTtlForSave = ttlTrimmed.empty()
    ? kDefaultMulticastTtl
    : std::stoi(ttlTrimmed);
  const std::vector<std::string> multicastRecvSubnetsForSave = splitCsv(multicastRecvSubnets);

  multicastSendSet(
    multicastSendSelected,
    multicastNetmaskForSave,
    multicastNetprefixForSave,
    multicastTtlForSave,
    ndiConfig
  );
  multicastRecvSet(multicastRecvSelected, multicastRecvSubnetsForSave, ndiConfig);

  string saveError;
  if (!saveConfigAtomicallyWithBackup(configPath, backupConfigPath, ndiConfig, saveError)) {
    std::cerr << saveError << endl;
    return 1;
  }

  return 0;

}
