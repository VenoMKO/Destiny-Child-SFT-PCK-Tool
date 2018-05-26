//  DCTool
//
//  Created by VenoMKO on 22/05/2018.
//  Copyright © 2018 VenoMKO. All rights reserved.
//
#include <iostream>
#include <boost/interprocess/streams/bufferstream.hpp>

#include "dc.hpp"
#include "aes256.h"
#include "yappy.hpp"
#include "utils.hpp"

namespace
{
  const int AES_BLOCK_SIZE = 16;
  const int MAX_NAME_LENGTH = 240;
  const unsigned char *KEY = (const unsigned char *)"\x37\xea\x79\x85\x86\x29\xec\x94\x85\x20\x7c\x1a\x62\xc3\x72\x4f\x72\x75\x25\x0b\x99\x99\xbd\x7f\x0b\x24\x9a\x8d\x85\x38\x0e\x39";
  
  std::string ExtensionByFirstByte(const unsigned char byte)
  {
    switch (byte)
    {
      case '[':
      case '{':
        return "json";
      case '#':
        return "mtn";
      case 'm':
        return "moc";
      case 'M':
        return "mmd";
      case 0x89:
        return "png";
      default:
        break;
    }
    return "unk";
  }
}

namespace DC
{
  PCKInfo::PCKInfo(std::basic_istream<char>& f)
  {
    f.read((char*)&Unk1, sizeof(Unk1));
    f.read((char*)&Flags, sizeof(Flags));
    f.read((char*)&Offset, sizeof(Offset));
    f.read((char*)&DecompressedSize, sizeof(DecompressedSize));
    f.read((char*)&Size, sizeof(Size));
    f.read((char*)&Alignment, sizeof(Alignment));
  }
  
  void PCKInfo::GetRawBytes(std::basic_istream<char>& s, unsigned char* out, unsigned int *outlen)
  {
    unsigned int sizeOnDisk = Size;
    *outlen = sizeOnDisk;
    
    unsigned char *src = new unsigned char[sizeOnDisk * 2];
    s.seekg(Offset);
    s.read((char*)src, sizeOnDisk);
    
    if (Flags & 2)
    {
      DecryptPCK(src, sizeOnDisk);
    }
    if (Flags & 1)
    {
      const unsigned char *end = src + DecompressedSize - Alignment;
      YappyUnCompress(src, end, out);
      delete[] src;
      return;
    }
    memcpy(out, src, sizeOnDisk);
    delete[] src;
  }
  
  bool PCKInfo::DecryptPCK(unsigned char *buff, int len)
  {
    aes256_context ctx;
    aes256_init(&ctx, KEY);
    
    const unsigned char *end = buff + len;
    while (buff < end)
    {
      aes256_decrypt_ecb(&ctx, buff);
      buff += AES_BLOCK_SIZE;
    }
    aes256_done(&ctx);
    return true;
  }
  
  void PCKInfo::Write(std::basic_ostream<char>& s)
  {
    s.write((char*)&Unk1, sizeof(Unk1));
    s.write((char*)&Flags, sizeof(Flags));
    s.write((char*)&Offset, sizeof(Offset));
    s.write((char*)&DecompressedSize, sizeof(DecompressedSize));
    s.write((char*)&Size, sizeof(Size));
    s.write((char*)&Alignment, sizeof(Alignment));
  }
  
  FileInfo::FileInfo(std::ifstream& s)
  {
    Name = Utils::ReadString(s, MAX_NAME_LENGTH);
    size_t pos = s.tellg();
    pos += MAX_NAME_LENGTH - Name.size() - 1;
    s.seekg(pos);
    s.read((char*)&DataSize, sizeof(long));
    s.read((char*)&Offset, sizeof(int));
    s.read((char*)&BlockSize, sizeof(int));
  }
  
  bool FileInfo::ExportPCK(std::basic_istream<char>& s, std::string const& dest)
  {
    int magic = 0;
    s.read((char*)&magic, sizeof(magic));
    if (magic != Utils::PCK_MAGIC)
    {
      std::cout << "Not a PCK file" << std::endl;
      return false;
    }
    
    uint32_t count = 0;
    uint32_t unk = 0;
    s.read((char*)&unk, sizeof(unk));
    s.read((char*)&count, sizeof(count));
    
    std::vector<uint64_t> metdata;
    for (unsigned int i = 0; i < count; ++i)
    {
      PCKInfo info(s);
      metdata.push_back(info.Unk1);
      size_t pos = s.tellg();
      unsigned int len = 0;
      unsigned char *data = new unsigned char[info.Size + info.Size];
      info.GetRawBytes(s, data, &len);
      s.seekg(pos);
      std::string extension = ExtensionByFirstByte(data[0]);
      std::string path = dest + Utils::SEPARATOR + std::to_string(i) + "." + extension;
      std::ofstream out(path);
      if (len > 0)
      {
        out.write((char *)data, len);
      }
      out.close();
      delete[] data;
    }
    {
      std::string path = dest + Utils::SEPARATOR + "Metadata.bin";
      std::ofstream out(path);
      count = (int32_t)metdata.size();
      out.write((char *)&unk, sizeof(unk));
      out.write((char *)&count, sizeof(count));
      for (unsigned long const field : metdata)
      {
        out.write((char *)&field, sizeof(field));
      }
      out.close();
    }
    std::cout << "Exported: " << dest << std::endl;
    return true;
  }
  
  bool FileInfo::ExportPCK(const unsigned char *in, int inlen, std::string const& dest)
  {
    boost::interprocess::bufferstream s((char*)in, inlen);
    return ExportPCK(s, dest);
  }
  
