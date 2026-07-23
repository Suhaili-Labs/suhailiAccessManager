#include <iostream>
#include <fstream>
#include <algorithm>
#include <filesystem>
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
  bool saveRequested = false;
  auto closeScreen = screen.ExitLoopClosure();
  auto saveAndExitButton = Button("Save & Exit", [&] {
    saveRequested = true;
    closeScreen();
  });
  auto discardAndExitButton = Button("Discard & Exit", [&] {
    saveRequested = false;
    closeScreen();
  });
  
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
    discardAndExitButton
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

    const bool hasUnsavedChanges =
      tcpSendSelected != initialTcpSendSelected ||
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

    Element changeStatus = hasUnsavedChanges
      ? text("Unsaved changes") | bold | color(Color::Yellow) | center
      : text("All changes saved") | color(Color::Green) | center;

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

      changeStatus,

      hbox(saveAndExitButton->Render(), text("  "), discardAndExitButton->Render()) | center,
      
      separator(),

      text("By Suhaili Labs | NDI® is a registered trademark of Vizrt NDI AB") | dim | center,

    });
    return layout;  
  });

  screen.Loop(renderer);
  
  // End TUI

  if (!saveRequested) {
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

  ofstream outputFile(configPath);
  if (!outputFile.is_open()) {
    std::cerr << "Could not open NDI Config JSON: " << configPath << endl;
    return 1;
  }

  outputFile << ndiConfig.dump(2);
  outputFile.close();

  return 0;

}
