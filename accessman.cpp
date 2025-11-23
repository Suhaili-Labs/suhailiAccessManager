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

void multicastGenConfig(json& ndiConfig) {
    ndiConfig["ndi"]["multicast"] = {
        {"recv", {{"enable", false}}},
        {"send", {
            {"enable", false}, 
            {"netmask", "255.255.0.0"}, 
            {"netprefix", "239.255.0.0"}, 
            {"ttl", 1}
        }}
    };
}

bool configExists(const string& filePath) {
  return std::filesystem::exists(filePath);
}

void machineNameSet(string name, json& ndiConfig) {
  
  ndiConfig["ndi"]["machinename"] = name;

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

  ndiConfig["ndi"]["tcp"]["send"]["enable"] = send;

  if (!ndiConfig["ndi"]["tcp"].contains("recv")) {
    ndiConfig["ndi"]["tcp"]["recv"] = {{"enable", false}};
  }

  ndiConfig["ndi"]["tcp"]["recv"]["enable"] = recv;

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

  ndiConfig["ndi"]["rudp"]["send"]["enable"] = send;

  if (!ndiConfig["ndi"]["rudp"].contains("recv")) {
   ndiConfig["ndi"]["rudp"]["recv"] = {{"enable", false}};
  }

  ndiConfig["ndi"]["rudp"]["recv"]["enable"] = recv;

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

  ndiConfig["ndi"]["unicast"]["send"]["enable"] = send;

  if (!ndiConfig["ndi"]["unicast"].contains("recv")) {
   ndiConfig["ndi"]["unicast"]["recv"] = {{"enable", false}};
  }

  ndiConfig["ndi"]["unicast"]["recv"]["enable"] = recv;

}

void multicastRecvSet(bool recv, json& ndiConfig) {

  if (!ndiConfig["ndi"].contains("multicast")){
    multicastGenConfig(ndiConfig);
  }

  if (!ndiConfig["ndi"]["multicast"].contains("recv")) {
    ndiConfig["ndi"]["multicast"]["recv"] = {{"enable", false}};
  }

  ndiConfig["ndi"]["multicast"]["recv"]["enable"] = recv;

}

void multicastSendSet(bool send, string netmask, string netprefix, int ttl, json& ndiConfig) {
  
  if (!ndiConfig["ndi"].contains("multicast")){
    multicastGenConfig(ndiConfig);
  }
  
  if (!ndiConfig["ndi"]["multicast"].contains("send")) {
    ndiConfig["ndi"]["multicast"]["send"] = {{"enable", false}, {"netmask", "255.255.0.0"}, {"netprefix", "239.255.0.0"}, {"ttl", 1}};
  }

  ndiConfig["ndi"]["multicast"]["send"]["enable"] = send;
  ndiConfig["ndi"]["multicast"]["send"]["netmask"] = netmask;
  ndiConfig["ndi"]["multicast"]["send"]["netprefix"] = netprefix;
  ndiConfig["ndi"]["multicast"]["send"]["ttl"] = ttl;

}

void groupsSet(string sendGroups, string recvGroups, json& ndiConfig) {
  
  if (!ndiConfig["ndi"].contains("groups")) {
    ndiConfig["ndi"]["groups"] = {{"recv", "Public,"}, {"send", "Public,"}};
  }
  
  ndiConfig["ndi"]["groups"]["recv"] = recvGroups;
  ndiConfig["ndi"]["groups"]["send"] = sendGroups;

}

void discoveryServerSet(string discoveryServers, json& ndiConfig) {

  if (!ndiConfig["ndi"].contains("networks")) {
    ndiConfig["ndi"]["networks"] = {{"discovery", ""}, {"ips", ""}};
  }

  ndiConfig["ndi"]["networks"]["discovery"] = discoveryServers;

}

void discoveryIpsSet(string ips, json& ndiConfig) {

  if (!ndiConfig["ndi"].contains("networks")) {
    ndiConfig["ndi"]["networks"] = {{"discovery", ""}, {"ips", ""}};
  }
 
  ndiConfig["ndi"]["networks"]["ips"] = ips;

}

void generateMissingConfig(json& ndiConfig) {
  
  // I'm sure there is a better way to do this, but I'm unsure, will need to come back to this.

  if (!ndiConfig["ndi"].contains("networks")) {
    ndiConfig["ndi"]["networks"] = {{"discovery", ""}, {"ips", ""}};
  }

  if (!ndiConfig["ndi"]["networks"].contains("discovery")) {
    ndiConfig["ndi"]["networks"]["discovery"] = "";
  }

  if (!ndiConfig["ndi"]["networks"].contains("ips")) {
    ndiConfig["ndi"]["networks"]["ips"] = "";
  }

  if (!ndiConfig["ndi"].contains("groups")) {
    ndiConfig["ndi"]["groups"] = {{"recv", "Public,"}, {"send", "Public,"}};
  }

  if (!ndiConfig["ndi"]["groups"].contains("recv")) {
    ndiConfig["ndi"]["groups"]["recv"] = "Public,";
  }

  if (!ndiConfig["ndi"]["groups"].contains("send")) {
    ndiConfig["ndi"]["groups"]["send"] = "Public,";
  }

  if (!ndiConfig["ndi"].contains("machinename")){
    ndiConfig["ndi"]["machinename"] = "";
  }

  if (!ndiConfig["ndi"].contains("multicast")){
    multicastGenConfig(ndiConfig);
  }
  
  if (!ndiConfig["ndi"]["multicast"].contains("send")) {
    ndiConfig["ndi"]["multicast"]["send"] = {{"enable", false}, {"netmask", ""}, {"netprefix", ""}, {"ttl", 1}};
  }

  if (!ndiConfig["ndi"]["multicast"].contains("recv")) {
    ndiConfig["ndi"]["multicast"]["recv"] = {{"enable", false}};
  }

  if (!ndiConfig["ndi"].contains("unicast")) {
    ndiConfig["ndi"]["unicast"] = {
      {"recv", {{"enable", false}}},
      {"send", {{"enable", false}}}
    };
  }

  if (!ndiConfig["ndi"]["unicast"].contains("send")) {
   ndiConfig["ndi"]["unicast"]["send"] = {{"enable", false}};
  }

  if (!ndiConfig["ndi"]["unicast"].contains("recv")) {
   ndiConfig["ndi"]["unicast"]["recv"] = {{"enable", false}};
  }

  if (!ndiConfig["ndi"].contains("rudp")) {
    ndiConfig["ndi"]["rudp"] = {
      {"recv", {{"enable", false}}},
      {"send", {{"enable", false}}}
    };
  }

  if (!ndiConfig["ndi"]["rudp"].contains("send")) {
   ndiConfig["ndi"]["rudp"]["send"] = {{"enable", false}};
  }

  if (!ndiConfig["ndi"]["rudp"].contains("recv")) {
   ndiConfig["ndi"]["rudp"]["recv"] = {{"enable", false}};
  }

  if (!ndiConfig["ndi"].contains("tcp")) {
    ndiConfig["ndi"]["tcp"] = {
      {"recv", {{"enable", false}}},
      {"send", {{"enable", false}}}
    }; 
  }
  
  if (!ndiConfig["ndi"]["tcp"].contains("send")) {
    ndiConfig["ndi"]["tcp"]["send"] = {{"enable", false}};
  }

  if (!ndiConfig["ndi"]["tcp"].contains("recv")) {
    ndiConfig["ndi"]["tcp"]["recv"] = {{"enable", false}};
  }
  
}
