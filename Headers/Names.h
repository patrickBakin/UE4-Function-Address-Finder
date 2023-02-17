#pragma once
#include <thread>
#include "Headers/NameClasses.h"
namespace UName
{
  int NumFound=0;
  bool EndSearching=false;
  bool FindFNameMethod1(MEMORY_BASIC_INFORMATION memInfo,ProfileGen& PG)
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
      if(int64_t GetNamesStrAddr = Algorithm::ScanforStringRef(buffer,L"Hardcoded name '%s' at index %i was duplicated (or unexpected concurrency). Existing entry is '%s'.",(int64_t)memInfo.BaseAddress,3,"FName::GetNames()"))
      {
        //std::cout << "Using FName::GetNames()"<<std::endl;
        int64_t BeforeCallGetName =Algorithm::ScanFor(GetNamesStrAddr,{0xBA,0x01,0x00,0x00,0x00,0x48,0x8B,0xC8,0xE8},true);
        if(BeforeCallGetName>=0)
        {
          int64_t CallGetNameFunction = Algorithm::ScanFor(BeforeCallGetName,{ 0xE8, 0xFF,0xFF,0xFF,0xFF,0xFF,0x8B},true);
          if(CallGetNameFunction>=0)
          {
            int indexE8 =CallGetNameFunction-(int64_t)memInfo.BaseAddress;
            int relative = *(int*)(&buffer[indexE8+1]);
            int64_t GetNameFunctionAddress = CallGetNameFunction+relative+4;
            if(int64_t NamesDataMov = Algorithm::ScanFor(GetNameFunctionAddress,{0x48,0x8B},false))
            {
              int indexMov = NamesDataMov-(int64_t)memInfo.BaseAddress;
              int relaTive = *(int*)(&buffer[indexMov+3]);
              int64_t NamesDataAddress = NamesDataMov+3 +relaTive+4;
              std::cout << "Found GName 0x"<<std::hex<< NamesDataAddress<<std::endl;;
              TNameEntryArray::Names = new TNameEntryArray(NamesDataAddress);
              PG.GetProfile().GNameOffset=NamesDataAddress-reinterpret_cast<int64_t>(BaseAddress);
              
              EndSearching=true;
              return true;
            }
                
          }
        }
            
      }
    }
    return false;
  }
  bool FindFNameMethod2(MEMORY_BASIC_INFORMATION memInfo,ProfileGen& PG)
  {
    std::size_t size = memInfo.RegionSize;
    std::vector<int64_t> buffer(size/8);
    SIZE_T bytesRead;
    
    //Read all memory in .exe module store every 8 bytes in buffer


    //REad fucking entire Region to buffer each element is 8 byte
   
    if (ReadProcessMemory(hProcess, memInfo.BaseAddress, buffer.data(), size, &bytesRead)/* && bytesRead == size*/)
    {
      //Loop the buffer
      for(int index=0;index<buffer.size();index++)
      {
        if(EndSearching)
        {
          return false;
        }
        //ptr2 is GNames->ptr1->ptr2
        int64_t ptr2 =buffer[index];
        if(int64_t name1 =Algorithm::ReadAs<int64_t>(ptr2) ? Algorithm::ReadAs<int64_t>(ptr2): 0)
        {
          if(int64_t NameIndexAddr = Algorithm::ReadAs<int64_t>(name1)? Algorithm::ReadAs<int64_t>(name1) : 0)
          {
                //+0x10 for locating the string
                NameIndexAddr=NameIndexAddr+0xC;
                size_t stringsize = 0x10;
                std::vector<byte> stringbuff(stringsize);
                size_t bRead;
                if(ReadProcessMemory(hProcess,(void*)NameIndexAddr,stringbuff.data(),stringsize,&bRead))
                {
                  for(int bi=0;bi<=4;bi++)
                  {
                    int checkvalue =*(int*)(&stringbuff[bi]);
                    if(checkvalue==0x656E6F4E)
                    {

                      int64_t GNamesAddr=(int64_t)memInfo.BaseAddress+index*8;
                      
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
                        PG.GetProfile().GNameOffset=GNamesAddr-reinterpret_cast<int64_t>(BaseAddress);
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

  bool FindFNamePool(MEMORY_BASIC_INFORMATION memInfo,ProfileGen& PG)
  {
    if ((memInfo.State & MEM_COMMIT) && memInfo.Protect != PAGE_NOACCESS)
      {
            // Get the size of the region
            std::size_t size = memInfo.RegionSize;
            std::vector<int64_t> buffer(size/8);

            // Read the contents of the region into the buffer
            SIZE_T bytesRead;
            int Num8Bytes;
            
            //ReadMemory In Buffer elem size = int64_t
            if (ReadProcessMemory(hProcess, memInfo.BaseAddress, buffer.data(), size, &bytesRead)/* && bytesRead == size*/)
            {

              Num8Bytes=(int32_t)bytesRead/8;

              for (int j = 0; j < Num8Bytes; j++)
              {
                //b is value of 8bytes 
                int64_t b = buffer[j];
             
              
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
                      //Means = None
                      if(checkvalue==0x656E6F4E)
                      {

                        //If address is in the module
                        if(b<(int64_t)hExeModule)
                        {
                          int64_t FNamePoolAddr= (int64_t)memInfo.BaseAddress + j*8-0x10;
                          std::cout  <<  "Found FNamePool 0x" << std::hex <<b << " <-- 0x" <<FNamePoolAddr<<std::endl;
                            
                            FNamePool::NamesData = new FNamePool(FNamePoolAddr); 
                            EndSearching=true;
                         
                            
                          PG.GetProfile().GNameOffset=FNamePoolAddr-reinterpret_cast<int64_t>(BaseAddress);
                          PG.GetProfile().UseFNamePool=true;
                          
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
  bool FindNames(ProfileGen& PG)
  {
  
    MEMORY_BASIC_INFORMATION memInfo1;
    std::vector<std::thread> FName1SearchingThreads;
    std::vector<std::thread> FName2SearchingThreads;
    
    if(PG.GetProfile().EngineVersion/100>=423)
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

          FNamePoolSearchingThreads.push_back(std::thread(FindFNamePool,memInfo1,std::ref(PG)));
        
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

          FName1SearchingThreads.push_back(std::thread(FindFNameMethod1,memInfo1,std::ref(PG)));
        
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

          FName2SearchingThreads.push_back(std::thread(FindFNameMethod2,memInfo1,std::ref(PG)));
        
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
