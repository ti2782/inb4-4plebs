#ifndef CONVERTER_H
#define CONVERTER_H

#include "rapidjson/filereadstream.h"
#include "rapidjson/document.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <memory>
#include <algorithm>
#include <cstdio>
#include <sstream>

struct ThreadCard
{
  std::string thumb, title, date, comment, threadnum;
};

class Converter
{
 private:

 public:
  Converter();
  ~Converter();

  void addColumn(std::ofstream& f);
  void addRow(std::ofstream& f);
  void addContainer(std::ofstream& f);
  void endDiv(std::ofstream& f);
  void addThreadCard(std::ofstream& f, ThreadCard* card = NULL);
  void sanitizeString(std::string& text);
};

#endif
