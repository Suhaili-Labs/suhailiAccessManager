#include <iostream>
#include <fstream>
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <functional>
#include <sstream>
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

string trim(const string& value) {
  const string whitespace = " \t\n\r";
  const size_t start = value.find_first_not_of(whitespace);
  if (start == string::npos) {
    return "";
  }
  const size_t end = value.find_last_not_of(whitespace);
  return value.substr(start, end - start + 1);
}

vector<string> splitCsv(const string& value) {
  vector<string> tokens;
  std::stringstream ss(value);
  string token;
  while (std::getline(ss, token, ',')) {
    token = trim(token);
    if (!token.empty()) {
      tokens.push_back(token);
    }
  }
  return tokens;
}

bool isValidIPv4(const string& value) {
  std::stringstream ss(value);
  string part;
  int count = 0;
  while (std::getline(ss, part, '.')) {
    if (part.empty() || part.size() > 3) {
      return false;
    }
    for (char c : part) {
      if (!std::isdigit(static_cast<unsigned char>(c))) {
        return false;
      }
    }
    int octet = std::stoi(part);
    if (octet < 0 || octet > 255) {
      return false;
    }
    ++count;
  }
  return count == 4;
}

bool isValidNetmask(const string& value) {
  if (!isValidIPv4(value)) {
    return false;
  }

  std::stringstream ss(value);
  string part;
  uint32_t mask = 0;
  while (std::getline(ss, part, '.')) {
    mask = (mask << 8) | static_cast<uint32_t>(std::stoi(part));
  }

  // Netmask bits must be contiguous ones followed by zeros.
  bool seenZero = false;
  for (int bit = 31; bit >= 0; --bit) {
    const bool set = ((mask >> bit) & 1U) != 0;
    if (!set) {
      seenZero = true;
      continue;
    }
    if (seenZero) {
      return false;
    }
  }
  return true;
}

bool isValidMulticastPrefix(const string& value) {
  if (!isValidIPv4(value)) {
    return false;
  }

  std::stringstream ss(value);
  string first;
  std::getline(ss, first, '.');
  int firstOctet = std::stoi(first);
  return firstOctet >= 224 && firstOctet <= 239;
}

bool validateCsvIPv4(const string& value) {
  for (const string& token : splitCsv(value)) {
    if (!isValidIPv4(token)) {
      return false;
    }
  }
  return true;
}

}  // namespace

int main() {

  const int minTerminalWidth = 80;
  const int minTerminalHeight = 32;
  const string appVersion = "1.0";
  const string defaultMulticastNetmask = "255.255.0.0";
  const string defaultMulticastNetprefix = "239.255.0.0";

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

  validateBeforeSave = [&](string& errorMessage) {
    if (!validateCsvIPv4(discoveryServers)) {
      errorMessage = "Discovery list must contain valid IPv4 values";
      return false;
    }
    if (!validateCsvIPv4(ips)) {
      errorMessage = "IPs list must contain valid IPv4 values";
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
    const bool changedMulticastTtl = multicastSendTTL != initialMulticastSendTTL;
    const bool changedNetmask = multicastSendNetmask != initialMulticastSendNetmask;
    const bool changedNetprefix = multicastSendNetprefix != initialMulticastSendNetprefix;

    const bool invalidDiscovery = validationAttempted && !validateCsvIPv4(discoveryServers);
    const bool invalidIps = validationAttempted && !validateCsvIPv4(ips);
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
          colorizeRow(hbox(text(" Send ") ,separator(),tcpSendToggle->Render()), changedTcpSend, false),
          colorizeRow(hbox(text(" Recv ") ,separator(),tcpRecvToggle->Render()), changedTcpRecv, false)
        ) | center),
        border(vbox(
          text("RUDP") | bold | center,
          separator(), 
          colorizeRow(hbox(text(" Send ") ,separator(),rudpSendToggle->Render()), changedRudpSend, false),
          colorizeRow(hbox(text(" Recv ") ,separator(),rudpRecvToggle->Render()), changedRudpRecv, false)
        ) | center),
        border(vbox(
          text("Unicast") | bold | center, 
          separator(), 
          colorizeRow(hbox(text(" Send ") ,separator(),unicastSendToggle->Render()), changedUnicastSend, false),
          colorizeRow(hbox(text(" Recv ") ,separator(),unicastRecvToggle->Render()), changedUnicastRecv, false)
        ) | center
      ) | center ) | center,

      hbox(
        border(
          vbox(
            text("     Multicast Enables     ") | bold | center,
            separator(),
            hbox(
              vbox(
                colorizeRow(
                  hbox(text(" Send ") | bold | center, separator(),multicastSendToggle->Render()),
                  changedMulticastSend,
                  false
                ),
                colorizeRow(
                  hbox(text(" Recv ") | bold | center, separator(),multicastRecvToggle->Render()),
                  changedMulticastRecv,
                  false
                )
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
                colorizeRow(
                  hbox(text(" TTL ") | bold | center, separator(),multicastSendTTLToggle->Render()),
                  changedMulticastTtl,
                  false
                )
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
      
      separator(),

      text("Navigation: Up/Down/Left/Right move focus | Tab toggles control") | dim | center,

      separator(),

      text("By Suhaili Labs | NDI® is a registered trademark of Vizrt NDI AB") | dim | center,

    });
    return layout;  
  });

  auto app = Modal(renderer, discardConfirmDialog, &showDiscardConfirm);
  app = Modal(app, restoreConfirmDialog, &showRestoreConfirm);

  app = CatchEvent(app, [&](Event event) {
    if (event == Event::ArrowLeft) {
      if (showDiscardConfirm) {
        return discardConfirmContainer->OnEvent(Event::ArrowUp);
      }
      if (showRestoreConfirm) {
        return restoreConfirmContainer->OnEvent(Event::ArrowUp);
      }
      return mainContainer->OnEvent(Event::ArrowUp);
    }

    if (event == Event::ArrowRight) {
      if (showDiscardConfirm) {
        return discardConfirmContainer->OnEvent(Event::ArrowDown);
      }
      if (showRestoreConfirm) {
        return restoreConfirmContainer->OnEvent(Event::ArrowDown);
      }
      return mainContainer->OnEvent(Event::ArrowDown);
    }

    return false;
  });

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
    trim(multicastSendNetmask).empty() ? defaultMulticastNetmask : trim(multicastSendNetmask);
  const string multicastNetprefixForSave =
    trim(multicastSendNetprefix).empty() ? defaultMulticastNetprefix : trim(multicastSendNetprefix);

  multicastSendSet(
    multicastSendSelected,
    multicastNetmaskForSave,
    multicastNetprefixForSave,
    multicastSendTTL,
    ndiConfig
  );
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
