#pragma once
#include "aLgorithm.h"

namespace FunctionFinder
{
    class FindFunctions
    {
    public:
        bool static GetFunctions()
        {
            MEMORY_BASIC_INFORMATION memInfo;
            bool LoadFound=false,SpawnFound=false,
            CallFuncFound=false,InitGameFound=false,ActorBeginFound=false,ProcessEventFound=false;
            for (char *p = (char *)moduleInfo.lpBaseOfDll;p < (char *)moduleInfo.lpBaseOfDll + moduleInfo.SizeOfImage;p += memInfo.RegionSize)
            {
                SIZE_T size = VirtualQueryEx(hProcess, p, &memInfo, sizeof(memInfo));
                if (size == 0)
                {
        
                    std::cerr << "VirtualQueryEx failed" << std::endl;
                    continue;
                }
                auto RegSize = memInfo.RegionSize;
                std::vector<BYTE> buffer(RegSize); //buffer size is region size Store in Bytes
                if (ReadProcessMemory(hProcess, memInfo.BaseAddress, buffer.data(), RegSize, NULL)/* && bytesRead == size*/)
                {
                    if( !LoadFound &&  GetStaticLoadObjectAddress(buffer,memInfo.BaseAddress))
                    {
                        LoadFound=true;
                    }
      
                    if(!SpawnFound && GetSpawnActorAddress(buffer,memInfo.BaseAddress)  )
                    {
                        SpawnFound=true;
                    }
                    if(!CallFuncFound && GetCallFunctionByNameWithArgumentsAddress(buffer,memInfo.BaseAddress) )
                    {
                        CallFuncFound=true;
                    }
                    if(!InitGameFound &&GetInitGameStateAddress(buffer,memInfo.BaseAddress) )
                    {
                        InitGameFound=true;
                    }
                    if(!ActorBeginFound && GetActorBeginPlayAddress(buffer,memInfo.BaseAddress)  )
                    {
                        ActorBeginFound=true;
                    }
                    if(!ProcessEventFound &&GetProcessEventAddress(buffer,memInfo.BaseAddress) )
                    {
                        ProcessEventFound=true;
                    }
                    
                    
                }
                
            }
            if(LoadFound && SpawnFound && CallFuncFound && InitGameFound && ActorBeginFound && ProcessEventFound)
            {
                return true;
            }
            return false;
        }
    private:
        int64_t static GetStaticLoadObjectAddress(std::vector<BYTE>& buffer,void* Address)
        {   
            if(auto StringAddr=Algorithm::ScanforStringRef(buffer,L"Failed to find object '{ClassName} {OuterName}.{ObjectName}'",reinterpret_cast<int64_t>(Address),0x3,"String Ref UObject::StaticLoadObject()"))
            {
                auto pushStaticLoad =Algorithm::ScanFor(StringAddr,{/*0xCC,*/0x41,0x56,0x41,0x57,0x48},true);
                auto StaticLoadObjectAddr = Algorithm::ScanFor(pushStaticLoad,{0x40,0x55},true,12);
                std::cout << "StaticLoadObject: 0x"<<std::hex<<StaticLoadObjectAddr<<std::endl;
                ProfileGen::GetProfile().StaticLoadObjectOffset=StaticLoadObjectAddr-reinterpret_cast<int64_t>(BaseAddress);
                return StaticLoadObjectAddr;
            }
            
            return 0;
        }
        int64_t static GetSpawnActorAddress(std::vector<BYTE>& buffer ,void* Address)
        {
            if(auto StringAddr = Algorithm::ScanforStringRef(buffer,L"SpawnActor failed.",reinterpret_cast<int64_t>(Address),0x3,"String Ref UWorld::SpawnActor(UClass *,FTransform*..."))
            {
                
                
                auto CallFunctionSpawnActor = Algorithm::ScanFor(StringAddr,{0xE8},true,20);
                auto relative = Algorithm::ReadAs<int32_t>(CallFunctionSpawnActor+1);
                auto FunctionSpawnactorAddr= CallFunctionSpawnActor+relative+0x5;
                if(ProfileGen::GetProfile().EngineVersion/100<=408)
                {   
                    std::cout <<"Version is Too Old! Using SpawnActorFVector instead\n"<< "SpawnActorFVector: 0x"<<std::hex<<FunctionSpawnactorAddr<<std::endl;
                    ProfileGen::GetProfile().SpawnActorFTransOffset=FunctionSpawnactorAddr-reinterpret_cast<int64_t>(BaseAddress);
                    return FunctionSpawnactorAddr;
                }
                Algorithm::CheckNSkipJump(FunctionSpawnactorAddr,FunctionSpawnactorAddr);
                auto retnAddr = Algorithm::ScanFor(FunctionSpawnactorAddr,{0x5B,0xC3},false,192);
                auto CallFunctionSpawnActorTransform = Algorithm::ScanFor(retnAddr,{0xE8,0xFF,0xFF,0xFF,0xFF,0x48,0xFF,0xFF,0xFF,0xFF,0x48},true,30);
                auto relative1 = Algorithm::ReadAs<int32_t>(CallFunctionSpawnActorTransform+1);
                auto FunctionSpawnActorTransformAddr = CallFunctionSpawnActorTransform+relative1+0x5;
                Algorithm::CheckNSkipJump(FunctionSpawnActorTransformAddr,FunctionSpawnActorTransformAddr);
                std::cout << "SpawnActorFTransform: 0x"<<std::hex<<FunctionSpawnActorTransformAddr<<std::endl;
                ProfileGen::GetProfile().SpawnActorFTransOffset=FunctionSpawnActorTransformAddr-reinterpret_cast<int64_t>(BaseAddress);
                return FunctionSpawnActorTransformAddr;
                
            }
            return 0;
        }
        
