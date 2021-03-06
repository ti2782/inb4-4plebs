#include "downloader.h"

using namespace rapidjson;
using namespace std::chrono_literals;

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  ((std::string*)userp)->append((char*)contents, size * nmemb);
  return size * nmemb;
}

static size_t header_callback(char *buffer, size_t size,
                              size_t nitems, void *userdata)
{
  /* received header is nitems * size long in 'buffer' NOT ZERO TERMINATED */
  /* 'userdata' is set with CURLOPT_HEADERDATA */
  if(userdata)
    {
      ((std::string*)buffer)->append((char*)userdata, size * nitems);
      std::cout << ">>INFO\nHEADER: " << buffer << std::endl;
    }
  return nitems * size;
}

Downloader::Downloader()
{
  curl_global_init(CURL_GLOBAL_ALL);
}

Downloader::~Downloader()
{
  curl_global_cleanup();  
}

void Downloader::downloadThreads()
{
  if(!metaHandler.populateMetaMap())
    {
      std::cout << ">>ERROR\nFailed to parse Metas" << std::endl;
      return;
    }
  
  auto metas = metaHandler.getMetas();
  for( const auto& meta : metas)
    {
      const char* hash = meta.second->hash.c_str();
      if(!meta.second->db.empty())
	db.switchDB(meta.second->db.c_str());
      
      int page = meta.second->page;
      int lastpage = 1;
      int iDownloads = 0;
      std::cout << ">>INFO\nDownloading " << meta.first << std::endl;
      while(true)
	{
	  std::string buff = searchThreads(hash, page);
	  rapidjson::Document doc;
	  doc.Parse(buff.c_str());

	  if(!doc.IsObject())
	    {
	      // Try Fallback
	      std::cout << "<<WARNING\nDocument invalid.\n" << buff << "\n" << "Trying Fallback" << std::endl;   
	      std::this_thread::sleep_for(45s);
	      buff = searchThreads(hash, page, true);
	      doc.Parse(buff.c_str());
	    }
	  
	  if(doc.IsObject())
	    {	  
	      if(doc.HasMember("error"))
		{
		  std::cout << ">>WARNING\n" << doc["error"].GetString() << std::endl;
		  std::this_thread::sleep_for(45s);
		  break;		  
		}
	      else
		{
		  std::string fileName = meta.second->directory;
		  fileName.append(std::to_string(page));
		  fileName.append(".html");
		  if(threadToHTML(doc, fileName.c_str()))
		    {
		      lastpage = page;
		      page++;
		      iDownloads++;
		    }
		}
	    }
	  else
	    {
	      std::cout << ">>WARNING\n" << buff << std::endl;
	      std::this_thread::sleep_for(45s);
	      break;
	    }

	  std::this_thread::sleep_for(45s);
	}

      if(iDownloads > 0)
      	{
	  metaHandler.setPage(meta.first.c_str(), lastpage);
	  metaHandler.saveProgress();
	  setLastUpdated(meta.second->directory.c_str());
	  setLastPage(meta.second->directory.c_str(), lastpage);
	}
    }
}

std::string Downloader::searchThreads(const char* hash, int page, bool fallback)
{
  std::string buff;
  
  // Init Curl
  CURL* curl = curl_easy_init();
  
  if(!curl || !hash)
    return buff;

  std::string url;
  if(!fallback)
    url = SEARCH_URL;
  else
    url = SEARCH_URL_FALLBACK;
  url.append("&image=");
  url.append(hash);
  url.append("&page=");
  url.append(std::to_string(page));
  
  CURLcode res;
  
  curl_easy_setopt(curl, CURLOPT_URL, url.c_str()); // URL
  curl_easy_setopt(curl, CURLOPT_USERAGENT, "inb4sauce.net bot. Contact @Inb4Sauce");
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
  curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buff);

  std::cout << ">>INFO\nDownloading Page: " << page << std::endl;
  
  res = curl_easy_perform(curl);
  if(res != CURLE_OK)
    {
      std::cout << ">>WARNING\n" << curl_easy_strerror(res) << std::endl;
      if( res == 429 )
	std::cout << ">>WARNING\nTIMEOUT" << std::endl;
      if(!fallback)
	{
	  std::cout << "Trying Fallback" << std::endl;
	  curl_easy_cleanup(curl);
	  return searchThreads(hash, page, true);
	}
    }
  
  curl_easy_cleanup(curl);
  return buff;
}

