#pragma once
#include <thread>
#include "Headers/NameClasses.h"
namespace UName
{
  inline int NumFound=0;
  inline bool EndSearching=false;
  bool FindFNameMethod1(MEMORY_BASIC_INFORMATION memInfo)
  {
      
    std::size_t size = memInfo.RegionSize;
    std::vector<BYTE> buffer(size);

    SIZE_T bytesRead;
    if(EndSearching)
    {
      return false;
    }
    if (ReadProcessMemory(hProcess, memInfo.BaseAddress, buffer.data(), size, &bytesRead)/* && bytesRead == size*/)
    {
      if(const auto GetNamesStrAddr = Algorithm::ScanforStringRef(buffer,L"Hardcoded name '%s' at index %i was duplicated (or unexpected concurrency). Existing entry is '%s'.",(int64_t)memInfo.BaseAddress,3,"FName::GetNames()"))
      {
        
        const auto BeforeCallGetName =Algorithm::ScanFor(GetNamesStrAddr,{0xBA,0x01,0x00,0x00,0x00,0x48,0x8B,0xC8,0xE8},true);
        if(BeforeCallGetName>=0)
        {
          const auto CallGetNameFunction = Algorithm::ScanFor(BeforeCallGetName,{ 0xE8, 0xFF,0xFF,0xFF,0xFF,0xFF,0x8B},true);
          if(CallGetNameFunction>=0)
          {
            const auto indexE8 =CallGetNameFunction-reinterpret_cast<int64_t>(memInfo.BaseAddress);
            const auto relative = *reinterpret_cast<int*>(&buffer[indexE8+1]);
            const auto GetNameFunctionAddress = CallGetNameFunction+relative+4;
            if(const auto NamesDataMov = Algorithm::ScanFor(GetNameFunctionAddress,{0x48,0x8B},false))
            {
              int indexMov = NamesDataMov-reinterpret_cast<int64_t>(memInfo.BaseAddress);
              int relaTive = *reinterpret_cast<int*>(&buffer[indexMov+3]);
              const auto NamesDataAddress = NamesDataMov+3 +relaTive+4;
              std::cout << "Found GName 0x"<<std::hex<< NamesDataAddress<<std::endl;;
              TNameEntryArray::Names = new TNameEntryArray(NamesDataAddress);
              ProfileGen::GetProfile().GNameOffset=NamesDataAddress-reinterpret_cast<int64_t>(BaseAddress);
              
              EndSearching=true;
              return true;
            }
                
          }
        }
            
      }
    }
    return false;
  }
  bool FindFNameMethod2(MEMORY_BASIC_INFORMATION memInfo)
  {
    std::size_t size = memInfo.RegionSize;
    std::vector<int64_t> buffer(size/8);
    SIZE_T bytesRead;
    

   
    if (ReadProcessMemory(hProcess, memInfo.BaseAddress, buffer.data(), size, &bytesRead)/* && bytesRead == size*/)
    {
      
      for(int index=0;index<buffer.size();index++)
      {
        if(EndSearching)
        {
          return false;
        }
       
        const auto ptr2 =buffer[index];
        
        //totally forgot what i wrote, but the procedures is like finding in CE
        if(int64_t name1 =Algorithm::ReadAs<int64_t>(ptr2) ? Algorithm::ReadAs<int64_t>(ptr2): 0)
        {
          if(int64_t NameIndexAddr = Algorithm::ReadAs<int64_t>(name1)? Algorithm::ReadAs<int64_t>(name1) : 0)
          {
                
                NameIndexAddr=NameIndexAddr+0xC;
                size_t stringsize = 0x10;
                std::vector<byte> stringbuff(stringsize);
                size_t bRead;
                if(ReadProcessMemory(hProcess,(void*)NameIndexAddr,stringbuff.data(),stringsize,&bRead))
                {
                  for(int bi=0;bi<=4;bi++)
                  {
                    auto checkvalue =*reinterpret_cast<int*>(&stringbuff[bi]);
                    if(checkvalue==0x656E6F4E)
                    {

                      auto GNamesAddr=reinterpret_cast<int64_t>(memInfo.BaseAddress)+index*8;
                      
                      TNameEntryArray GlobalNames = GNamesAddr;
                      if(strcmp(GlobalNames.GetById(0).GetAnsiNameValue().c_str(),"None")!=0)
                      {
                        std::cout <<"False GNames Address detected: "<<std::hex<<GNamesAddr<<std::endl;
                        continue;
                      }
                      std::cout <<"Found GNames 0x"<<std::hex << GNamesAddr<<std::endl;
                      
                      NumFound++;
                      if(NumFound==2)
                      {
                        ProfileGen::GetProfile().GNameOffset=GNamesAddr-reinterpret_cast<int64_t>(BaseAddress);
                        TNameEntryArray::Names = new TNameEntryArray(GlobalNames);
                        EndSearching=true;
                        
                        return true;
                      }
                      
                      
                    }
                  }
                }
                
              
            
          }
        }
      }
    }
    return false;
  }

