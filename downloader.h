#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include <curl/curl.h>
#include <iostream>
#include <chrono>
#include <thread>

#include "metahandler.h"
#include "converter.h"
#include "db.h"

class Downloader
{
 private:
  MetaHandler metaHandler;
  Converter converter;
  Db db;

  std::string downloadThread(const char* hash = NULL, int page = 1);
  bool threadToHTML(rapidjson::Document& doc, const char* fileName);
  void setLastUpdated(const char* directory);
  void setLastPage(const char* directory, int page);
  void archiveThreadNum(int threadnum);
  void archiveThread(int threadnum);
  
 public:
  Downloader();
  ~Downloader();

  void downloadThreads();
  void archiveThreads(int limit);
  void archiveAllThreads();
};

#endif
