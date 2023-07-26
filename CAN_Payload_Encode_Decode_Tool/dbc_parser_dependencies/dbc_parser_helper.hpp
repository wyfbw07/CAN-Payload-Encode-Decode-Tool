//
//  dbc_parser_helper.hpp
//  CAN_Payload_Encode_Decode_Tool
//
//  Created by Yifan Wang on 7/19/23.
//

#ifndef dbc_parser_helper_h
#define dbc_parser_helper_h

#include <algorithm>
#include <sstream>
#include <string>

// Template of variables that can be set only once
template <typename T, typename Counter = unsigned char>
class SetOnce {
public:
    SetOnce(const T& initval = T(), const Counter& initcount = 1):
        val(initval), counter(initcount) {}
    SetOnce(const SetOnce&) = default;
    SetOnce<T, Counter>& operator=(const T& newval) {
        if (counter) {
            --counter;
            val = newval;
            return *this;
        }
        else throw "Variable can't be set more than once.";
    }
    operator const T&() const { return val; } // "getter"
protected:
    T val;
    Counter counter;
};

namespace utils {

    // A special version of trim function that removes any leading and trailling white spaces
    // and also removes all occurrences of new line and tab characters
    inline std::string& trim(std::string& str) {
        str.erase(str.find_last_not_of(' ') + 1);   //suffixing spaces
        str.erase(0, str.find_first_not_of(' '));   //prefixing spaces
        str.erase(remove(str.begin(), str.end(), '\t'), str.end());
        str.erase(remove(str.begin(), str.end(), '\r'), str.end());
        str.erase(remove(str.begin(), str.end(), '\n'), str.end());
        return str;
    }
    // A custom getline function that trims the word before returning
    inline std::string getline(std::istream& lineStream, char delimiter) {
        std::string word;
        getline(lineStream, word, delimiter);
        trim(word);
        return word;
    }
    // A custom stoi function that detects the input number base
    inline int stoi(std::string number) {
        if (number[0] == '0' && number.size() > 1) {
            if (number[1] == 'x') {
                // Input is HEX
                return std::stoi(number, 0, 16);
            }
            // Input is OCTAL
            return std::stoi(number, 0, 8);
        }
        // Input is DEC
        return std::stoi(number, 0, 10);
    }
}

#endif /* dbc_parser_helper_h */
