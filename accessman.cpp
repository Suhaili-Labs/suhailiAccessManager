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

void machineName(string name, json& ndiConfig) {
  
  ndiConfig["ndi"]["machineName"] = name;

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

void rudpSet(bool send, bool recv, json& ndiConfig) {

  if (!ndiConfig["ndi"].contains("rudp")) {
    ndiConfig["ndi"]["rudp"] = {
      {"recv", {{"enable", false}}},
      {"send", {{"enable", false}}}
    };
  }

  if (!ndiConfig["ndi"]["rudp"].contains("send")) {
   ndiConfig["ndi"]["rudp"]["send"] = {{"enable", false}};
  }

  if (send) {
    ndiConfig["ndi"]["rudp"]["send"]["enable"] = true;
  } else { 
    ndiConfig["ndi"]["rudp"]["send"]["enable"] = false;
  }

  if (!ndiConfig["ndi"]["rudp"].contains("recv")) {
   ndiConfig["ndi"]["rudp"]["recv"] = {{"enable", false}};
  }

  if (recv) {
    ndiConfig["ndi"]["rudp"]["recv"]["enable"] = true;
  } else { 
    ndiConfig["ndi"]["rudp"]["recv"]["enable"] = false;
  }

}

void unicastSet(bool send, bool recv, json& ndiConfig) {

  if (!ndiConfig["ndi"].contains("unicast")) {
    ndiConfig["ndi"]["unicast"] = {
      {"recv", {{"enable", false}}},
      {"send", {{"enable", false}}}
    };
  }

  if (!ndiConfig["ndi"]["unicast"].contains("send")) {
   ndiConfig["ndi"]["unicast"]["send"] = {{"enable", false}};
  }

  if (send) {
    ndiConfig["ndi"]["unicast"]["send"]["enable"] = true;
  } else { 
    ndiConfig["ndi"]["unicast"]["send"]["enable"] = false;
  }

  if (!ndiConfig["ndi"]["unicast"].contains("recv")) {
   ndiConfig["ndi"]["unicast"]["recv"] = {{"enable", false}};
  }

  if (recv) {
    ndiConfig["ndi"]["unicast"]["recv"]["enable"] = true;
  } else { 
    ndiConfig["ndi"]["unicast"]["recv"]["enable"] = false;
  }

}

void multicastRecvSet(bool recv, string subnets, json& ndiConfig) {

  if (!ndiConfig["ndi"].contains("multicast")) {
    ndiConfig["ndi"]["multicast"] = {
      {"recv", {{"enable", false}, {"subnets", ""}}},
      {"send", {{"enable", false}, {"netmask", ""}, {"netprefix", ""}, {"ttl", 0}}}
    };
  }

}
