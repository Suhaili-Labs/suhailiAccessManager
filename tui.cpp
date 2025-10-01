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
  int rudpSendSelected = 0;
  int rudpRecvSelected = 0;
  int unicastSendSelected = 0;
  int unicastRecvSelected = 0;
  int multicastSendSelected = 0;
  int multicastRecvSelected = 0;

  Component tcpSendToggle = Toggle(&toggleEntries, &tcpSendSelected);
  Component tcpRecvToggle = Toggle(&toggleEntries, &tcpRecvSelected);
  Component rudpSendToggle = Toggle(&toggleEntries, &rudpSendSelected);
  Component rudpRecvToggle = Toggle(&toggleEntries, &rudpRecvSelected);
  Component unicastSendToggle = Toggle(&toggleEntries, &unicastSendSelected);
  Component unicastRecvToggle = Toggle(&toggleEntries, &unicastRecvSelected);
  Component multicastSendToggle = Toggle(&toggleEntries, &multicastSendSelected);
  Component multicastRecvToggle = Toggle(&toggleEntries, &multicastRecvSelected);

  auto tcpContainer = Container::Vertical({ 
    tcpSendToggle,
    tcpRecvToggle
  });

  auto rudpContainer = Container::Vertical({ 
    rudpSendToggle,
    rudpRecvToggle
  });

  auto unicastContainer = Container::Vertical({
    unicastSendToggle,
    unicastRecvToggle
  });

  auto topRowContainer = Container::Vertical({
    tcpContainer,
    rudpContainer,
    unicastContainer
  });

  auto mainContainer = Container::Vertical({
    topRowContainer,
    exitButton
  });

  
  auto renderer = Renderer(mainContainer, [&] {
    auto layout = vbox({
      text(titleL1) | center,
      text(titleL2) | center,
      text(titleL3) | center,
      text(""),
      text("TUI NDI Access Manager for Linux") | bold | center,
 

      border(text("Send/Recv Modes") | center),

      hbox(
        border(vbox(
          text("TCP") | bold | center,
          separator(),
          hbox(text("  Send:  ") ,separator(),tcpSendToggle->Render()), 
          hbox(text("  Recv:  ") ,separator(),tcpRecvToggle->Render())
        ) | center),
        border(vbox(
          text("RUDP") | bold | center, 
          separator(), 
          hbox(text("  Send:  ") ,separator(),rudpSendToggle->Render()), 
          hbox(text("  Recv:  ") ,separator(),rudpRecvToggle->Render())
        ) | center),
        border(vbox(
          text("Unicast") | bold | center, 
          separator(), 
          hbox(text("  Send:  ") ,separator(),unicastSendToggle->Render()), 
          hbox(text("  Recv:  ") ,separator(),unicastRecvToggle->Render())
        ) | center
      ) | center),

      separator(),

      hbox(exitButton->Render()) | center,
      
      separator(),

      text("By Alexander Ujvarosy | NDI® is a registered trademark of Vizrt NDI AB") | dim | center,

    });
    return layout;  
  });


  screen.Loop(renderer);
  
  // End TUI
  
  tcpSet(tcpSendSelected, tcpRecvSelected, ndiConfig);


  ofstream outputFile(configPath);
  if (!outputFile.is_open()) {
    std::cerr << "Could not open NDI Config JSON: " << configPath << endl;
    return 1;
  }

  outputFile << ndiConfig.dump(2);
  outputFile.close();

  return 0;

}
