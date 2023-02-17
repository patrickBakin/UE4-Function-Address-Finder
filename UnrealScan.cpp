#include "header.h"



HMODULE hExeModule;
void* BaseAddress=nullptr;
HANDLE hProcess=NULL;
MODULEINFO moduleInfo;


int main() {
  DWORD processId = 0; // Replace this with the process ID you want to check
  std::cout << "Written By PotatoPie\n"<<"PID: "<<std::endl;
  std::cin >> processId;
  hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
  if (hProcess == NULL) {
    std::cout << "Failed to open process. Error code: " << GetLastError() << std::endl;
    return 1;
  }

  

  // Get the size of the module array
  DWORD cbNeeded;
  if (!EnumProcessModules(hProcess, NULL, 0, &cbNeeded)) {
    std::cout << "Failed to get module array size. Error code: " << GetLastError() << std::endl;

    CloseHandle(hProcess);
    return 1;
  }

  // Allocate memory for the module array
  HMODULE* hMods = new HMODULE[cbNeeded / sizeof(HMODULE)];
  if (hMods == NULL) {
    std::cout << "Failed to allocate memory for module array." << std::endl;
    
    CloseHandle(hProcess);
    return 1;
  }

  // Get the array of modules
  if (!EnumProcessModules(hProcess, hMods, cbNeeded, &cbNeeded)) {
    std::cout << "Failed to get module array. Error code: " << GetLastError() << std::endl;
    delete[] hMods;
    
    CloseHandle(hProcess);
    return 1;
  }

  // Find the .exe module
  hExeModule = NULL;
  TCHAR szModuleName[MAX_PATH];
  for (unsigned int i = 0; i < cbNeeded / sizeof(HMODULE); i++) {
    if (GetModuleFileNameEx(hProcess, hMods[i], szModuleName, sizeof(szModuleName) / sizeof(TCHAR))) {
      if (_tcsstr(szModuleName, _T(".exe"))) {
        hExeModule = hMods[i];
        break;
      }
    }
  }
  if(hExeModule)
  {
    BaseAddress = hExeModule;
    std::cout<<"Base Address: 0x"<<std::hex<<BaseAddress<<std::endl;
    //MEMORY_BASIC_INFORMATION memInfo;
    
    GetModuleInformation(hProcess,hExeModule,&moduleInfo,sizeof(moduleInfo));
    std::wstring wExeName(szModuleName);
    std::string ExeName(wExeName.begin(),wExeName.end());
    std::string Name=ExeName.substr(0,ExeName.size()-4);
    ProfileGen PG(Name);
    if(Util::GetEngineCurrent(PG))
    {
      
      
      
      
      
      if(UName::FindNames(PG))
      {
          std::cout<<"UsingFNamePool: "<<PG.GetProfile().UseFNamePool<<"\nDumping Names..."<<std::endl;
         
          std::ofstream  Dump("DumpName.txt");
         int NameNum=0;
         if(!PG.GetProfile().UseFNamePool)
         {
             UName::TNameEntryArray::Names->DumpName(Dump,NameNum);
         }
         else
         {
           
          UName::FNamePool::NamesData->DumpName(Dump,NameNum);
         }
          
          Dump.close();
         
          std::cout<<"Dumping "<<std::dec<<NameNum<<" Names \n"<<"Saving Dump as DumpName.txt" <<std::endl;
      }
      
      if(FunctionFinder::FindFunctions::GetFunctions(PG))
      {
        
      }
      if(Object::FindGObject(PG))
        {
          
        }
      if(World::FindGWorld(PG))
      {
        
      }
      PG.GenProfile();
      
      
    }
  }
    
    
  

  delete[] hMods;
 
  CloseHandle(hProcess);
  system("pause");
  return 0;
}