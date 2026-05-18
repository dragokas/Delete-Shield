#include "wildcard_matcher.h"

void WildcardMatcher::AddPattern(const std::wstring& pattern)
{
    if (!pattern.empty())
        _patterns.push_back(pattern);
}

void WildcardMatcher::AddPatterns(const std::vector<std::wstring>& patterns)
{
    for (const auto& pattern : patterns)
        AddPattern(pattern);
}

bool WildcardMatcher::Matches(const std::wstring& filename) const
{
    if (_patterns.empty())
        return true;
        
    for (const auto& pattern : _patterns)
        if (MatchPattern(filename, pattern))
            return true;
            
    return false;
}

bool WildcardMatcher::MatchPattern(const std::wstring& filename, const std::wstring& pattern) const
{
    std::wstring f = filename;
    std::wstring p = pattern;
    
    // Case-insensitive for Windows
    std::transform(f.begin(), f.end(), f.begin(), ::towlower);
    std::transform(p.begin(), p.end(), p.begin(), ::towlower);
    
    size_t fpos = 0, ppos = 0;
    size_t fstar = std::wstring::npos, pstar = std::wstring::npos;
    
    while (fpos < f.length())
    {
        if (ppos < p.length() && (p[ppos] == L'?' || p[ppos] == f[fpos]))
        {
            fpos++;
            ppos++;
        }
        else if (ppos < p.length() && p[ppos] == L'*')
        {
            pstar = ppos;
            fstar = fpos;
            ppos++;
        }
        else if (pstar != std::wstring::npos)
        {
            ppos = pstar + 1;
            fstar++;
            fpos = fstar;
        }
        else
        {
            return false;
        }
    }
    
    while (ppos < p.length() && p[ppos] == L'*')
    {
        ppos++;
    }
    
    return ppos == p.length();
}