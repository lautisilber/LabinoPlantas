#include <SD_Labino.h>
#include <SPI.h>
#include <SD.h>

SD_Labino sd = SD_Labino(10, 13);

void setup()
{
  Serial.begin(9600);
  sd.SD_init(false);
  for (int i = 0; i < 4; i++)
  {
    sd.Log("d\newrt\nfoaisdnf\n" + String(i) + '\n');
  }
}

void loop()
{
  delay(1500);
}

void Tabbed(String s)
{
  int last = -1;
  int index = 0;
  while (true)
  {
    index = s.indexOf('\n', index + 1);
    if (index == -1)
      break;
    Serial.println('h');
  }
}
