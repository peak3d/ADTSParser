#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

class ID3TAG
{
public:
  enum PARSECODE
  {
    PARSE_SUCCESS,
    PARSE_FAIL,
    PARSE_NO_ID3
  };

  PARSECODE parse(FILE *f);
private:
  static uint64_t getSize(const uint8_t *data, unsigned int size, unsigned int shift);

  static const unsigned int HEADER_SIZE = 10;

  uint8_t m_majorVer;
  uint8_t m_flags;
  uint64_t m_timestamp;
};

uint64_t ID3TAG::getSize(const uint8_t *data, unsigned int len, unsigned int shift)
{
  uint64_t size(0);
  const uint8_t *dataE(data + len);
  for (data; data < dataE; ++data)
    size = size << shift | *data;
  return size;
};

ID3TAG::PARSECODE ID3TAG::parse(FILE *f)
{
  uint8_t buffer[64];
  uint64_t startPos = ftell(f);
  PARSECODE ret(PARSE_NO_ID3);

  if (fread(buffer, 1, HEADER_SIZE, f) != HEADER_SIZE || memcmp(buffer, "ID3", 3) != 0)
    goto FAIL;

  ret = PARSE_FAIL;
  
  m_majorVer = buffer[3];
  m_flags = buffer[5];
  uint32_t size = static_cast<uint32_t>(getSize(buffer + 6, 4, 7));

  //iterate through frames and search timestamp
  while (size > HEADER_SIZE)
  {
    if (fread(buffer, 1, HEADER_SIZE, f) != HEADER_SIZE)
      goto FAIL;
    
    uint32_t frameSize = static_cast<uint32_t>(getSize(buffer + 4, 4, 8));

    if (memcmp(buffer, "PRIV", 4) == 0 && frameSize == 53)
    {
      if (fread(buffer, 1, frameSize, f) != frameSize)
        goto FAIL;
      
      if (strncmp(reinterpret_cast<const char*>(buffer), "com.apple.streaming.transportStreamTimestamp", 44) == 0 && buffer[44] == 0)
      {
        m_timestamp = getSize(buffer+45, 8, 8);
      }
    }
    else
      fseek(f, frameSize, SEEK_CUR);
    size -= (HEADER_SIZE + frameSize);
  }
  return PARSE_SUCCESS;
FAIL:
  fseek(f, startPos, SEEK_SET);
  return ret;
}

/**********************************************************************************************************************************/















int main(int argc, char *argv[])
{
  ID3TAG tag;
  FILE *f = fopen("C:\\Temp\\euro.aac", "rb");
  while (tag.parse(f) == ID3TAG::PARSE_SUCCESS);
  fclose(f);
}