bool Downloader::threadToHTML(rapidjson::Document& doc, const char* fileName)
{
  if(!fileName)
    {
      std::cout << "<<WARNING\nNo Filename specified." << std::endl;
      return false;
    }
  
  if(!doc.HasMember("0"))
    {
      std::cout << "<<WARNING\nDocument invalid. Missing Results" << std::endl;
      return false;
    }

  if(!doc["0"].HasMember("posts"))
    {
      std::cout << "<<WARNING\nDocument invalid. Missing posts" << std::endl;
      return false;
    }
  
  rapidjson::Value& posts = doc["0"]["posts"];

  std::ofstream outfile(fileName, std::ofstream::binary);
  if(!outfile.is_open())
    {
      std::cout << ">>WARNING\nFailed to open file " << fileName << std::endl;
      return false;
    }

  converter.addContainer(outfile);
  converter.addRow(outfile);
  
  int sz = posts.Size();
  int rowCount = 0;  

  std::vector<std::unique_ptr<ThreadCard>> vcards;
  for(rapidjson::SizeType i = 0; i < sz; i++)
    {
      std::unique_ptr<ThreadCard> card(new ThreadCard);
      
      if(posts[i].HasMember("thread_num"))
	card->threadnum = posts[i]["thread_num"].GetString();
      if(posts[i].HasMember("fourchan_date"))
	card->date = posts[i]["fourchan_date"].GetString();
      if(!posts[i]["title"].IsNull())
	card->title = posts[i]["title"].GetString();
      if(!posts[i]["media"]["thumb_link"].IsNull())
	card->thumb = posts[i]["media"]["thumb_link"].GetString();
      if(!posts[i]["comment"].IsNull())
	card->comment = posts[i]["comment"].GetString();
      
      // ARCHIVE THREAD NUM FOR FULL TEXT SEARCH INTERFACE
      archiveThreadNum(std::atoi(card->threadnum.c_str()));

      vcards.push_back(std::move(card));      
    }
  
  // Reverse Order
  std::reverse(std::begin(vcards), std::end(vcards));
  
  // Write to File
  for(int i = 0; i < vcards.size(); i++)
    {
      if(i % 4 == 0 && i != 0)
       	{
       	  converter.endDiv(outfile); // Row
	  outfile << "<br><br>" << std::endl;
       	  converter.addRow(outfile);
	  rowCount+=1;
       	}
      
      converter.addColumn(outfile);
      
      converter.addThreadCard(outfile, vcards[i].get());
      
      converter.endDiv(outfile); // Column
    }
  
  if(rowCount <= (sz / 4))
    converter.endDiv(outfile); // Row
  
  converter.endDiv(outfile); // Container
  
  return true;
}

void Downloader::archiveThreadNum(int threadnum)
{
  db.addThread(threadnum);
}

