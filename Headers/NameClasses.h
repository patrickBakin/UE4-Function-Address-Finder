#pragma once

namespace UName
{
    
    class FNameEntry 
    {
        
    public:
        FNameEntry(int64_t address) : address(address){};
        int64_t address;
        
    private:
        int8_t IndexOffset=ProfileGen::GetProfile().IsUsing4_22? 0x8:0x0;//0x0; //4.16
        int8_t HasNextOFfset=ProfileGen::GetProfile().IsUsing4_22?0x0:0x8; //4.16
        int8_t AnsiNameOffset=ProfileGen::GetProfile().IsUsing4_22?0xC:0x10; //4.16
                
    public:

        operator bool() const
        {   
            int64_t buff;
            size_t BytesRead;
            return ReadProcessMemory(hProcess,(void*)address,&buff,8,&BytesRead);
        }
        int64_t GetHashNextValue()
        {
            
            if(address)
            {
                
                if(const auto HashNextPtr=Algorithm::ReadAs<int64_t>(address+HasNextOFfset))
                {
                    return Algorithm::ReadAs<int64_t>(HashNextPtr); // HasNextPtr value
                }
                
                
            }
            return 0;
        }
        int32_t GetIndexValue()
        {
            if(address)
            {
                int32_t Index=Algorithm::ReadAs<int32_t>(address+IndexOffset);
                return Index;
                
                
            }
            return 0;
        }
        std::string GetAnsiNameValue()
        {
            if(address)
            {
                char AnsiName[1024];
                size_t BytesRead;
                if(ReadProcessMemory(hProcess,(void*)(address+AnsiNameOffset),AnsiName,1024,&BytesRead))
                {
                        
                    return std::string(AnsiName);
                }
               
            }
         
            return "Not Found";
        }

    };
    class TNameEntryArray
    {

    public:
        TNameEntryArray(int64_t addr)
        {
            address = Algorithm::ReadAs<int64_t>(addr);

        };
        
        
        
        inline static TNameEntryArray* Names=nullptr;
    private:
        enum
        {
            ElementsPerChunk = 16 * 1024,
            ChunkTableSize = (2* 1024 * 1024 + ElementsPerChunk - 1) / ElementsPerChunk
        };
        int64_t address;
        int32_t NumElementsOffset=ProfileGen::GetProfile().IsUsing4_22 ? 4*ChunkTableSize: 8*ChunkTableSize;
        int32_t NumChunksOffset=NumElementsOffset+0x4;
    public:
        static TNameEntryArray* GetNames()
        {
            if(Names)
            {
                return Names;
            }
            return nullptr;
        }
        
        int32_t Num() const
        {
            if(address)
            {
                return Algorithm::ReadAs<int32_t>(address+NumElementsOffset);
                
            }
        }
        bool IsValidIndex(int Index)
        {
            return Index >=0 && Index < Num() ;/*&& GetById(Index);*/
        }
        FNameEntry GetById(int Index)
        {
            int64_t NameItem=GetItemPtr(Index); //6B90000
            int64_t ptr2 =Algorithm::ReadAs<int64_t>(NameItem);
          
            return ptr2;
        }

        int64_t GetItemPtr(int Index)
        {
            if(address)
            {
                
                    auto ChunkIndex = (Index / 16384);
                    auto WithinChunkIndex = (Index % 16384);
                    auto ChunkItem =address+8*ChunkIndex; //ChunkItem =6B80080
                    auto InChunk = Algorithm::ReadAs<int64_t>(ChunkItem)+8*WithinChunkIndex;
       
                return InChunk;
            }
            return 0x0;
        }

        void DumpName(std::ofstream& D,int& NameNum)
        {
            int i=0;
            int Num;
            while( GetItemPtr(i))
            {

            
                auto NameEntry = GetNames()->GetById(i);
                std::string name = NameEntry.GetAnsiNameValue();
                if(name!="Not Found")
                {
                    Num=GetNames()->GetById(i).GetIndexValue()/2;
                    D <<"["<<std::uppercase<<std::hex<<Num<<"] "<<name<<"\n";
                    
                    
                }
                i++;
            }
            NameNum=Num;
        }

        
    };



    //4.23++ Copy from UnrealDumper-4.25 Source https://github.com/guttir14/UnrealDumper-4.25
    class FNamePoolFNameEntry
    {
    public:
        FNamePoolFNameEntry(int64_t address) : address(address){};
        
