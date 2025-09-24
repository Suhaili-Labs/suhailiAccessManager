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

  const string configPath = getHomeDir() + "/.ndi/ndi-config.v1.json";

  cout << "NDI Config Dir: ";
  cout << configPath << endl;
  
  ifstream inputFile(configPath);
  if (!inputFile.is_open()) {
    std::cerr << "Could not open NDI Config JSON: " << configPath << endl;
    return 1;
  }

  inputFile >> ndiConfig;
  inputFile.close();

  cout << "===========NDI CONFIG JSON===========" << endl;
  cout << ndiConfig.dump(2) << endl; 
  cout << "===========NDI CONFIG JSON===========" << endl;

  tcpSet(true, true, ndiConfig);
  rudpSet(true, true, ndiConfig);
  unicastSet(true, true, ndiConfig);
  machineName("ALEX MAC", ndiConfig);
  multicastRecvSet(false, "", ndiConfig);

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
