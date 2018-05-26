//
//  main.cpp
//  DCTool
//
//  Created by VenoMKO on 22/05/2018.
//  Copyright © 2018 VenoMKO. All rights reserved.
//

#include <iostream>

#include "yappy.hpp"
#include "utils.hpp"
#include "dc.hpp"

using namespace std;

#ifndef EXIT_FAILURE
#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1
#endif

const string CMD_UNPACK = "unpack";
const string CMD_UNPACK_FULL = "unpackfull";
const string CMD_PACK = "pack";
const string CMD_PACK_PCK = "packpck";


void PrintInfo()
{
  cout << "Usage:" << endl;
  cout << "dc unpack source.sft [destination_dir - optional] - unpack sft" << endl;
  cout << "dc unpack source.pck [destination_dir - optional] - unpack pck" << endl;
  cout << "dc unpackfull source.sft [destination_dir - optional] - unpack sft and all containing pck" << endl;
  cout << "dc pack source_dir [destination.sft - optional] - pack folder to sft" << endl;
  cout << "dc packpck source_dir [destination.pck - optional] - pack folder to pck" << endl;
}

int Pack(string const& source, string const& destination)
{
  if (Utils::EndsWith(destination, ".pck"))
  {
    return DC::PackPCK(source, destination);
  }
  return DC::PackSFT(source, destination);
}

int Unpack(string const& src, string const& destination, bool unpackPck)
{
  if (Utils::EndsWith(src, ".sft"))
  {
    return DC::UnpackSFT(src, destination, unpackPck);
  }
  return DC::UnpackPCK(src, destination);
}

int main(int argc, const char * argv[])
{
  if (argc == 1)
  {
    PrintInfo();
    return EXIT_FAILURE;
  }
  YappyFillTables();
  
  if (argv[1] == CMD_UNPACK || argv[1] == CMD_UNPACK_FULL)
  {
    if (argc < 3)
    {
      PrintInfo();
      return EXIT_FAILURE;
    }
    const string dest = argc >= 4 ? argv[3] : (Utils::ParentPath(argv[2]) + Utils::SEPARATOR + Utils::FileName(argv[2]));
    return Unpack(argv[2], dest, (argv[1] == CMD_UNPACK_FULL));
  }
  else if (argv[1] == CMD_PACK)
  {
    if (argc < 3)
    {
      PrintInfo();
      return EXIT_FAILURE;
    }
    
    const string dest = argc >= 4 ? argv[3] : (argv[2] + std::string(".sft"));
    return Pack(argv[2], dest);
  }
  else if (argv[1] == CMD_PACK_PCK)
  {
    if (argc < 3)
    {
      PrintInfo();
      return EXIT_FAILURE;
    }
    
    string dest = argc >= 4 ? argv[3] : (argv[2] + std::string(".pck"));
    if (!Utils::EndsWith(dest, ".pck"))
    {
      dest += dest.back() == '.' ? "pck" : ".pck";
    }
    return Pack(argv[2], dest);
  }
  else
  {
    PrintInfo();
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