    private:
        int64_t address;
        int8_t KeyOffset=0x0;
        int8_t AnsiNameOffset=0x2;
    public:
        uint32_t GetLength() const
        {
            
            auto smth=Algorithm::ReadAs<uint16_t>(address+KeyOffset) >> 6;
            return smth;
        }
        static uint16_t Size(bool wide,uint16_t len)
        {
            uint16_t bytes = 0x2+len*(wide?2:1);
            return (bytes+0x2-1u) & ~(2-1u);
        }
        bool IsWide() const
        {
            return Algorithm::ReadAs<uint32_t>(address+KeyOffset) & 1;
        }
        void String(char* buf,bool wide, uint16_t len) const
        {
            if (wide) {
                wchar_t wbuf[1024]{};
                ReadProcessMemory(hProcess,(void*)(address + 0x2), &wbuf, len * 2ull,NULL);
                /*if (Decrypt_WIDE) { Decrypt_WIDE(wbuf, len); }*/
                auto copied = WideCharToMultiByte(CP_UTF8, 0, wbuf, len, buf, len, 0, 0);
                if (copied == 0) {
                    buf[0] = '\x0';
                }
            } else {
                ReadProcessMemory(hProcess,(void*)(address + 0x2), buf, len,NULL);
             
            }
        }
        std::string GetAnsiName() const
        {
            auto len = GetLength();
            if (len > 1024) return "[Error: Overflow]";
            char AnsiName[1024];
        
            if(ReadProcessMemory(hProcess,(void*)(address+AnsiNameOffset),AnsiName,1024,NULL))
            {
                        
                return std::string(AnsiName);
            }
            
        }
        
        std::string GetName() const
        {
            return GetAnsiName();
        }
    };
    class FNameEntryHandle
    {
    public:
        
        uint32_t Block=0;
        uint32_t Offset=0;
        FNameEntryHandle(uint32_t block, uint32_t offset) : Block(block), Offset(offset){};
        FNameEntryHandle(uint32_t id) : Block(id >> 16), Offset(id & 65535){};
        operator uint32_t() const { return (Block << 16 | Offset); }
    };
    
    class FNameEntryAllocator
    {
    public:
        FNameEntryAllocator(int64_t address) : address(address){};
    private:
        int64_t address;
        int8_t frwLockOffset =0x0;
        int8_t CurrentBlockOffset =0x8;
        int8_t CurrentByteCursorOffset=0xC;
        int8_t BlocksOffset =0x10;
    public:
        uint32_t NumBlocks() const
        {
            return Algorithm::ReadAs<uint32_t>(address+CurrentBlockOffset)+1;
        }

        FNamePoolFNameEntry GetById(int32_t key) const
        {
            uint32_t block = key >> 16;
            uint16_t offset = (uint16_t)key;

            if (!IsValidIndex(key, block, offset))
                return (Algorithm::ReadAs<int64_t>(address+BlocksOffset) + 0); // "None"
           return Algorithm::ReadAs<int64_t>((address+BlocksOffset+8*block)  )+2*offset;
            
        }
        int64_t GetBlock(int32_t blockId)
        {
            return Algorithm::ReadAs<int64_t>((address+BlocksOffset+8*blockId)  );
        }

        uint32_t GetCurrentByteCursor()
        {
            return Algorithm::ReadAs<uint32_t>(address+CurrentBlockOffset);
        }
        
        bool IsValidIndex(int32_t key) const
        {
            uint32_t block = key >> 16;
            uint32_t offset = key;
            return IsValidIndex(key, block, offset);
        }

        bool IsValidIndex(int32_t key, uint32_t block, uint32_t offset) const
        {
            return (key >= 0 && block < NumBlocks() && offset * 2 < 0x1FFFE);
        }

   
    };
    
    class FNamePool
    {
    public:
        FNamePool(int64_t address) : address(address), FNameEntryAllocatorInstance(address+FNameEntryAllocatorOffset){};
        inline static FNamePool* NamesData=nullptr;
    private:
        int64_t address;
        int8_t FNameEntryAllocatorOffset=0x0;
        FNameEntryAllocator FNameEntryAllocatorInstance =NULL;
        
    public:
        FNamePoolFNameEntry GetById(int32_t key)
        {
           return FNameEntryAllocatorInstance.GetById(key);
        }
        FNamePoolFNameEntry operator[](int32_t id)
        {
            return FNameEntryAllocatorInstance.GetById(id);
        }
        void DumpBlock(uint32_t blockId,uint32_t blockSize,std::ofstream& D,int& NameNum)
        {
            int64_t it = FNameEntryAllocatorInstance.GetBlock(blockId);
            int64_t end = it+blockSize-2;
            FNameEntryHandle entry_handle = {blockId,0};
          
            while(it<end)
            {
                FNamePoolFNameEntry entry = FNamePoolFNameEntry(it);
                auto wide = entry.IsWide();
                auto len = entry.GetLength();
                if(len)
                {
                    char buf[1024];
                    entry.String(buf,wide,len);
                    D<<"["<<NameNum<<"]"<<std::string_view(buf,len)<<"\n";
                    uint16_t size = FNamePoolFNameEntry::Size(wide,len);
                    entry_handle.Offset+=size/0x2;
                    it+=size;
                    NameNum++;
                        
                }
                else
                {
                    break;
                }
            }
            
        }
        void DumpName(std::ofstream& D,int& NameNum)
        {
            for(uint32_t i=0;i<FNameEntryAllocatorInstance.NumBlocks()-1;i++)
            {
               DumpBlock(i,2*65536,D,NameNum);
            }
            DumpBlock(FNameEntryAllocatorInstance.NumBlocks()-1,FNameEntryAllocatorInstance.GetCurrentByteCursor(),D,NameNum);
            
        }
        
    };
}


