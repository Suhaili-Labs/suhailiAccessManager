
#ifndef ACCESSMAN_H
#define ACCESSMAN_H

#include <string>
#include "json.hpp"

using std::string;
using nlohmann::json;


string getHomeDir();
bool configExists(const string& filePath);
void machineNameSet(string name, json& ndiConfig);
void tcpSet(bool send, bool recv, json& ndiConfig);
void rudpSet(bool send, bool recv, json& ndiConfig);
void unicastSet(bool send, bool recv, json& ndiConfig);
void multicastRecvSet(bool recv, string subnets, json& ndiConfig);
void multicastSendSet(bool send, string netmask, string netprefix, int ttl, json& ndiConfig);
void groupsSet(string sendGroups, string recvGroup, json& ndiConfig);
void discoveryServerSet(string discoveryServers, json& ndiConfig);
void discoveryIpsSet(string ips, json& ndiConfig);
void generateMissingConfig(json& ndiConfig);

#endif
