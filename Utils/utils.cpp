//  DCTool
//
//  Created by VenoMKO on 22/05/2018.
//  Copyright © 2018 VenoMKO. All rights reserved.
//
#include "utils.hpp"
#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>

namespace fs = boost::filesystem;
namespace
{
  const std::string _arr[] = {".DS_Store", "desktop.ini", "Desktop.ini", "thumbs.db", "Thumbs.db", "metadata.bin", "Metadata.bin"};
  std::vector<std::string> IgnoreSftNames(_arr, _arr + sizeof(_arr)/sizeof(std::string));
}

namespace Utils
{
  bool CreateDir(std::string const& path)
  {
    fs::path p(path);
    
    if (!fs::exists(p) && !fs::create_directories(p))
    {
      std::cout << "Failed to create directory " << p << std::endl;
      return false;
    }
    if (!fs::is_directory(p))
    {
      std::cout << "Path is not a directory " << p << std::endl;
      return false;
    }
    return true;
  }
  
  std::string FileName(std::string const& path)
  {
    fs::path p(path);
    p.replace_extension(fs::path());
    return p.filename().string();
  }
  
  std::vector<std::string> ContentsOfDir(std::string const& source)
  {
    fs::path src(source);
    fs::directory_iterator it(src), eod;
    std::vector<std::string> result;
    BOOST_FOREACH(fs::path const &p, std::make_pair(it, eod))
    {
      if(fs::is_regular_file(p) && !std::any_of(IgnoreSftNames.begin(), IgnoreSftNames.end(), [p](auto const& i){ return i == p.filename().string(); }))
      {
        result.push_back(p.string());
      }
      else if (fs::is_directory(p))
      {
        auto contents = ContentsOfDir(p.string());
        result.insert(result.end(), contents.begin(), contents.end());
      }
    }
    return result;
  }
  
  std::string ReadString(std::ifstream& s, int max)
  {
    char c = 0;
    int cnt = 1;
    s.read(&c, sizeof(c));
    std::string result;
    while (c && cnt <= max)
    {
      result += c;
      s.read(&c, sizeof(c));
      cnt++;
    }
    return result;
  }
  
  std::string ParentPath(std::string const& path)
  {
    fs::path parent(path);
    return parent.parent_path().string();
  }
}
