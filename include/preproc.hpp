#pragma once

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <cstdlib>

#include "ast.hpp"
#include "visitor.hpp"
#include "nlohmann/json.hpp"

using std::string, std::ifstream, std::ofstream;
using json = nlohmann::json;

class PreProc: public CompilerPass {
    string result;
public:
    void accept(shared_ptr<IVisitor> vis) { vis->visit(*this); }

    string getResult() {
        return result;
    }

    PreProc(const string& code) {
        ofstream out_file("src/buffer_code.txt");
        out_file << code;
        
        out_file.close();

        system("python src/api.py");

        system("python src/chat.py");

        ifstream file("src/answer.json");
        
        json data;
        file >> data;

        result = data["answer"];
    }
};
