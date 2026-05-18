#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include <cwctype>

class WildcardMatcher
{
public:
    WildcardMatcher() = default;
    
    void AddPattern(const std::wstring& pattern);
    void AddPatterns(const std::vector<std::wstring>& patterns);
    bool Matches(const std::wstring& filename) const;
    bool HasPatterns() const { return !_patterns.empty(); }
    void Clear() { _patterns.clear(); }

private:
    bool MatchPattern(const std::wstring& filename, const std::wstring& pattern) const;
    
    std::vector<std::wstring> _patterns;
};