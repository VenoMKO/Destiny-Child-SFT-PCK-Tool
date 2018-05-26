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
const string CMD_PATCH = "patch";


void PrintInfo()
{
  cout << "Usage:" << endl;
  cout << "dc unpack source.pck [destination_dir - optional] - unpack pck" << endl;
  cout << "dc unpack source.sft [destination_dir - optional] - unpack sft" << endl;
  cout << "dc unpackfull source.sft [destination_dir - optional] - unpack sft and all containing pck" << endl;
  cout << "dc pack source_dir [destination.pck - optional] - pack folder to pck" << endl;
  cout << "dc patch target.sft source [name - optional] - replace sft's file called 'name' with the provided 'source' file" << endl;
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
  if (argc < 3)
  {
    PrintInfo();
    return EXIT_FAILURE;
  }
  
  YappyFillTables();
  
  if (argv[1] == CMD_UNPACK || argv[1] == CMD_UNPACK_FULL)
  {
    string dest = argc >= 4 ? argv[3] : (Utils::ParentPath(argv[2]) + Utils::SEPARATOR + Utils::FileName(argv[2]));
    return Unpack(argv[2], dest, (argv[1] == CMD_UNPACK_FULL));
  }
  else if (argv[1] == CMD_PACK)
  {
    string dest = argc >= 4 ? argv[3] : (argv[2] + std::string(".pck"));
    if (!Utils::EndsWith(dest, ".pck"))
    {
      dest += dest.back() == '.' ? "pck" : ".pck";
    }
    return DC::PackPCK(argv[2], dest);
  }
  else if (argv[1] == CMD_PATCH)
  {
    string name = argc >= 5 ? argv[4] : "";
    DC::PatchSFT(argv[2], argv[3], name);
  }
  else
  {
    PrintInfo();
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
