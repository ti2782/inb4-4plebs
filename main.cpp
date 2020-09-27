#include <csignal>
#include "downloader.h"

void terminate(int signal);

int main()
{
  // Register Terminate Signal Handler
  signal(SIGINT, terminate);
  
  Downloader downloader;
  while(true)
    {
      auto start = std::chrono::steady_clock::now();
      for(auto end = std::chrono::steady_clock::now(); std::chrono::duration_cast<std::chrono::minutes>(end - start).count() < 30; end = std::chrono::steady_clock::now())
	{
       	  downloader.downloadThreads();
	}
      
      std::cout << ">>INFO\nStarting Archive Update..." << std::endl;
      downloader.archiveThreads(10);
      //downloader.archiveAllThreads();
    }
  
  return EXIT_SUCCESS;
}

void terminate(int signal)
{
  std::cout << ">>TERMINATE\n" << "Interrupt signal " << signal << std::endl;
  
  exit(signal);
}
