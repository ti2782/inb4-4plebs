#ifndef METAHANDLER_H
#define METAHANDLER_H

#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/writer.h"

#include <iostream>
#include <unordered_map>
#include "config.h"

struct Meta
{
  std::string name, hash, directory, db;
  int page;
};

class MetaHandler
{
 private:
  std::unordered_map<std::string, Meta*> metaMap;
  std::unordered_map<std::string, std::string> dbMap;
  rapidjson::Document doc;
  
 public:
  MetaHandler();
  ~MetaHandler();

  bool populateMetaMap();
  const char* urlFriendly(std::string hash);
  inline auto getMetas() {return metaMap;};
  inline auto getDBs() {return dbMap;};
  void setPage(const char* meta, int page);
  void saveProgress();
};

#endif
