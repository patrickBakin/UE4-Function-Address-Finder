#pragma once
namespace World
{
    bool FindGWorld()
    {
        MEMORY_BASIC_INFORMATION memInfo;
        for (char *p = (char *)moduleInfo.lpBaseOfDll;
          p < (char *)moduleInfo.lpBaseOfDll + moduleInfo.SizeOfImage;
          p += memInfo.RegionSize)
        {
           
            if (VirtualQueryEx(hProcess, p, &memInfo, sizeof(memInfo)) )
            {
                size_t Size = memInfo.RegionSize;
                std::vector<BYTE> buffer(Size);
                if(ReadProcessMemory(hProcess,memInfo.BaseAddress,buffer.data(),Size,NULL))
                {
                    
                    int64_t GWorldLocIndex=Algorithm::searchArray(buffer,{0x0F,0x2E,0xFF,0x74,0xFF,0x48,0x8B,0x1D,0xFF,0xFF,0xFF,0xFF,0x48,0x85,0xDB,0x74},false);
                    
                    if(GWorldLocIndex !=-1)
                    {
                        int64_t GWorldLoc=reinterpret_cast<int64_t>(memInfo.BaseAddress)+GWorldLocIndex;
                      
                        auto relative = Algorithm::ReadAs<int32_t>(GWorldLoc+0x8);
                        auto GWorldAddress = GWorldLoc+relative+0xC;
                        std::cout <<"GWorld Address: 0x"<<std::hex << GWorldAddress<<std::endl;
                        ProfileGen::GetProfile().GWorldOffset=GWorldAddress-reinterpret_cast<int64_t>(BaseAddress);
                        return true;
                    }
                    
                    
                }
              
            }
        }

        memInfo={};
        for (char *p = (char *)moduleInfo.lpBaseOfDll;
          p < (char *)moduleInfo.lpBaseOfDll + moduleInfo.SizeOfImage;
          p += memInfo.RegionSize)
        {

      
            if (VirtualQueryEx(hProcess, p, &memInfo, sizeof(memInfo)) )
            {
                size_t Size = memInfo.RegionSize;
                std::vector<BYTE> buffer(Size);
                if(ReadProcessMemory(hProcess,memInfo.BaseAddress,buffer.data(),Size,NULL))
                {
                    auto StringAddr = Algorithm::ScanforStringRef(buffer,L"Failed to load package '%s' into a new game world.",reinterpret_cast<int64_t>(memInfo.BaseAddress),0x3,"Searching for GWorld");
                if(StringAddr)
                {
                    int64_t movGWorld=0x0,lastfound=0x0;;
                    movGWorld = Algorithm::ScanFor(StringAddr,{0x48,0x89,0x1D},false,300);
                    if(movGWorld!=-1)
                    {
                        std::cout << "Possibly a mov GWolrd,xxx GWord Address 0x"<<std::hex<<movGWorld<<std::endl;
                        lastfound=movGWorld;
                    }
                    movGWorld = Algorithm::ScanFor(StringAddr,{0x48,0x89,0x2D},false,300);
                    if(movGWorld!=-1)
                    {
                        std::cout << "Possibly a mov GWolrd,xxx Address 0x"<<std::hex<<movGWorld<<std::endl;
                        lastfound=movGWorld;
                    }
                    movGWorld = Algorithm::ScanFor(StringAddr,{0x4C,0x89,0x2D},false,300);
                    if(movGWorld!=-1)
                    {
                        std::cout << "Possibly a mov GWolrd,xxx GWord Address 0x"<<std::hex<<movGWorld<<std::endl;
                        lastfound=movGWorld;
                    }
                    movGWorld = Algorithm::ScanFor(StringAddr,{0x4C,0x89,0x1D},false,300);
                    if(movGWorld!=-1)
                    {
                        std::cout << "Possibly a mov GWolrd,xxx GWord Address 0x"<<std::hex<<movGWorld<<std::endl;
                        lastfound=movGWorld;
                    }
                    movGWorld = Algorithm::ScanFor(StringAddr,{0x48,0x89,0x15},false,300);
                    if(movGWorld!=-1)
                    {
                        std::cout << "Possibly a mov GWolrd,xxx GWord Address 0x"<<std::hex<<movGWorld<<std::endl;
                        lastfound=movGWorld;
                    }
                    if(lastfound>0)
                    {
                        auto relative =Algorithm::ReadAs<int32_t>(lastfound+0x3);
                        auto GWorldAddr = lastfound+relative+0x7;
                        std::cout <<"GWorld Address: 0x"<<std::hex<<GWorldAddr<<std::endl;
                        ProfileGen::GetProfile().GWorldOffset=GWorldAddr-reinterpret_cast<int64_t>(BaseAddress);
                        return true;
                    }
                    std::cout<<"GWorld Not Found"<<std::endl;
                    return false;
                    
             
                }
                }
                
            }
            
        }
        return false;
    }
}
