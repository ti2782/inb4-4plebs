#include "db.h"

using namespace rapidjson;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::open_document;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_document;

Db::Db()
{
  
}

Db::~Db()
{
  
}

void Db::addThread(int thread)
{
  mongocxx::client client{mongocxx::uri{}};
  mongocxx::database db = client["inb4"];
  mongocxx::collection coll = db["threads"];

  bsoncxx::stdx::optional<bsoncxx::document::value> threadret = coll.find_one(document{} << "thread" << thread << finalize);
  if(threadret) {
    return;
  }

  auto timestamp = bsoncxx::types::b_date{std::chrono::system_clock::now()};
  
  bsoncxx::document::value document = bsoncxx::builder::basic::make_document(kvp("thread", thread), kvp("title", ""), kvp("created_at", timestamp));
  coll.insert_one(document.view()); 
}

std::vector<int> Db::getThreads(int limit)
{
  std::vector<int> ret;
  mongocxx::client client{mongocxx::uri{}};
  mongocxx::database db = client["inb4"];
  mongocxx::collection coll = db["threads"];
  auto order = document{} << "created_at" << -1 << finalize;
  auto opts = mongocxx::options::find{};
  opts.sort(order.view());
  opts.limit(limit);
  
  mongocxx::cursor cursor = coll.find({}, opts);  
  
  for(auto&& doc : cursor)
    {       
      auto store = doc["thread"];      
      if(store)
	ret.push_back(store.get_int32());
    }
  return ret;
}

std::vector<int> Db::getAllThreads()
{
  std::vector<int> ret;
  mongocxx::client client{mongocxx::uri{}};
  mongocxx::database db = client["inb4"];
  mongocxx::collection coll = db["threads"];
  
  mongocxx::cursor cursor = coll.find({});  
  
  for(auto&& doc : cursor)
    {       
      auto store = doc["thread"];      
      if(store)
	ret.push_back(store.get_int32());
    }
  return ret;
}

void Db::addPost(int thread, int post, const char* text, int timestamp, const char* title, const char* thumbnail)
{
  mongocxx::client client{mongocxx::uri{}};
  mongocxx::database db = client["inb4"];
  mongocxx::collection coll = db["posts"];
  mongocxx::collection threadcoll = db["threads"];
  bsoncxx::builder::stream::document document{};
  
  bsoncxx::stdx::optional<bsoncxx::document::value> ret = coll.find_one(bsoncxx::builder::stream::document{} << "post" << post << finalize);
  if(ret) {
    return;
  }
  
  document << "thread" << thread
	   << "post" << post
	   << "txt" << text
           << "timestamp" << timestamp;
    
  coll.insert_one(document.view());

  // SET title
  if(title)
    if(strlen(title) > 0)
      threadcoll.update_one(make_document(kvp("thread", thread)),
			    make_document(kvp("$set", make_document(kvp("title", title)))));
  if(thumbnail)
    if(strlen(thumbnail) > 0)
      threadcoll.update_one(make_document(kvp("thread", thread)),
			    make_document(kvp("$set", make_document(kvp("thumbnail", thumbnail)))));
}
