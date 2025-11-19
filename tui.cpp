#include <iostream>
#include <fstream>
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


int main(){

  json ndiConfig; 
  
// Make Function????
  const std::filesystem::path ndiDir = std::filesystem::path(getHomeDir()) / ".ndi";
  
  if (!std::filesystem::exists(ndiDir)) {
    std::filesystem::create_directory(ndiDir);
    cout << ".ndi Direcory does not exist. Creating." << endl;
  }
// Make Function????
  const std::filesystem::path configPath = ndiDir / "ndi-config.v1.json";

  cout << "NDI Config Dir: " << configPath << endl;
  
// Need to add a function to generate missing settings if they do not exist
// in the ndi-config, or program will crash if they are not present in the config.

  ifstream inputFile(configPath);
  if (!inputFile.is_open()) {
    std::cerr << "Could not open NDI Config JSON: " << configPath << endl;
    cout << "New ndi-config.v1.json will be created." << endl;
  } else {
    inputFile >> ndiConfig;
    inputFile.close();
  }

 // cout << "===========NDI CONFIG JSON===========" << endl;
 // cout << ndiConfig.dump(2) << endl; 
 // cout << "===========NDI CONFIG JSON===========" << endl;
  

  generateMissingConfig(ndiConfig);

// TUI BELOW

  string titleL1 = "░█▀█░█▀▀░█▀▀░█▀▀░█▀▀░█▀▀░░░█▄█░█▀█░█▀█░█▀█░█▀▀░█▀▀░█▀▄";
  string titleL2 = "░█▀█░█░░░█░░░█▀▀░▀▀█░▀▀█░░░█░█░█▀█░█░█░█▀█░█░█░█▀▀░█▀▄";
  string titleL3 = "░▀░▀░▀▀▀░▀▀▀░▀▀▀░▀▀▀░▀▀▀░░░▀░▀░▀░▀░▀░▀░▀░▀░▀▀▀░▀▀▀░▀░▀";

  auto screen = ScreenInteractive::Fullscreen();
  auto exitButton = Button("Exit", screen.ExitLoopClosure());
  
  vector<string> toggleEntries = {
    "  Disable  ",
    "  Enable  "
  };

  int tcpSendSelected = ndiConfig["ndi"]["tcp"]["send"]["enable"];
  int tcpRecvSelected = ndiConfig["ndi"]["tcp"]["recv"]["enable"];
  int rudpSendSelected = ndiConfig["ndi"]["rudp"]["send"]["enable"];
  int rudpRecvSelected = ndiConfig["ndi"]["rudp"]["recv"]["enable"];
  int unicastSendSelected = ndiConfig["ndi"]["unicast"]["send"]["enable"];
  int unicastRecvSelected = ndiConfig["ndi"]["unicast"]["recv"]["enable"];
  int multicastSendSelected = 0;
  int multicastRecvSelected = 0;
  
  string sendGroups = ndiConfig["ndi"]["groups"]["send"];
  string recvGroups = ndiConfig["ndi"]["groups"]["recv"];
  string discoveryServers = ndiConfig["ndi"]["networks"]["discovery"];
  string ips = ndiConfig["ndi"]["networks"]["ips"];
  string machineName = ndiConfig["ndi"]["machinename"];  
  string multicastSubnets;


  Component tcpSendToggle = Toggle(&toggleEntries, &tcpSendSelected);
  Component tcpRecvToggle = Toggle(&toggleEntries, &tcpRecvSelected);
  Component rudpSendToggle = Toggle(&toggleEntries, &rudpSendSelected);
  Component rudpRecvToggle = Toggle(&toggleEntries, &rudpRecvSelected);
  Component unicastSendToggle = Toggle(&toggleEntries, &unicastSendSelected);
  Component unicastRecvToggle = Toggle(&toggleEntries, &unicastRecvSelected);
  Component multicastSendToggle = Toggle(&toggleEntries, &multicastSendSelected);
  Component multicastRecvToggle = Toggle(&toggleEntries, &multicastRecvSelected);
  Component sendGroupInput = Input(&sendGroups, "Public, Group1, Group2");
  Component recvGroupInput = Input(&recvGroups, "Public, Group1, Group2");
  Component discoveryServersInput = Input(&discoveryServers, "192.168.1.21,192.168.1.22");
  Component ipsInput = Input(&ips, "192.168.1.1,192.168.1.2");
  Component machineNameInput = Input(&machineName, "My Machine Name");

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

  Component mainContainer = Container::Vertical({ 
    machineNameInput,
    discoveryServersInput,
    ipsInput,
    sendGroupInput,
    recvGroupInput,
    modesRowContainer,
    exitButton
  });

  
  auto renderer = Renderer(mainContainer, [&] {
    auto layout = vbox({
      text(titleL1) | center,
      text(titleL2) | center,
      text(titleL3) | center,
      text(""),
      text("TUI NDI Access Manager for Linux") | bold | center,
  
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
          text("TCP") | bold | center,
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
      ) | center) | center,

      separator(),

      hbox(exitButton->Render()) | center,
      
      separator(),

      text("By Alexander Ujvarosy | NDI® is a registered trademark of Vizrt NDI AB") | dim | center,

    });
    return layout;  
  });


  screen.Loop(renderer);
  
  // End TUI

  discoveryServerSet(discoveryServers, ndiConfig);
  discoveryIpsSet(ips, ndiConfig);
  groupsSet(sendGroups, recvGroups, ndiConfig);
  tcpSet(tcpSendSelected, tcpRecvSelected, ndiConfig);
  rudpSet(rudpSendSelected, rudpRecvSelected, ndiConfig);
  unicastSet(unicastSendSelected,unicastRecvSelected, ndiConfig);
  machineNameSet(machineName, ndiConfig);


  ofstream outputFile(configPath);
  if (!outputFile.is_open()) {
    std::cerr << "Could not open NDI Config JSON: " << configPath << endl;
    return 1;
  }

  outputFile << ndiConfig.dump(2);
  outputFile.close();

  return 0;

}
