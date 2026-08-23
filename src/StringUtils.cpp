#include "StringUtils.h"
#include <cctype>
#include <algorithm>

namespace StringUtils{

std::string Slice(const std::string &str, ssize_t start, ssize_t end) noexcept{
    size_t len = str.length();
    
    // Handle negative start (count from end)
    if(start < 0){
        start = len + start;
        if(start < 0) start = 0;
    }
    
    // Handle negative end (count from end)
    if(end < 0){
        end = len + end;
        if(end < 0) end = 0;
    }
    
    // Handle end == 0 (means end of string)
    if(end == 0){
        end = len;
    }
    
    // Ensure start and end are within bounds
    if(start > len) start = len;
    if(end > len) end = len;
    if(start > end) return "";
    
    return str.substr(start, end - start);
}

std::string Capitalize(const std::string &str) noexcept{    
    std::string Temp = str;
    if(!Temp.empty()){
        Temp[0] = toupper(Temp[0]);
        for(size_t Index = 1; Index < Temp.length(); Index++){
            Temp[Index] = tolower(Temp[Index]);
        }
    }
    return Temp;
}

std::string Upper(const std::string &str) noexcept{
    std::string result = str;
    for(size_t i = 0; i < result.length(); i++){
        result[i] = toupper(result[i]);
    }
    return result;
}

std::string Lower(const std::string &str) noexcept{
    std::string result = str;
    for(size_t i = 0; i < result.length(); i++){
        result[i] = tolower(result[i]);
    }
    return result;
}

std::string LStrip(const std::string &str) noexcept{
    std::string Temp = str;
    size_t Index = 0;
    size_t Length = Temp.length();
    while((Index < Length) && isspace(Temp[Index])){
        Index++;
    }

    return Temp.substr(Index);
}

std::string RStrip(const std::string &str) noexcept{
    std::string Temp = str;
    if(!Temp.empty()){
        size_t Index = Temp.length()-1;
        while(isspace(Temp[Index])){
            if(!Index){
                return "";
            }
            Index--;
        }
        return Temp.substr(0,Index+1);
    }
    return "";
}

std::string Strip(const std::string &str) noexcept{
    return LStrip(RStrip(str));
}

std::string Center(const std::string &str, int width, char fill) noexcept{
    size_t strLen = str.length();
    
    // If string is already wider than width, return as-is
    if(strLen >= static_cast<size_t>(width)){
        return str;
    }
    
    // Calculate padding needed
    int totalPadding = width - strLen;
    int leftPadding = totalPadding / 2;
    int rightPadding = totalPadding - leftPadding;
    
    // result = left padding + string + right padding
    std::string result;
    result.append(leftPadding, fill);
    result += str;
    result.append(rightPadding, fill);
    
    return result;
}

std::string LJust(const std::string &str, int width, char fill) noexcept{
    size_t strLen = str.length();
    
    // If string is already wider than width, return as-is
    if(strLen >= static_cast<size_t>(width)){
        return str;
    }
    
    // Calculate padding needed on the right
    int rightPadding = width - strLen;
    
    // Build result: string + right padding
    std::string result = str;
    result.append(rightPadding, fill);
    
    return result;
}

std::string RJust(const std::string &str, int width, char fill) noexcept{
    size_t strLen = str.length();
    
    // If string is already wider than width, return as-is
    if(strLen >= static_cast<size_t>(width)){
        return str;
    }
    
    // Calculate padding needed on the left
    int leftPadding = width - strLen;
    
    // result = left padding + string
    std::string result;
    result.append(leftPadding, fill);
    result += str;
    return result;
}

std::string Replace(const std::string &str, const std::string &old, const std::string &rep) noexcept{
    // If the substring to replace is empty, return the original string
    if(old.empty()){
        return str;
    }

    std::string result;
    size_t pos = 0;
    size_t matchPos;

    // Find each occurrence of 'old' and build the result string
    while((matchPos = str.find(old, pos)) != std::string::npos){
        // Append the part before the match
        result.append(str, pos, matchPos - pos);
        // Append the replacement
        result += rep;
        // Move past the matched substring
        pos = matchPos + old.length();
    }

    // Append any remaining part of the original string
    result.append(str, pos, str.length() - pos);

    return result;
}

std::vector< std::string > Split(const std::string &str, const std::string &splt) noexcept{
    std::vector<std::string> result;
    
    // If splt is empty, split on whitespace
    if(splt.empty()){
        size_t start = 0;
        size_t end = 0;
        
        while(start < str.length()){
            // Skip leading whitespace
            while(start < str.length() && isspace(str[start])){
                start++;
            }
            
            if(start >= str.length()){
                break;
            }
            
            // Find end of current word
            end = start;
            while(end < str.length() && !isspace(str[end])){
                end++;
            }
            
            // Add word to result
            result.push_back(str.substr(start, end - start));
            start = end;
        }
    } else {
        // Split on the splt string
        size_t pos = 0;
        size_t found;
        
        while((found = str.find(splt, pos)) != std::string::npos){
            result.push_back(str.substr(pos, found - pos));
            pos = found + splt.length();
        }
        
        // Add the last part
        result.push_back(str.substr(pos));
    }
    
    return result;
}

std::string Join(const std::string &str, const std::vector< std::string > &vect) noexcept{
    if(vect.empty()){
        return "";
    }
    
    std::string result = vect[0];
    
    for(size_t i = 1; i < vect.size(); i++){
        result += str;
        result += vect[i];
    }
    
    return result;
}

std::string ExpandTabs(const std::string &str, int tabsize) noexcept {
    if (tabsize <= 0) return str;  // avoid divide/mod by zero

    std::string result;
    int column = 0;

    for (size_t i = 0; i < str.length(); i++) {
        char c = str[i];
        if (c == '\t') {
            int spacesNeeded = tabsize - (column % tabsize);
            result.append(spacesNeeded, ' ');
            column += spacesNeeded;
        } else {
            result += c;
            if (c == '\n' || c == '\r') column = 0;
            else column++;
        }
    }
    return result;
}


int EditDistance(const std::string &left, const std::string &right, bool ignorecase) noexcept{
    size_t m = left.length();
    size_t n = right.length();
    
    // Create DP table
    std::vector<std::vector<int>> dp(m + 1, std::vector<int>(n + 1, 0));
    
    // Base cases: empty strings
    for(size_t i = 0; i <= m; i++){
        dp[i][0] = i;  // i deletions needed
    }
    for(size_t j = 0; j <= n; j++){
        dp[0][j] = j;  // j insertions needed
    }
    
    // Fill DP table
    for(size_t i = 1; i <= m; i++){
        for(size_t j = 1; j <= n; j++){
            // Check if characters match
            char leftChar = left[i - 1];
            char rightChar = right[j - 1];
            
            bool charsMatch;
            if(ignorecase){
                charsMatch = (tolower(leftChar) == tolower(rightChar));
            } else {
                charsMatch = (leftChar == rightChar);
            }
            
            if(charsMatch){
                // Characters match, no cost
                dp[i][j] = dp[i - 1][j - 1];
            } else {
                
                //  Deletion: dp[i-1][j] + 1
                //  Insertion: dp[i][j-1] + 1
                //  Substitution: dp[i-1][j-1] + 1
                dp[i][j] = 1 + std::min({dp[i - 1][j],      // deletion
                                         dp[i][j - 1],      // insertion
                                         dp[i - 1][j - 1]}); // substitution
            }
        }
    }
    
    return dp[m][n];
}

};