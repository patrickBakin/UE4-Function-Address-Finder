#pragma once
namespace Object
{
    bool FindGObject(ProfileGen& PG)
    {
        MEMORY_BASIC_INFORMATION memInfo;

        auto StartAddress = reinterpret_cast<int64_t>(BaseAddress);
        while (VirtualQueryEx(hProcess, (void*)StartAddress, &memInfo, sizeof(memInfo)) == sizeof(memInfo))
        {
            if ((memInfo.State & MEM_COMMIT) && memInfo.Protect != PAGE_NOACCESS)
            {
                
                std::size_t size = memInfo.RegionSize;
           
                std::vector<BYTE> buffer(size); 
                size_t bytesRead;
                if(ReadProcessMemory(hProcess, memInfo.BaseAddress, buffer.data(), size, &bytesRead))
                {
                    int foundFirstInstructionIndex=Algorithm::searchArray(buffer,{0x8B ,0x46 ,0x10 ,0x3B ,0x46 ,0x3C,0x75 ,0x0F ,0x48 ,0x8B ,0xD6 ,0x48 ,0x8D ,0x0D ,0xFF ,0xFF, 0xFF ,0xFF ,0xE8},false);
                    if(foundFirstInstructionIndex!=-1)
                    {
                        int relative=*(int32_t*)(&buffer[foundFirstInstructionIndex+14]);
                        int64_t GUObjectArray = (int64_t)StartAddress +foundFirstInstructionIndex+14+relative+4;
                        std::cout <<"Found GObject: 0x"<<std::hex<<GUObjectArray<<std::endl;
                        PG.GetProfile().GObjectOffset=GUObjectArray-reinterpret_cast<int64_t>(BaseAddress);
                        return true;
                    }
                }
               
            }
            StartAddress = StartAddress + memInfo.RegionSize;
        }
        
        if(PG.GetProfile().SpawnActorFTransOffset!=0x0)
        {
            auto SpawnActorFTransAddr = reinterpret_cast<int64_t>(BaseAddress)+ PG.GetProfile().SpawnActorFTransOffset;
            auto cmpJge= Algorithm::ScanFor(SpawnActorFTransAddr,{0x3B,0x05,0xFF,0xFF,0xFF,0xFF,0x7D,0xFF},false);
            auto leaMov = Algorithm::ScanFor(cmpJge,{0x48,0x8D,0xFF,0x40,0x48,0x8B,0x05,0xFF,0xFF,0xFF,0xFF},false,50)+0x4;
            auto relative1 = Algorithm::ReadAs<int32_t>(cmpJge+0x2);
            auto relative2 = Algorithm::ReadAs<int32_t>(leaMov+0x3);
            auto GUObjectNumElem= cmpJge+relative1+0x6;
            auto GUObjectFlags=leaMov+relative2+0x7;
            auto offset = GUObjectNumElem-GUObjectFlags;
            auto GObjectAddress=GUObjectFlags-0x10;
            if(offset ==0x14)
            {
                std::cout<<"Using FChunkedFixedUObjectArray"<<std::endl;
                PG.GetProfile().IsUsingFChunkedFixedUObjectArray=true;
                
                std::cout<<"Found GObject: 0x"<<GObjectAddress<<std::endl;
                PG.GetProfile().GObjectOffset= GObjectAddress- reinterpret_cast<int64_t>(BaseAddress);
                return true;
            }
            if(offset ==0xC)
            {
                std::cout<<"Not Using FChunkedFixedUObjectArray"<<std::endl;
                PG.GetProfile().IsUsingFChunkedFixedUObjectArray=false;
                std::cout<<"Found GObject: 0x"<<GObjectAddress<<std::endl;
                PG.GetProfile().GObjectOffset= GObjectAddress- reinterpret_cast<int64_t>(BaseAddress);
                return true;
            }

            std::cout<<"Either FUObjectItem or UObjectArray has uncommon members alignment"<<std::endl;
            std::cout <<"FUObject::NumElements 0x"<<std::hex<<GUObjectNumElem<<std::endl;
            std::cout <<"FUObject::Objects 0x"<<std::hex<<GUObjectFlags<<std::endl;
            
            
        }
        return false;
    }
}
