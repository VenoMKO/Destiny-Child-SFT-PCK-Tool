//  DCTool
//
//  Created by VenoMKO on 22/05/2018.
//  Copyright © 2018 VenoMKO. All rights reserved.
//
#include <string>
#include <vector>

namespace Utils
{
#ifdef _WIN32
  static const std::string SEPARATOR = "\\";
#else
  static const std::string SEPARATOR = "/";
#endif
  static const int PCK_MAGIC = 4932432;
  bool CreateDir(std::string const& path);
  std::vector<std::string> ContentsOfDir(std::string const& src);
  std::string ReadString(std::ifstream& s, int max = 240);
  std::string ParentPath(std::string const& path);
  std::string FileName(std::string const& path);
  inline bool EndsWith(std::string const & value, std::string const & ending)
  {
    if (ending.size() > value.size()) return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
  }
}
