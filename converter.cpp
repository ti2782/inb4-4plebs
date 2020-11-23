#include "converter.h"

Converter::Converter()
{
  
}

Converter::~Converter()
{
  
}

void Converter::addColumn(std::ofstream& f)
{
  f << "<div class=\"col-sm-3\">" << std::endl;
}

void Converter::addRow(std::ofstream& f)
{
  f << "<div class=\"row row-no-gutters\">" << std::endl;
}

void Converter::addContainer(std::ofstream& f)
{
  f << "<div class=\"container\">" << std::endl;
}

void Converter::endDiv(std::ofstream& f)
{
  f << "</div>" << std::endl;
}

void Converter::addThreadCard(std::ofstream& f, ThreadCard* card)
{
  if(!card)
    return;

  sanitizeString(card->title);
  sanitizeString(card->comment);
  
  f << "<div class=\"card bg-dark text-white\">" << std::endl <<
    "<img class=\"card-img-top\" src=\"" << card->thumb << "\" alt=\"Card image cap\">" << std::endl << 
    "<div class=\"card-body\">" << std::endl <<
    "<h4 class=\"card-title\">" << card->title << "</h4>" << std::endl <<    
    "<p class=\"card-text\">" << card->comment.substr(0, 128).append("...") << "</p> </div>" << std::endl <<
    "<a href=\"https://archive.4plebs.org/pol/thread/" << card->threadnum << "\" class=\"btn btn-primary\">View Thread</a>" << std::endl <<
    "<div class=\"card-footer\">" << std::endl << "<h6 class=\"card-subtitle mb-2 text-muted\">" << card->date << "</h6>" << std::endl;
  
  endDiv(f); // card
  endDiv(f); // card-footer
}

void Converter::sanitizeString(std::string& text)
{
  for(std::size_t pos = text.find("<"); pos != std::string::npos; pos = text.find("<"))
    {
      std::size_t end = text.find(">", pos);
      if(end != std::string::npos)
	text.erase(pos, end + 1);
      else
	text.erase(pos, std::string::npos);
    }
}
