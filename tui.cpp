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


void testFunc() {
    cout << "Button was pressed!" << endl;
}

int main(){

  json ndiConfig; 
  
// Make Functionm?
  const std::filesystem::path ndiDir = std::filesystem::path(getHomeDir()) / ".ndi";
  
  if (!std::filesystem::exists(ndiDir)) {
    std::filesystem::create_directory(ndiDir);
    cout << ".ndi Direcory does not exist. Creating." << endl;
  }
// Make Function 
  const std::filesystem::path configPath = ndiDir / "ndi-config.v1.json";

  cout << "NDI Config Dir: " << configPath << endl;
  
  
  ifstream inputFile(configPath);
  if (!inputFile.is_open()) {
    std::cerr << "Could not open NDI Config JSON: " << configPath << endl;
    cout << "New ndi-config.v1.json will be created." << endl;
  } else {
    inputFile >> ndiConfig;
    inputFile.close();
  }

  cout << "===========NDI CONFIG JSON===========" << endl;
  cout << ndiConfig.dump(2) << endl; 
  cout << "===========NDI CONFIG JSON===========" << endl;
  


// TUI BELOW

  auto screen = ScreenInteractive::Fullscreen();

  auto exitButton = Button("Exit", screen.ExitLoopClosure());
  
  vector<string> toggleEntries = {
    "  Disable  ",
    "  Enable  ",
  };

  int tcpSendSelected = 0;
  int tcpRecvSelected = 0;
  int rudpSendSelected = 0;
  int rudpRecvSelected = 0;

  Component tcpSendToggle = Toggle(&toggleEntries, &tcpSendSelected);
  Component tcpRecvToggle = Toggle(&toggleEntries, &tcpRecvSelected);
  Component rudpSendToggle = Toggle(&toggleEntries, &rudpSendSelected);
  Component rudpRecvToggle = Toggle(&toggleEntries, &rudpRecvSelected);

  auto tcpContainer = Container::Vertical({ 
    tcpSendToggle,
    tcpRecvToggle,
  });

  auto rudpContainer = Container::Vertical({ 
    rudpSendToggle,
    rudpRecvToggle,
  });

  auto topRowContainer = Container::Horizontal({
    tcpContainer,
    rudpContainer,
  });

  auto mainContainer = Container::Vertical({
    topRowContainer,
    exitButton,
  });

  auto renderer = Renderer(mainContainer, [&] {
    auto layout = vbox({
      text("Suhaili NDI Access Manager for Linux") | bold | center,
      
      separator(),

      hbox(
        separator(),
        vbox(
          text("TCP") | bold | center, 
          separator(), 
          hbox(text("  Send:  ") ,separator(),tcpSendToggle->Render()), 
          hbox(text("  Recv:  ") ,separator(),tcpRecvToggle->Render())
        ) | center,
        separator(),
        vbox(
          text("RUDP") | bold | center, 
          separator(), 
          hbox(text("  Send:  ") ,separator(),rudpSendToggle->Render()), 
          hbox(text("  Recv:  ") ,separator(),rudpRecvToggle->Render())
        ) | center
      ) | center,

      separator(),

      hbox(exitButton->Render()) | center,
      
      separator(),

      text("By Suhaili AV | NDI® is a registered trademark of Vizrt NDI AB") | dim | center,

    });
    return layout;  
  });


  screen.Loop(renderer);
  
  // End TUI

  ofstream outputFile(configPath);
  if (!outputFile.is_open()) {
    std::cerr << "Could not open NDI Config JSON: " << configPath << endl;
    return 1;
  }

  outputFile << ndiConfig.dump(2);
  outputFile.close();

  return 0;

}
