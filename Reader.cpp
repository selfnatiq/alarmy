#include <SPI.h>
#include <MFRC522.h>

#include <Arduino.h>
#include "Reader.h"

Reader::Reader(uint8_t ss_pin = 3, uint8_t rst_pin = 2)
    : _rc(MFRC522(ss_pin, rst_pin))
{
}

void Reader::begin()
{
  SPI.begin();
  _rc.PCD_Init();
}

// A -> active = Reader is listening
// T -> true = valid
// F -> false = invalid
char Reader::validate()
{
  // Look for new cards
  if (!_rc.PICC_IsNewCardPresent())
    return 'A';

  // Select one of the cards
  if (!_rc.PICC_ReadCardSerial())
    return 'A';

  byte letter;
  String content = "";

  for (byte i = 0; i < _rc.uid.size; i++)
  {
    content.concat(String(_rc.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(_rc.uid.uidByte[i], HEX));
  }

  content.toUpperCase();

  Serial.print("Tag: ");
  Serial.println(content);

  if (content.substring(1) == "CD 03 80 38")
    return 'T';

  return 'F';
}
