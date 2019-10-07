//  DCTool
//
//  Created by VenoMKO on 22/05/2018.
//  Copyright © 2018 VenoMKO. All rights reserved.
//
#include <fstream>
#include <vector>

namespace DC
{
  struct PCKInfo
  {
    uint64_t Unk1 = 0;
    uint8_t Flags = 0;
    uint32_t Offset = 0;
    uint32_t DecompressedSize = 0;
    uint32_t Size = 0;
    uint32_t Alignment = 0;
    
    explicit PCKInfo() {};
    PCKInfo(std::basic_istream<char>& f);
    void GetRawBytes(std::basic_istream<char>& s, unsigned char* out, unsigned int *outlen);
    bool DecryptPCK(unsigned char *buff, int len);
    void Write(std::basic_ostream<char>& s);
    
    static uint32_t InfoSize()
    {
      return 25;
    }
  };
  
  struct SFTInfo
  {
    explicit SFTInfo() {};
    SFTInfo(std::ifstream& s);
    void Export(std::string const& root, std::ifstream& s, bool pck = false) const;
    static bool ExportPCK(const unsigned char *in, int inlen, std::string const& dest);
    static bool ExportPCK(std::basic_istream<char>& s, std::string const& dest);
    static bool ReadSFT(std::vector<SFTInfo>& contents, std::ifstream& s);
    static uint32_t InfoSize()
    {
      return 252;
    }
    
    std::string Name;
    uint32_t DataSize = 0;
    uint32_t Offset = 0;
    uint32_t BlockSize = 0;
  };
  
  int PackPCK(std::string const& source, std::string const& destination);
  
  int UnpackPCK(std::string const& src, std::string const& destination);
  int UnpackSFT(std::string const& src, std::string const& destination, bool unpackPck);
  
  int PatchSFT(std::string const& source, std::string const& sft, std::string const& name);
}
