#pragma once
#include "Headers/mini/ini.h"
class ProfileGen
{
public:
    struct Profile
    {
        int32_t EngineVersion=0;
        bool IsUsing4_22=false;
        bool UseFNamePool=false;
        bool IsUsingFChunkedFixedUObjectArray=false;
        int64_t GNameOffset=0x0;
        int64_t GObjectOffset=0x0;
        int64_t GWorldOffset=0x0;

        int64_t GameStateInitOffset=0x0;
        int64_t BeginPlayOffset =0x0;
        int64_t StaticLoadObjectOffset =0x0;
        int64_t SpawnActorFTransOffset=0x0;
        int64_t CallFunctionByNameWithArgumentsOffset=0x0;
        int64_t ProcessEventOffset =0x0;
    };
   
    ProfileGen(const std::string& ProfileName)
    {
       profileName=ProfileName+".profile";
    }

    static Profile& GetProfile()
    {
        if(P==nullptr)
        {
            P= new Profile;
            return *P;
        }
        return *P;
    }
    std::string DecToStringHex(int64_t Dec)
    {
        std::stringstream ss;
        ss<<std::hex<<Dec;
        return "0x"+ss.str();
    }
    void GenProfile()
    {
        mINI::INIFile file(profileName);
        mINI::INIStructure ini;
        ini["GameInfo"]["UsesFNamePool"]=P->UseFNamePool ? "1":"0";
        ini["GameInfo"]["IsUsingFChunkedFixedUObjectArray"]=P->IsUsingFChunkedFixedUObjectArray ? "1":"0";
        ini["GameInfo"]["IsUsing4_22"]=P->IsUsing4_22 ? "1":"0";
        ini["GInfo"]["IsGInfoPatterns"]="0";
        ini["GInfo"]["GName"]=DecToStringHex(P->GNameOffset);
        ini["GInfo"]["GObject"]=DecToStringHex(P->GObjectOffset);
        ini["GInfo"]["GWorld"]=DecToStringHex(P->GWorldOffset);
        ini["FunctionInfo"]["IsFunctionPatterns"]="0";
        
        ini["FunctionInfo"]["GameStateInit"]=DecToStringHex(P->GameStateInitOffset);
        ini["FunctionInfo"]["BeginPlay"]=DecToStringHex(P->BeginPlayOffset);
        ini["FunctionInfo"]["StaticLoadObject"]=DecToStringHex(P->StaticLoadObjectOffset);
        ini["FunctionInfo"]["SpawnActorFTrans"]=DecToStringHex(P->SpawnActorFTransOffset);
        ini["FunctionInfo"]["CallFunctionByNameWithArguments"]=DecToStringHex(P->CallFunctionByNameWithArgumentsOffset);
        ini["FunctionInfo"]["ProcessEvent"]=DecToStringHex(P->ProcessEventOffset);
        
        file.generate(ini);
        std::cout<<"Profile saved as "<< profileName<< " in game binaries folder "<<std::endl;
        
    }
private:
     
    inline static Profile* P=nullptr;
    std::string profileName;
};