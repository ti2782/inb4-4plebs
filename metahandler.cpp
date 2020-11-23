#include "metahandler.h"

using namespace rapidjson;

MetaHandler::MetaHandler()
{

}

MetaHandler::~MetaHandler()
{
  // FREE METAS
  for(const auto& meta : metaMap)
    if(meta.second)
      delete meta.second;
}

bool MetaHandler::populateMetaMap()
{
  std::string configFileName(CONFIG_DIR);
  configFileName.append(CONFIG_FILE);

  // FREE METAS
  for(const auto& meta : metaMap)
    if(meta.second)
      delete meta.second;
  metaMap.clear();
  
  
  // OPEN FILE
  FILE* configFile = fopen(configFileName.c_str(), "r");
  if(!configFile)
    {
      std::cout << ">>WARNING\nFailed to open config file at " << configFileName << std::endl;
      return false;
    }

  char buff[2048];
  FileReadStream is(configFile, buff, sizeof(buff));

  // PARSE JSON DOCUMENT
  doc.ParseStream<kParseCommentsFlag>(is);
  fclose(configFile);

  if(!doc.IsObject())
    {
      std::cout << ">>WARNING\nFailed to parse config file. No Object" << std::endl;
      return false;
    }

  if(!doc.HasMember("metas"))
    {
      std::cout << ">>WARNING\nFailed to parse config file" << std::endl;
      return false;
    }
  
  const Value& metaJson = doc["metas"];
  for( SizeType i = 0; i < metaJson.Size(); i++)    
  {
    Meta* meta = new Meta;
    if(metaJson[i].HasMember("name"))
      meta->name = metaJson[i]["name"].GetString();
    if(metaJson[i].HasMember("hash"))
      meta->hash = urlFriendly(metaJson[i]["hash"].GetString());
    if(metaJson[i].HasMember("directory"))
      meta->directory = metaJson[i]["directory"].GetString();
    if(metaJson[i].HasMember("page"))
      meta->page = metaJson[i]["page"].GetInt();
    if(metaJson[i].HasMember("db"))
      meta->db = metaJson[i]["db"].GetString();

    if(!meta->name.empty() || !meta->hash.empty() || !meta->directory.empty())
      metaMap.insert({meta->name, meta});

    if(!meta->name.empty() || !meta->db.empty())
      dbMap.insert({meta->name, meta->db});
  }
  return true;
}

void MetaHandler::setPage(const char* meta, int page)
{
  auto search = metaMap.find(meta);
  if(search != metaMap.end())
    {
      if(search->second)
	search->second->page = page;
    }
  else
    std::cout << ">>WARNING\nFailed to set page for " << meta << std::endl;
}

void MetaHandler::saveProgress()
{
  if(!doc.HasMember("metas"))
    {
      std::cout << ">>WARNING\nNo Metas loaded" << std::endl;
      return;
    }
  
  Value& metaJson = doc["metas"];
  for( SizeType i = 0; i < metaJson.Size(); i++)    
    {
      std::string name;
      int page;
      if(metaJson[i].HasMember("name") && metaJson[i].HasMember("page"))
	name = metaJson[i]["name"].GetString();

      auto search = metaMap.find(name); // Cross check against loaded metas
      if(search != metaMap.end())
	{
	  if(search->second)
	    {
	      metaJson[i]["page"].SetInt(search->second->page);
	    }
	}
      else
	std::cout << "WARNING\nMeta " << name << " not loaded" << std::endl;
    }
  
  std::string configFileName(CONFIG_DIR);
  configFileName.append(CONFIG_FILE);
  FILE* fp = fopen(configFileName.c_str(), "w");
  
  char buff[2048];
  rapidjson::FileWriteStream os(fp, buff, sizeof(buff));
  
  rapidjson::Writer<rapidjson::FileWriteStream> writer(os);
  doc.Accept(writer);
  
  fclose(fp);	      
}

const char* MetaHandler::urlFriendly(std::string hash)
{
  // REPLACE + with -
  for(size_t pos = hash.find("+"); pos != std::string::npos; pos = hash.find("+"))
    hash.replace(pos, 1, "-");

  // REPLACE / with _
  for(size_t pos = hash.find("/"); pos != std::string::npos; pos = hash.find("/"))
    hash.replace(pos, 1, "_");

  return hash.c_str();
}