  void FileInfo::Export(std::string const& root, std::ifstream& s, bool pck) const
  {
    if (!DataSize)
    {
      return;
    }
    
    unsigned char *buff = new unsigned char[DataSize];
    s.seekg(Offset);
    s.read((char *)buff, DataSize);
    if (!s.good())
    {
      std::cout << "Failed to read: " << Name << std::endl;
      delete[] buff;
      return;
    }
    
    if (pck && Utils::EndsWith(Name, ".pck"))
    {
      Utils::CreateDir(root + Name);
      if (ExportPCK(buff, DataSize, root + Name))
      {
        delete[] buff;
        return;
      }
    }
    
    std::string dest = root + Name;
    if (!Utils::CreateDir(Utils::ParentPath(dest)))
    {
      delete[] buff;
      return;
    }
    
    std::ofstream out(dest);
    out.write((char *)buff, DataSize);
    out.close();
    std::cout << "Exported: " << root + Name << std::endl;
    delete[] buff;
  }
  
  int PackPCK(std::string const& source, std::string const& destination)
  {
    std::vector<std::string> contents = Utils::ContentsOfDir(source);
    std::sort(contents.begin(), contents.end(), [](auto const& a, auto const& b){
      int ai = 0;
      int bi = 0;
      try
      {
        ai = std::stoi(Utils::FileName(a));
      }
      catch (...)
      {
        return false;
      }
      try
      {
        bi = std::stoi(Utils::FileName(b));
      }
      catch (...)
      {
        return true;
      }
      return ai < bi;});
    std::ofstream out = std::ofstream(destination);
    std::ifstream meta = std::ifstream(source + Utils::SEPARATOR + "metadata.bin");
    int32_t metasize;
    int32_t unk;
    meta.read((char *)&metasize, sizeof(metasize));
    meta.read((char *)&unk, sizeof(unk));
    if (!meta.good())
    {
      std::cout << "Missing metadata.bin" << std::endl;
      return EXIT_FAILURE;
    }
    else if (metasize != contents.size())
    {
      std::cout << "Incompatible metadata.bin" << std::endl;
      return EXIT_FAILURE;
    }
    out.write((char*)&Utils::PCK_MAGIC, sizeof(Utils::PCK_MAGIC));
    out.write((char*)&unk, sizeof(unk));
    out.write((char*)&metasize, sizeof(metasize));
    for (int i = 0; i < DC::PCKInfo::InfoSize() * metasize; ++i)
    {
      uint8_t zero = 0;
      out.write((char*)&zero, sizeof(zero));
    }
    for (int i = 0; i < metasize; ++i)
    {
      std::ifstream in(contents[i], std::ios::ate);
      DC::PCKInfo info;
      info.Size = (uint32_t)in.tellg();
      info.Offset = (uint32_t)out.tellp();
      meta.read((char *)&info.Unk1, sizeof(info.Unk1));
      in.seekg(std::ios::beg);
      copy_n(std::istreambuf_iterator<char>(in), info.Size, std::ostreambuf_iterator<char>(out));
      uint32_t endPos = (uint32_t)out.tellp();
      out.seekp(sizeof(Utils::PCK_MAGIC) + sizeof(unk) + sizeof(metasize) + (i * DC::PCKInfo::InfoSize()));
      info.Write(out);
      out.seekp(endPos);
      in.close();
    }
    out.close();
    std::cout << "Packed " << destination << std::endl;
    return EXIT_SUCCESS;
  }
  
  int PackSFT(std::string const& source, std::string const& destination)
  {
    auto contents = Utils::ContentsOfDir(source);
    int size = static_cast<int>(contents.size());
    std::ofstream s = std::ofstream(destination);
    s.write(reinterpret_cast<char*>(&size), sizeof(size));
    
    uint32_t pos = size * DC::FileInfo::InfoSize() + static_cast<uint32_t>(sizeof(size));
    for (int i = 0; i < size; ++i)
    {
      auto const& path = contents[i];
      DC::FileInfo info;
      info.Name = path.substr(source.size());
      info.Offset = pos;
      
      pos += info.BlockSize;
    }
    return EXIT_SUCCESS;
  }
  
  int UnpackPCK(std::string const& src, std::string const& destination)
  {
    if (!Utils::CreateDir(destination))
    {
      return EXIT_FAILURE;
    }
    
    std::ifstream s(src, std::ios::binary);
    if (!s.is_open())
    {
      std::cout << "Failed to open " << src << std::endl;
      return EXIT_FAILURE;
    }
    s.seekg(0, std::ios::beg);
    int result = DC::FileInfo::ExportPCK(s, destination);
    s.close();
    return result;
  }
  
  int UnpackSFT(std::string const& src, std::string const& destination, bool unpackPck)
  {
    if (!Utils::CreateDir(destination))
    {
      return EXIT_FAILURE;
    }
    
    std::ifstream s(src, std::ios::binary);
    if (!s.is_open())
    {
      std::cout << "Failed to open " << src << std::endl;
      return EXIT_FAILURE;
    }
    s.seekg(0, std::ios::beg);
    unsigned int count = 0;
    s.read((char*)&count, sizeof(count));
    
    std::vector<DC::FileInfo> contents;
    for (unsigned int i = 0; i < count; ++i)
    {
      DC::FileInfo info(s);
      if (!s.good())
      {
        std::cout << "Stream error!" << std::endl;
        return EXIT_FAILURE;
      }
      contents.push_back(info);
    }
    std::cout << "Loaded " << contents.size() << " elements" << std::endl;
    
    std::string dst = Utils::EndsWith(destination, Utils::SEPARATOR) ? destination : destination + Utils::SEPARATOR;
    for (DC::FileInfo const& info : contents)
    {
      info.Export(dst, s, unpackPck);
    }
    std::cout << "Done!" << std::endl;
    return EXIT_SUCCESS;
  }
}
