#include <DHT11.h>
#include <MYWIFI.h>
#include <SoftwareSerial.h>

MYWIFI  myWifi  ;
DHT11   dht(2)  ;

void setup()  
{
  Serial.begin(9600);
  while (!Serial) {;}
  
  myWifi.Reset();
  //myWifi.Begin("chonglin","Office@SH");
  Serial.println(myWifi.GetIP());
  myWifi.UDPSERVER(5000,5000);
}

void loop()
{
  byte  index = 0,Temperature = 0,mark = 0;
  int   bufLength = 0;
  byte  indexTemp1,indexTemp2,indexBytes;
  bool  flag = false,ReadTemp = true,isSend = true;
  byte  values[5];
  unsigned int  Value,sendBuf[270];
  unsigned long previousMillis = millis();

  while(1)
  {  
    if(isSend && (millis() - previousMillis >= 5000))
    {
      dht.dump();
      myWifi.UDPSEND("10.200.200.26",5000,"{\"Temp\":\"" + String(dht.readTemp()) + "\",\"Humi\":\"" + String(dht.readHumi()) + "\"}");
      previousMillis = millis();
    }
    
    char c = myWifi.readFromESP();
    if(c==NULL){  continue;  }
    if(c== '{'){  flag = true;  }
    if(!flag)  { continue; }

    isSend = false;
    
    if(ReadTemp)
    {
      //读取温度数据
      if(c == 'T')
      {
        indexTemp1 = index + 2;
        indexTemp2 = index + 3;
      }

      if((index == indexTemp1) && (c >= 48) && (c <= 57))
      {
        Temperature = Temperature + (c - 48) * 10;
      }

      if((index == indexTemp2) && (c >= 48) && (c <= 57))
      {
        Temperature = Temperature + (c - 48);
        ReadTemp = false;
      }
    }
    else
    {
      //读取字节数组
      if(c >= 48 && c <= 57)
      {
        values[mark] = c - 48;
        mark ++;
      }

      else
      {
        int mult = 1;
        for(int i = mark - 1;i>=0;i--)
        {
          Value = Value + values[i] * mult;
          mult = mult * 10;
        }
        if(Value != 0)
        {
          sendBuf[bufLength++] = Value;
        }
        mark = 0;
        Value = 0;
      }
    }

    index++;
    if(c == '}')
    {
      Serial.println(bufLength);
      flag = false;
      ReadTemp = true;
      index = 0;
      mark = 0;
      bufLength = 0;
      isSend = true;
    }
  }
}
