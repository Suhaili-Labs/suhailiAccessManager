#include <iostream>
#include <fstream>
#include <filesystem>
#include "json.hpp"


using std::cout;
using nlohmann::json;
using std::string;
using std::endl;
using std::ifstream;
using std::ofstream;


string getHomeDir(){
  const char* home = std::getenv("HOME");
  return home ? string(home) : string("");
}

bool configExists(const string& filePath) {
  return std::filesystem::exists(filePath);
}

void tcpSet(bool send, bool recv, json& ndiConfig) {

  if (!ndiConfig["ndi"].contains("tcp")) {
    ndiConfig["ndi"]["tcp"] = {
      {"recv", {{"enable", false}}},
      {"send", {{"enable", false}}}
    }; 
  }
  
  if (!ndiConfig["ndi"]["tcp"].contains("send")) {
    ndiConfig["ndi"]["tcp"]["send"] = {{"enable", false}};
  }

  if (send){
    ndiConfig["ndi"]["tcp"]["send"]["enable"] = true;
  } else {
    ndiConfig["ndi"]["tcp"]["send"]["enable"] = false;
  }

  if (!ndiConfig["ndi"]["tcp"].contains("recv")) {
    ndiConfig["ndi"]["tcp"]["recv"] = {{"enable", false}};
  }

  if (recv){
    ndiConfig["ndi"]["tcp"]["recv"]["enable"] = true;
  } else {
    ndiConfig["ndi"]["tcp"]["recv"]["enable"] = false;
  }

}

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

  tcpSet(true, false, ndiConfig);

  cout << "===========NDI CONFIG JSON===========" << endl;
  cout << ndiConfig.dump(2) << endl; 
  cout << "===========NDI CONFIG JSON===========" << endl;
  
  ofstream outputFile(configPath);
  if (!outputFile.is_open()) {
    std::cerr << "Could not open NDI Config JSON: " << configPath << endl;
    return 1;
  }

  outputFile << ndiConfig.dump(2);

  return 0;

}