        int64_t static GetCallFunctionByNameWithArgumentsAddress(std::vector<BYTE>& buffer,void* Address)
        {
            if(auto StringAddr = Algorithm::ScanforStringRef(buffer,L"'{Message}': Bad or missing property '{PropertyName}'",reinterpret_cast<int64_t>(Address),0x3,"String Ref UObject::CallFunctionByNameWithArguments()"))
            {
                auto pushCallFunctionByNameWithArguments = Algorithm::ScanFor(StringAddr,{0x41,0x56,0x41,0x57,0x48},true,3500);
                auto CallFunctionByNameWithArgumentsAddr = Algorithm::ScanFor(pushCallFunctionByNameWithArguments,{0x40,0x55},true,0x40);
                std::cout << "CallFunctionByNameWithArguments: 0x"<<std::hex<<CallFunctionByNameWithArgumentsAddr<<std::endl;
                ProfileGen::GetProfile().CallFunctionByNameWithArgumentsOffset=CallFunctionByNameWithArgumentsAddr-reinterpret_cast<int64_t>(BaseAddress);
                return CallFunctionByNameWithArgumentsAddr;
            }
            return 0;
            
            
        }
        int64_t static GetInitGameStateAddress(std::vector<BYTE>& buffer,void* Address)
        {
            auto Begin=Algorithm::searchArray(buffer,{0x40,0x53,0x48,0x83,0xEC,0x20,0x48,0x8B,0x41,0x10,0x48,0x8B,0xD9,0x48,0x8B,0x91},false);
            if(Begin!=-1)
            {
                
                auto InitGameStateFunctionAddr = reinterpret_cast<int64_t>(Address)+Begin;
                std::cout<< "InitGameState: 0x"<<std::hex<<InitGameStateFunctionAddr<<std::endl;
                ProfileGen::GetProfile().GameStateInitOffset=InitGameStateFunctionAddr-reinterpret_cast<int64_t>(BaseAddress);
                return InitGameStateFunctionAddr;
            }

            //Need to find a better pattern or stringref?
            Begin =Algorithm::searchArray(buffer,{0x48,0xFF, 0xFF, 0xFF ,0x90 ,0xFF,0xFF,0xFF, 0xFF, 0x48,0x8B, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x48 ,0x8B ,0xFF, 0xFF, 0xFF, 0xFF, 0xFF,0x48, 0x89 ,0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x48 ,0x8B},false);
            
            if(Begin!=-1)
            {
                auto movInitGameStateAddrr = reinterpret_cast<int64_t>(Address)+Begin;
                auto InitGameStateFunctionAddr = Algorithm::ScanFor(movInitGameStateAddrr,{0x40,0x53},true,100);
                std::cout << "InitGameState: 0x"<<std::hex<<InitGameStateFunctionAddr<<std::endl;
                ProfileGen::GetProfile().GameStateInitOffset=InitGameStateFunctionAddr-reinterpret_cast<int64_t>(BaseAddress);
                return InitGameStateFunctionAddr;
            }
            
            return 0;
        }
        int64_t static GetActorBeginPlayAddress(std::vector<BYTE>& buffer,void* Address)
        {
            auto Begin =Algorithm::searchArray(buffer,{0x48,0x8B,0xD9,0xE8,0xFF,0xFF ,0xFF , 0xFF , 0xF6 ,0x83 ,0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 0x74,0x12,0x48,0x8B ,0x03},false);
            if(Begin!=-1)
            {
                auto CallBeginPlay = reinterpret_cast<int64_t>(Address)+Begin+0x3;
        
                auto ActorBeginPlayFuncAddr = CallBeginPlay+Algorithm::ReadAs<int32_t>(CallBeginPlay+0x1)+0x5;
                Algorithm::CheckNSkipJump(ActorBeginPlayFuncAddr,ActorBeginPlayFuncAddr);
                std::cout << "ActorBeginPlay: 0x"<<std::hex<<ActorBeginPlayFuncAddr<<std::endl;
                ProfileGen::GetProfile().BeginPlayOffset=ActorBeginPlayFuncAddr-reinterpret_cast<int64_t>(BaseAddress);
                return ActorBeginPlayFuncAddr;
            }
           return 0;
        }
        int64_t static GetProcessEventAddress(std::vector<BYTE>& buffer,void* Address)
        {
            auto Begin=Algorithm::searchArray(buffer,{0x75,0x0E,0xFF,0xFF, 0xFF, 0x48,0xFF,0xFF, 0x48 ,0xFF, 0xFF, 0xE8 ,0xFF, 0xFF, 0xFF, 0xFF, 0x48,0x8B,0xFF, 0x24,0xFF, 0x48 ,0x8B ,0xFF, 0x24,0x38, 0x48, 0x8B, 0xFF, 0x24, 0x40},false);
            if(Begin!=-1)
            {
                auto MovB4ProcessEvent = reinterpret_cast<int64_t>(Address)+Begin;
                auto CallProcessEvent = Algorithm::ScanFor(MovB4ProcessEvent,{0xE8},false,16);
                auto ProcessEventFuncAddr = CallProcessEvent + Algorithm::ReadAs<int32_t>(CallProcessEvent+0x1)+0x5;
                Algorithm::CheckNSkipJump(ProcessEventFuncAddr,ProcessEventFuncAddr);
                std::cout << "ProcessEvent: 0x"<<std::hex<<ProcessEventFuncAddr<<std::endl;
                ProfileGen::GetProfile().ProcessEventOffset=ProcessEventFuncAddr-reinterpret_cast<int64_t>(BaseAddress);
                return ProcessEventFuncAddr;
            }
            return 0;
        }
       
    };
    
    
    
}
