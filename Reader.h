#ifndef __READER_H__
#define __READER_H__

#include <MFRC522.h>

class Reader
{
public:
  Reader(uint8_t ss_pin = 3, uint8_t rst_pin = 2);
  void begin();
  bool isValid();

private:
  MFRC522 _rc;
};

#endif
