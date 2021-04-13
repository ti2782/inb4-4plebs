#include <csignal>
#include "downloader.h"

void terminate(int signal);

int main(int argc, char** argv)
{
  Downloader downloader;
  
  // Register Terminate Signal Handler
  signal(SIGINT, terminate);

  // Parse Arguments
  if(argc > 1)
    {
      std::string arg(argv[1]);
      if(arg.compare("-a") == 0)
      {

	if(argc > 2)
	  {
	    int numthreads = std::atoi(argv[2]);
	    std::cout << ">>INFO\nStarting Archive Update with " << numthreads << " threads..." << std::endl;
	    downloader.archiveThreads(numthreads);
	    return EXIT_SUCCESS;
	  }
	else
	  {
	    std::cout << ">>INFO\nStarting Archive Update..." << std::endl;
	    downloader.archiveAllThreads();
	    return EXIT_SUCCESS;
	  }
      }
      
    }
      
  while(true)
    {
      auto start = std::chrono::steady_clock::now();
      for(auto end = std::chrono::steady_clock::now(); std::chrono::duration_cast<std::chrono::minutes>(end - start).count() < 30; end = std::chrono::steady_clock::now())
	{
	  downloader.downloadThreads();
	}
      
      std::cout << ">>INFO\nStarting Archive Update..." << std::endl;
      downloader.archiveThreads(25);
      }
  
  return EXIT_SUCCESS;
}

void terminate(int signal)
{
  std::cout << ">>TERMINATE\n" << "Interrupt signal " << signal << std::endl;
  
  exit(signal);
}
