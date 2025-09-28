#include <iostream>
#include <fstream>
#include "json.hpp"
#include "accessman.h"

using std::cout;
using nlohmann::json;
using std::string;
using std::endl;
using std::ifstream;
using std::ofstream;


int main(){

  json ndiConfig; 
  
// Make Functionm?
  const std::filesystem::path ndiDir = std::filesystem::path(getHomeDir()) / ".ndi";
  
  if (!std::filesystem::exists(ndiDir)) {
    std::filesystem::create_directory(ndiDir);
    cout << ".ndi Direcory does not exist. Creating." << endl;
  }
// Make Function 
  const string configPath = getHomeDir() + "/.ndi/ndi-config.v1.json";

  cout << "NDI Config Dir: " << configPath << endl;
  
  // TEST CODE BELOW
  
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

  tcpSet(true, true, ndiConfig);
  rudpSet(true, true, ndiConfig);
  unicastSet(true, true, ndiConfig);
  machineName("ALEX MAC", ndiConfig);
  multicastRecvSet(false, "", ndiConfig);
  multicastSendSet(true, "255.255.255.0", "255.0.0.", 3, ndiConfig);
  groupsSet("Public, Alex", "Public, Test", ndiConfig);
  discoveryServerSet("10.159.0.21,10.4.5.3", ndiConfig);
  discoveryIpsSet("10.11.12.13", ndiConfig);

  cout << "===========NDI CONFIG JSON===========" << endl;
  cout << ndiConfig.dump(2) << endl; 
  cout << "===========NDI CONFIG JSON===========" << endl;
  
  ofstream outputFile(configPath);
  if (!outputFile.is_open()) {
    std::cerr << "Could not open NDI Config JSON: " << configPath << endl;
    return 1;
  }

  outputFile << ndiConfig.dump(2);
  outputFile.close();

  return 0;

}
