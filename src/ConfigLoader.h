//
// Created by jakob on 07.10.2021.
//

#ifndef LAB1_1_CONFIGLOADER_H
#define LAB1_1_CONFIGLOADER_H

#include <string>
#include <fstream>
#include <functional>

class LoaderTokens
{
    std::string delim;
    std::function<void(std::string const & val, int token_code)> setter_fun;
    std::function<int(std::string const & val)> token_fun;
public:
    LoaderTokens(std::string d, std::function<void(std::string const &, int)> sf, std::function<int(std::string const &)> tf);
    std::string const & getDelim() const;
    int getTokenId(std::string const & token) const;
    void setVal(std::string const & val, int token_code) const;
};

class ConfigLoader
{
    bool fine = true;
    std::string input_file_name;
    std::ifstream input_file;
public:
    ConfigLoader() = delete;

    explicit ConfigLoader(std::string  file_name);
    void load(LoaderTokens const * token_params);
    bool valid() const;
};


#endif //LAB1_1_CONFIGLOADER_H