void Downloader::archiveThread(int threadnum, bool fallback)
{
  std::string buff;
  std::string url;
  
  // Init Curl
  CURL* curl = curl_easy_init();
  
  if(!curl)
    return;

  if(fallback)
    url.append(THREAD_URL_FALLBACK);
  else
    url.append(THREAD_URL);
  
  url.append(std::to_string(threadnum));
  
  CURLcode res;

  curl_easy_setopt(curl, CURLOPT_URL, url.c_str()); // URL
  curl_easy_setopt(curl, CURLOPT_USERAGENT, "inb4sauce.net bot. Contact @Inb4Sauce");
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
  curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buff);

  std::cout << ">>INFO\nDownloading Thread: " << threadnum << std::endl;
  
  res = curl_easy_perform(curl);
  if(res != CURLE_OK)
    {
      std::cout << curl_easy_strerror(res) << std::endl;
      curl_easy_cleanup(curl);
      std::cout << ">>WARNING\nFailed to Archive Thread " << threadnum << "\n" << buff << std::endl;
      if(!fallback)
	{
	  std::cout << ">>INFO\nTrying Fallback URL" << std::endl;
	  archiveThread(threadnum, true);
	}
      return;
    }
  else
    {
      curl_easy_cleanup(curl);
      
      
      rapidjson::Document doc;
      doc.Parse(buff.c_str());      
      if(doc.IsObject())
	{	  
	  if(doc.HasMember("error"))
	    {
	      std::cout << ">>WARNING\n" << doc["error"].GetString() << std::endl;
	      if(!fallback)
		{
		  std::cout << ">>INFO\nTrying Fallback URL" << std::endl;
		  archiveThread(threadnum, true);
		}
	      return;
	    }
	  const char* key = std::to_string(threadnum).c_str();

	  if(!doc.HasMember(key))
	    {
	      std::cout << ">>WARNING\nNo thread in document" << std::endl;
	      return;
	    }

	  // Get Title & Thumbnail
	  if(!doc[key].HasMember("op"))
	    {
	      std::cout << ">>WARNING\nNo OP in document" << std::endl;
	      return;
	    }
	  rapidjson::Value& op = doc[key]["op"];
	  std::string title, thumbnail, opText;
	  int opTimestamp, opNum;

	  // Append Title to text for archive
	  if(!op["title"].IsNull())
	    {
	      title = op["title"].GetString();
	      opText = title;
	    }

	  if(!op["comment"].IsNull())
	    opText.append(op["comment"].GetString());

	  if(!op["num"].IsNull())
	    opNum = std::atoi(op["num"].GetString());

	  if(!op["media"]["thumb_link"].IsNull())
	    thumbnail = op["media"]["thumb_link"].GetString();

	  if(!op["timestamp"].IsNull())
	    opTimestamp = op["timestamp"].GetInt();

	  db.addPost(opNum, opNum, opText.c_str(), opTimestamp, title.c_str(), thumbnail.c_str());
	  
	  if(!doc[key].HasMember("posts"))
	    {
	      std::cout << ">>WARNING\nNo posts in document" << std::endl;
	      return;
	    }
	  
	  rapidjson::Value& posts = doc[key]["posts"];
	  for(rapidjson::Value::ConstMemberIterator it = posts.MemberBegin(); it != posts.MemberEnd(); it++)
	    {
	      int num, timestamp;
	      std::string text;
	      if(!it->value["comment"].IsNull())
		text.append(it->value["comment"].GetString());
	      if(!it->value["num"].IsNull())
		num = std::atoi(it->value["num"].GetString());
	      if(!it->value["timestamp"].IsNull())
		timestamp = it->value["timestamp"].GetInt();

	      db.addPost(threadnum, num, text.c_str(), timestamp, title.c_str(), thumbnail.c_str());
	    }
	  
	}
      else
	{
	  std::cout << ">>WARNING\nFailed to Archive Thread " << threadnum << "\n" << buff << std::endl;
	  if(!fallback)
	    {
	      std::cout << ">>INFO\nTrying Fallback URL" << std::endl;
	      archiveThread(threadnum, true);
	    }
	}      
    }
}

void Downloader::setLastUpdated(const char* directory)
{
  if(directory)
    {
      std::string fileName(directory);
      fileName.append(LASTUPDATED_FILE);
      std::ofstream outfile(fileName, std::ofstream::trunc);
      if(outfile.is_open())
	{
	  std::time_t now = std::time(nullptr);
	  outfile << std::asctime(std::localtime(&now));
	  outfile.close();
	}
      else
	std::cout << ">>WARNING\nFailed to open " << fileName << std::endl;      
    }  
}

void Downloader::setLastPage(const char* directory, int page)
{
   if(directory)
    {
      std::string fileName(directory);
      fileName.append(LASTPAGE_FILE);
      std::ofstream outfile(fileName, std::ofstream::trunc);
      if(outfile.is_open())
	{
	  outfile << page;
	  outfile.close();
	}
      else
	std::cout << ">>WARNING\nFailed to open " << fileName << std::endl;      
    }   
}

void Downloader::archiveThreads(int limit)
{
  metaHandler.populateMetaMap();
  auto dbs = metaHandler.getDBs();
  for(auto& database : dbs)
    {
      db.switchDB(database.c_str());
      auto threadnums = db.getThreads(limit);
      std::cout << ">>INFO\n" << "Downloading " << threadnums.size() << " threads from " << database << " database" << std::endl;
      for(int i = 0; i < threadnums.size(); i++)
	{
	  archiveThread(threadnums[i]);
	  std::this_thread::sleep_for(std::chrono::seconds(15));
	}
    }
}

void Downloader::archiveAllThreads()
{
  metaHandler.populateMetaMap();
  auto dbs = metaHandler.getDBs();
  for(auto& database : dbs)
    {
      db.switchDB(database.c_str());
      auto threadnums = db.getAllThreads();
      std::cout << ">>INFO\n" << "Downloading " << threadnums.size() << " threads from " << database << " database" << std::endl;
      for(int i = 0; i < threadnums.size(); i++)
	{
	  archiveThread(threadnums[i]);
	  std::this_thread::sleep_for(std::chrono::seconds(15));
	}
    }
}
