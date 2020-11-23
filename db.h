#ifndef DB_H
#define DB_H

#include "rapidjson/filereadstream.h"
#include "rapidjson/document.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <memory>
#include <algorithm>
#include <cstdio>
#include <chrono>

#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>
#include <bsoncxx/string/to_string.hpp>

#include <mongocxx/client.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/instance.hpp>

#define SQL_URL "localhost"
#define SQL_USER "inb4"
#define SQL_PW ""
#define SQL_DATABASE "inb4"

class Db
{
 private:
  mongocxx::instance instance{}; // This should be done only once.
  std::string dbName;
 public:
  Db();
  ~Db();

  void addThread(int thread);
  void addPost(int thread, int post, const char* text, int timestamp, const char* title = NULL, const char* thumbnail = NULL);
  std::vector<int> getThreads(int limit = 10);
  std::vector<int> getAllThreads();

  void switchDB(const char* db) { if(db) dbName.clear(); dbName = db; } ;
};

#endif
