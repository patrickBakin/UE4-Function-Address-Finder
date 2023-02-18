#pragma once

namespace Algorithm
{
  inline std::vector<int> getPartialMatchTable(const std::vector<unsigned char> &pattern)
    {
      const int patternSize = pattern.size();
      std::vector<int> partialMatchTable(patternSize, 0);

      int j = 0;
      for (int i = 1; i < patternSize; i++) {
        while (j > 0 && (pattern[j] != pattern[i] && pattern[j] != 0xff)) {
          j = partialMatchTable[j - 1];
        }
        if (pattern[j] == pattern[i] || pattern[j] == 0xff) {
          j++;
        }
        partialMatchTable[i] = j;
      }

      return partialMatchTable;
    }


  int searchArray(const std::vector<unsigned char> &array, const std::vector<unsigned char> &pattern, bool lastResult) {
    
    int arraySize = array.size();
    int patternSize = pattern.size();
    std::vector<int> partialMatchTable = getPartialMatchTable(pattern);

    int j = 0;
    int result = -1;
    for (int i = 0; i < arraySize; i++) {
      while (j > 0 && (pattern[j] != array[i] && pattern[j] != 0xff)) {
        i=i-j+1;
        j = partialMatchTable[j - 1];
        
      }
      if (pattern[j] == array[i] || pattern[j] == 0xff) {
        j++;
      }
      if (j == patternSize) {
        if (lastResult) {
          result = i - patternSize + 1;
        } else {
          return i - patternSize + 1;
        }
        i=i-j+1;
        j = partialMatchTable[j - 1];
        
      }
    }

    return result;
  
    
  }

  inline int64_t ScanFor(int64_t StartAddress,std::vector<BYTE> bytearray,bool lastresult,size_t Size=3000)
  {
    int64_t result=-1;
    //size_t Size =3000;
    StartAddress = lastresult ? StartAddress-static_cast<int64_t>(Size):StartAddress ;
    
    std::vector<BYTE> buffer(Size);
    size_t bytesRead;
    if(ReadProcessMemory(hProcess, reinterpret_cast<void*>(StartAddress), buffer.data(), Size, &bytesRead))
    {
      int indexfound=searchArray(buffer,bytearray,lastresult);
      if(indexfound>=0)
        {
          return StartAddress+indexfound;
        }
    }

    return result;
  }
  //buffer = Region
  int64_t ScanforStringRef(const std::vector<BYTE>& buffer,const wchar_t* String,int64_t AbsoluteRegionStartAddress,int32_t offset,std::string Name)
  {
    int64_t result=0x0;
    for(uint32_t i=0;i<buffer.size();i++)
    {
      if(i+offset >=buffer.size())
      {
        return result;
      }
   
      if((buffer[i]==0x4C || buffer[i]==0x48) && buffer[i+1]==0x8D)
      {
        int relative =  *(int*)(&buffer[i+offset]);

        int64_t absoluteStringaddr = (int64_t)(AbsoluteRegionStartAddress+i+offset+relative+4);

        int StringLength=wcslen(String)*sizeof(String[0]);
   

        std::vector<wchar_t>  StringBuffer(wcslen(String));
       
        SIZE_T bytesRead;
        if(ReadProcessMemory(hProcess, reinterpret_cast<void*>(absoluteStringaddr), StringBuffer.data(), StringLength, &bytesRead))
        {
         
          
          if(std::wstring(StringBuffer.begin(),StringBuffer.end())==String)
          {
            std::cout << "Searching for " << Name <<std::endl;

            
            return AbsoluteRegionStartAddress+i;
          }
          
        }
        
        
      }
      
    }
    return result;
  }
 
  template<typename T>
  T ReadAs(uint64_t address)
  {
    std::vector<T> buffer(1);
    if(ReadProcessMemory(hProcess,(void*)address,buffer.data(),sizeof(T),NULL))
    {
      return buffer[0];
      
    }
    return (T)0;
  }

  inline bool CheckNSkipJump(int64_t AddrIn,int64_t& AddrOut)
  {
    BYTE Read=ReadAs<BYTE>(AddrIn);
    if(Read==0xE9 || Read==0xEB)
    {
      AddrOut = AddrIn+ReadAs<int32_t>(AddrIn+1)+0x5;
      return true;
    }
    AddrOut=AddrIn;
    return false;
  }
}
