#include <iostream>
#include <string>
#include <vector>
#include "scanner.h"

/*
 * C++ Starter code for CS241 A3
 * All code requires C++14, so if you're getting compile errors make sure to
 * use -std=c++14.
 *
 * This file contains the main function of your program. By default, it just
 * prints the scanned list of tokens back to standard output.
 */

void print_bin(std::string &str) {
    if (str.length() != 32) {
        std::cerr << "ERROR: Not 32 bits" << std::endl;
    }
    int64_t instr = 0;
    for (int i = 0; i < str.length(); ++i) {
        if (str[i] == '1') {
            instr = instr | (1 << (31 - i));
        }
    }
    char c = instr >> 24;
    std::cout << c;
    c = instr >> 16;
    std::cout << c;
    c = instr >> 8;
    std::cout << c;
    c = instr;
    std::cout << c;
}

void print_hex_in_binary(std::string &hex) {
    std::string result = "";
    std::string content = hex.substr(2, 8);
    for (int i = content.length(); i < 8; ++i) {
        content = "0" + content;
    }
    for (auto &ch : content) {
        switch (ch) {
            case '0':   result += "0000";   break;
            case '1':   result += "0001";   break;
            case '2':   result += "0010";   break;
            case '3':   result += "0011";   break;
            case '4':   result += "0100";   break;
            case '5':   result += "0101";   break;
            case '6':   result += "0110";   break;
            case '7':   result += "0111";   break;
            case '8':   result += "1000";   break;
            case '9':   result += "1001";   break;
            case 'a':   result += "1010";   break;
            case 'b':   result += "1011";   break;
            case 'c':   result += "1100";   break;
            case 'd':   result += "1101";   break;
            case 'e':   result += "1110";   break;
            case 'f':   result += "1111";   break;
        }
    }
    print_bin(result);
}

void print_non_neg_in_binary(int64_t integer) {
    int64_t quo = integer;
    int rem;
    std::string result = "";
    while (quo > 0) {
        rem = quo % 2;
        result = std::to_string(rem) + result;
        quo /= 2;
    }
    for (int i = result.length(); i < 32; ++i) {
        result = "0" + result;
    }
    print_bin(result);
}

void print_neg_in_binary(int64_t integer) {
    int64_t pos = 0 - integer;
    int64_t quo = pos;
    int rem;
    std::string result = "";
    while (quo > 0) {
        rem = quo % 2;
        result = std::to_string(rem) + result;
        quo /= 2;
    }
    for (int i = result.length(); i < 32; ++i) {
        result = "0" + result;
    }
    // Flip bits
    for (int i = 0; i < 32; ++i) {
        if (result[i] == '0') result[i] = '1';
        else if (result[i] == '1') result[i] = '0';
    }
    // Plus one
    for (int i = 31; i >= 0; --i) {
        if (result[i] == '0') {
            result[i] = '1';
            break;
        } 
        else if (result[i] == '1') {
            result[i] = '0';
        }
    }
    print_bin(result);
}

class Label {
    std::string name;
    int value;
  public:
    Label(std::string &name, int value) : name{name}, value{value} {}
    std::string &getname() {return name;}
    int getvalue() {return value;}
    friend std::ostream &operator<<(std::ostream &out, Label &label);
};

std::ostream &operator<<(std::ostream &out, Label &label) {
    out << label.name << " " << label.value;
    return out;
}

// Return 1 if the input token is valid or 0 otherwise
bool pharse_word(Token *tok, std::vector<Label *> *ref) {
    if (tok->getKind() == Token::HEXINT) {
        std::string context = tok->getLexeme();
        print_hex_in_binary(context);
        return 1;
    }
    else if (tok->getKind() == Token::INT) {
        std::string context = tok->getLexeme();
        int64_t num = tok->toLong();
        if (context[0] != '-') {
            print_non_neg_in_binary(num);
        } else {
            print_neg_in_binary(num);
        }
        return 1;
    }
    else if (tok->getKind() == Token::ID) {
        std::string id_name = tok->getLexeme();
        for (auto &ele : (*ref)) {
            if ((ele->getname()) == id_name) {
                int val = ele->getvalue();
                print_non_neg_in_binary(val);
                return 1;
            }
        }
        return 0;
    }
    return 0;
}

int main() {
    std::string line;
    std::vector<Label *> labels;
    int index = 0;
    std::vector<std::vector<Token>> input;
    try {
        while (getline(std::cin, line)) {
            // Scan Inputs for the first time, collect all labels
            std::vector<Token> tokenLine = scan(line);
            // Store Inputs for the Second Scan
            input.emplace_back(tokenLine);
            int size = tokenLine.size();
            for (int i = 0; i < size; ++i) {
                if (tokenLine[i].getKind() == Token::LABEL) {
                    std::string label_name = tokenLine[i].getLexeme();
                    label_name.pop_back();
                    for (auto &ele : labels) {
                        if (label_name == ele->getname()) {
                            throw ScanningFailure("ERROR: Double declaration of label");
                        }
                    }
                    labels.emplace_back(new Label{label_name, index});
                }
                else {
                    index += 4;
                    break;
                }
            }
        }
        // The Second Scan begins here
        for (auto &inputLine : input) {
            int size = inputLine.size();
            for (int i = 0; i < size; ++i) {
                if (inputLine[i].getKind() == Token::LABEL) {
                    continue;
                }
                else if (inputLine[i].getKind() == Token::WORD) {
                    // Not the last element
                    if (i == size - 2) {
                        int success = pharse_word(&inputLine[i+1], &labels);
                        if (!success) {
                            throw ScanningFailure("ERROR: The token after .word is invalid");
                        }
                        break;
                    }
                    else {
                        throw ScanningFailure("ERROR: Invalid Number of Tokens");
                    }
                }
                else {
                    throw ScanningFailure("ERROR: Invalid Instruction");
                }
            }
        }
    } catch (ScanningFailure &f) {
        std::cerr << f.what() << std::endl;
        return 1;
      }
    // Clear the labels
    for (auto &label : labels) {
        delete label;
    }
    labels.clear();
    return 0;
}
