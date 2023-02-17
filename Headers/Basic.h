#pragma once
namespace Basic
{
    template<class T>
class TArray
    {
    public:
        T* Data;
        int Count;
        int Max;
    };

    class FString : public TArray<wchar_t>
    {
    public:

        inline const wchar_t* c_str() const
        {
            return Data;
        }

        std::string ToString() const
        {
            const auto length = std::wcslen(Data);

            std::string str(length, '\0');

            std::use_facet<std::ctype<wchar_t>>(std::locale()).narrow(Data, Data + length, '?', &str[0]);

            return str;
        }
    };

}