  bool FindFNamePool(MEMORY_BASIC_INFORMATION memInfo)
  {
    
          
            std::size_t size = memInfo.RegionSize;
            std::vector<int64_t> buffer(size/8);

          
            SIZE_T bytesRead;
            int Num8Bytes;
            
            //Method is as same as manually doing in CE
            if (ReadProcessMemory(hProcess, memInfo.BaseAddress, buffer.data(), size, &bytesRead)/* && bytesRead == size*/)
            {

              Num8Bytes=static_cast<int32_t>(bytesRead/8);

              for (int j = 0; j < Num8Bytes; j++)
              {
                
                auto b = buffer[j];
             
              
                if(EndSearching)
                {
                  return false;
                }
                
                if(b)
                {

                
                  std::vector<int64_t> tempbuffer(1);
                  SIZE_T TempBytesRead;

                  //Read the value of that address whether it point to any address, store in tempbuffer
                  if(ReadProcessMemory(hProcess, reinterpret_cast<LPCVOID>(b),tempbuffer.data(),sizeof(void*),&TempBytesRead))
                  {
                    //ByteArray, contains Readable String
                    char* ByteArray = static_cast<char*>(static_cast<void*>(&tempbuffer[0]));
                
                    for(int bi=0;bi<=4;bi++)
                    {
                      int checkvalue =*(int*)(&ByteArray[bi]);
                      
                      //Means "None"
                      if(checkvalue==0x656E6F4E)
                      {

                        //If address is in the module
                        
                        if(b<reinterpret_cast<int64_t>(BaseAddress))
                        {
                          auto FNamePoolAddr= reinterpret_cast<int64_t>(memInfo.BaseAddress) + j*8-0x10;
                          std::cout  <<  "Found FNamePool 0x" << std::hex <<b << " <-- 0x" <<FNamePoolAddr<<std::endl;
                            
                          FNamePool::NamesData = new FNamePool(FNamePoolAddr); 
                          EndSearching=true;
                         
                            
                          ProfileGen::GetProfile().GNameOffset=FNamePoolAddr-reinterpret_cast<int64_t>(BaseAddress);
                          ProfileGen::GetProfile().UseFNamePool=true;
                          
                           return true;
                        }
                          
                      
                      }
                    
                    }
                    
                  }
              
            
                }
              }
            }
      
    return false;
  }
  bool FindNames(ProfileGen& PG)
  {
  
    MEMORY_BASIC_INFORMATION memInfo1;
    std::vector<std::thread> FName1SearchingThreads;
    std::vector<std::thread> FName2SearchingThreads;
    
    if(ProfileGen::GetProfile().EngineVersion/100>=423)
    {
        std::vector<std::thread> FNamePoolSearchingThreads;
        for (char *p = (char *)moduleInfo.lpBaseOfDll;
          p < (char *)moduleInfo.lpBaseOfDll + moduleInfo.SizeOfImage;
          p += memInfo1.RegionSize)
        {
        
          SIZE_T size = VirtualQueryEx(hProcess, p, &memInfo1, sizeof(memInfo1));
          if (size == 0)
          {
          
            std::cerr << "VirtualQueryEx failed" << std::endl;
            return false;
          }

          FNamePoolSearchingThreads.push_back(std::thread(FindFNamePool,memInfo1));
        
        }
        std::cout << "Searching for FNamePool" <<std::endl;
        for(auto& thread1: FNamePoolSearchingThreads)
        {
          thread1.join();
        }
        if(EndSearching)
        {
       
          return true;
        }
    }
    else
    {
        memInfo1={};
        for (char *p = (char *)moduleInfo.lpBaseOfDll;
           p < (char *)moduleInfo.lpBaseOfDll + moduleInfo.SizeOfImage;
           p += memInfo1.RegionSize)
        {
          // Call VirtualQueryEx to get information about the memory region
          SIZE_T size = VirtualQueryEx(hProcess, p, &memInfo1, sizeof(memInfo1));
          if (size == 0)
          {
          
            std::cerr << "VirtualQueryEx failed" << std::endl;
            return false;
          }

          FName1SearchingThreads.push_back(std::thread(FindFNameMethod1,memInfo1));
        
        }
        
        std::cout << "Searching for GNames Using method 1" <<std::endl;
        for(auto& thread1: FName1SearchingThreads)
        {
          thread1.join();
        }
        if(EndSearching)
        {
          return true;
        }
      
        memInfo1={};
        for (char *p = (char *)moduleInfo.lpBaseOfDll;p < (char *)moduleInfo.lpBaseOfDll + moduleInfo.SizeOfImage;p += memInfo1.RegionSize)
        {
          // Call VirtualQueryEx to get information about the memory region
          SIZE_T size = VirtualQueryEx(hProcess, p, &memInfo1, sizeof(memInfo1));
          if (size == 0)
          {
          
            std::cerr << "VirtualQueryEx failed" << std::endl;
            return false;
          }

          FName2SearchingThreads.push_back(std::thread(FindFNameMethod2,memInfo1));
        
        }
        std::cout << "Searching for GNames Using method 2" <<std::endl;
        NumFound=0;
        for(auto& thread1: FName2SearchingThreads)
        {
          thread1.join();
        }
        if(NumFound==2)
        {
        
          return true;
        }
    }
    
     
    return false;
}

    
}
