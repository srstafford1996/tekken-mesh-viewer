#pragma once
#include <string>

inline void rtrim(std::string &s)
{
    for (int i = s.size() - 1; i >= 0; i--)
    {
        if (s[i] != 0 && s[i] != '\n' && s[i] != '\r')
        {
            if (i != s.size() - 1) s.erase(i + 1);
            break;
        }
    }
}