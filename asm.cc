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

void print_bin(std::string &str, int digit) {
    int64_t instr = 0;
    for (int i = 0; i < str.length(); ++i) {
        if (str[i] == '1') {
            instr = instr | (1 << ((digit - 1) - i));
        }
    }
    char c;
    for (int j = (digit - 8); j >= 0; j -= 8) {
        c = instr >> j;
        std::cout << c;
    }
}

void print_hex_in_binary(std::string &hex, int digit) {
    std::string result = "";
    std::string content = hex.substr(2, 8);
    digit /= 4;
    for (int i = content.length(); i < digit; ++i) {
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
    digit *= 4;
    print_bin(result, digit);
}

void print_non_neg_in_binary(int64_t integer, int digit) {
    int64_t quo = integer;
    int rem;
    std::string result = "";
    while (quo > 0) {
        rem = quo % 2;
        result = std::to_string(rem) + result;
        quo /= 2;
    }
    for (int i = result.length(); i < digit; ++i) {
        result = "0" + result;
    }
    print_bin(result, digit);
}

void print_neg_in_binary(int64_t integer, int digit) {
    int64_t pos = 0 - integer;
    int64_t quo = pos;
    int rem;
    std::string result = "";
    while (quo > 0) {
        rem = quo % 2;
        result = std::to_string(rem) + result;
        quo /= 2;
    }
    for (int i = result.length(); i < digit; ++i) {
        result = "0" + result;
    }
    // Flip bits
    for (int i = 0; i < digit; ++i) {
        if (result[i] == '0') result[i] = '1';
        else if (result[i] == '1') result[i] = '0';
    }
    // Plus one
    for (int i = (digit - 1); i >= 0; --i) {
        if (result[i] == '0') {
            result[i] = '1';
            break;
        } 
        else if (result[i] == '1') {
            result[i] = '0';
        }
    }
    print_bin(result, digit);
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
bool pharse_word(Token *tok, std::vector<Label> *ref, int digit, 
        bool is_encoded = 0, int index = 0) {
    if (tok->getKind() == Token::HEXINT) {
        std::string context = tok->getLexeme();
        print_hex_in_binary(context, digit);
        return 1;
    }
    else if (tok->getKind() == Token::INT) {
        std::string context = tok->getLexeme();
        int64_t num = tok->toLong();
        if (context[0] != '-') {
            print_non_neg_in_binary(num, digit);
        } else {
            print_neg_in_binary(num, digit);
        }
        return 1;
    }
    else if (tok->getKind() == Token::ID) {
        std::string id_name = tok->getLexeme();
        for (auto &ele : (*ref)) {
            if ((ele.getname()) == id_name) {
                int val = ele.getvalue();
                if (is_encoded) val = (val - index - 4) / 4;
                if (val < 0) print_neg_in_binary(val, digit);
                else print_non_neg_in_binary(val, digit);
                return 1;
            }
        }
        return 0;
    }
    return 0;
}

// obtain_regnum() append the binary representation of the register #, reg
// to the string provided, str
void obtain_regnum(int64_t reg, std::string &str) {
    if (reg < 0 || reg > 31) {
        throw ScanningFailure("ERROR: Invalid Register #");
    }
    std::string result = "";
    int64_t quo = reg;
    int rem;
    while (quo > 0) {
        rem = quo % 2;
        result = std::to_string(rem) + result;
        quo /= 2;
    }
    for (int i = result.length(); i < 5; ++i) {
        result = "0" + result;
    }
    str += result;
}

int main() {
    std::string line;
    std::vector<Label> labels;
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
                        if (label_name == ele.getname()) {
                            throw ScanningFailure("ERROR: Double declaration of label");
                        }
                    }
                    labels.emplace_back(Label{label_name, index});
                }
                else {
                    index += 4;
                    break;
                }
            }
        }
        // The Second Scan begins here
        index = 0;
        for (auto &inputLine : input) {
            int size = inputLine.size();
            for (int i = 0; i < size; ++i) {
                if (inputLine[i].getKind() == Token::LABEL) {
                    continue;
                }
                else if (inputLine[i].getKind() == Token::WORD) {
                    // Must be the last second element
                    if (i == size - 2) {
                        int success = pharse_word(&inputLine[i+1], &labels, 32);
                        if (!success) {
                            throw ScanningFailure("ERROR: The token after .word is invalid");
                        }
                        index += 4;
                        break;
                    }
                    else {
                        throw ScanningFailure("ERROR: Invalid Number of Tokens");
                    }
                }
                else if (inputLine[i].getKind() == Token::ID) {
                    if (inputLine[i].getLexeme() == "jr") {
                        // Make sure "jr" is the last second token and the last token is a register
                        if (i == (size - 2) && inputLine[i+1].getKind() == Token::REG) {
                            int64_t reg = inputLine[i+1].toLong();
                            std::string instru;
                            obtain_regnum(reg, instru);
                            instru = "000000" + instru + "000000000000000001000";
                            print_bin(instru, 32);
                        }
                        else {
                            throw ScanningFailure("ERROR: The token after jr is invalid");
                        }
                        index += 4;
                        break;
                    }
                    else if (inputLine[i].getLexeme() == "jalr") {
                        // Make sure "jalr" is the last second token and the last token is a register
                        if (i == (size - 2) && inputLine[i+1].getKind() == Token::REG) {
                            int64_t reg = inputLine[i+1].toLong();
                            std::string instru = "";
                            obtain_regnum(reg, instru);
                            instru = "000000" + instru + "000000000000000001001";
                            print_bin(instru, 32);
                        }
                        else {
                            throw ScanningFailure("ERROR: The token after jalr is invalid");
                        }
                        index += 4;
                        break;
                    }
                    else if (inputLine[i].getLexeme() == "add") {
                        // Make sure "add" is the last sixth token and all the tokens after "add" are registers and comma
                        if (i == (size - 6) && inputLine[i+1].getKind() == Token::REG
                            && inputLine[i+2].getKind() == Token::COMMA && inputLine[i+3].getKind() == Token::REG
                            && inputLine[i+4].getKind() == Token::COMMA && inputLine[i+5].getKind() == Token::REG) {
                            int64_t reg1 = inputLine[i+1].toLong();
                            int64_t reg2 = inputLine[i+3].toLong();
                            int64_t reg3 = inputLine[i+5].toLong();
                            std::string instru = "";
                            obtain_regnum(reg2, instru);
                            obtain_regnum(reg3, instru);
                            obtain_regnum(reg1, instru);
                            instru = "000000" + instru + "00000100000";
                            print_bin(instru, 32);
                        }
                        else {
                            throw ScanningFailure("ERROR: The tokens after add are invalid");
                        }
                        index += 4;
                        break;
                    }
                    else if (inputLine[i].getLexeme() == "sub") {
                        // Make sure "sub" is the last fourth token and all the tokens after "sub" are registers and comma
                        if (i == (size - 6) && inputLine[i+1].getKind() == Token::REG
                            && inputLine[i+2].getKind() == Token::COMMA && inputLine[i+3].getKind() == Token::REG
                            && inputLine[i+4].getKind() == Token::COMMA && inputLine[i+5].getKind() == Token::REG) {
                            int64_t reg1 = inputLine[i+1].toLong();
                            int64_t reg2 = inputLine[i+3].toLong();
                            int64_t reg3 = inputLine[i+5].toLong();
                            std::string instru = "";
                            obtain_regnum(reg2, instru);
                            obtain_regnum(reg3, instru);
                            obtain_regnum(reg1, instru);
                            instru = "000000" + instru + "00000100010";
                            print_bin(instru, 32);
                        }
                        else {
                            throw ScanningFailure("ERROR: The tokens after sub are invalid");
                        }
                        break;
                    }
                    else if (inputLine[i].getLexeme() == "slt") {
                        // Make sure "slt" is the last fourth token and all the tokens after "slt" are registers and comma
                        if (i == (size - 6) && inputLine[i+1].getKind() == Token::REG
                            && inputLine[i+2].getKind() == Token::COMMA && inputLine[i+3].getKind() == Token::REG
                            && inputLine[i+4].getKind() == Token::COMMA && inputLine[i+5].getKind() == Token::REG) {
                            int64_t reg1 = inputLine[i+1].toLong();
                            int64_t reg2 = inputLine[i+3].toLong();
                            int64_t reg3 = inputLine[i+5].toLong();
                            std::string instru = "";
                            obtain_regnum(reg2, instru);
                            obtain_regnum(reg3, instru);
                            obtain_regnum(reg1, instru);
                            instru = "000000" + instru + "00000101010";
                            print_bin(instru, 32);
                        }
                        else {
                            throw ScanningFailure("ERROR: The tokens after slt are invalid");
                        }
                        index += 4;
                        break;
                    }
                    else if (inputLine[i].getLexeme() == "sltu") {
                        // Make sure "sltu" is the last fourth token and all the tokens after "sltu" are registers and comma
                        if (i == (size - 6) && inputLine[i+1].getKind() == Token::REG
                            && inputLine[i+2].getKind() == Token::COMMA && inputLine[i+3].getKind() == Token::REG
                            && inputLine[i+4].getKind() == Token::COMMA && inputLine[i+5].getKind() == Token::REG) {
                            int64_t reg1 = inputLine[i+1].toLong();
                            int64_t reg2 = inputLine[i+3].toLong();
                            int64_t reg3 = inputLine[i+5].toLong();
                            std::string instru = "";
                            obtain_regnum(reg2, instru);
                            obtain_regnum(reg3, instru);
                            obtain_regnum(reg1, instru);
                            instru = "000000" + instru + "00000101011";
                            print_bin(instru, 32);
                        }
                        else {
                            throw ScanningFailure("ERROR: The tokens after sltu are invalid");
                        }
                        index += 4;
                        break;
                    }
                    else if (inputLine[i].getLexeme() == "beq") {
                        if (i == (size - 6) && inputLine[i+1].getKind() == Token::REG
                            && inputLine[i+2].getKind() == Token::COMMA && inputLine[i+3].getKind() == Token::REG
                            && inputLine[i+4].getKind() == Token::COMMA) {
                            int64_t reg1 = inputLine[i+1].toLong();
                            int64_t reg2 = inputLine[i+3].toLong();
                            std::string instru = "";
                            obtain_regnum(reg1, instru);
                            obtain_regnum(reg2, instru);
                            instru = "000100" + instru;
                            // Output the offset
                            if (inputLine[i+5].getKind() == Token::INT) {
                                int64_t value = inputLine[i+5].toLong();
                                if (value < -32768 || value > 32767) {
                                    throw ScanningFailure("ERROR: Offset overflow (int)");
                                }
                            }
                            else if (inputLine[i+5].getKind() == Token::HEXINT) {
                                std::string hex = inputLine[i+5].getLexeme();
                                if (hex.length() > 6) {
                                    throw ScanningFailure("ERROR: Offset overflow (hex)");
                                }
                            }
                            if (inputLine[i+5].getKind() == Token::ID) {
                                bool label_exist = 0;
                                std::string label_name = inputLine[i+5].getLexeme();
                                for (auto &ele : labels) {
                                    if (label_name == ele.getname()) {
                                        label_exist = 1;
                                        break;
                                    }
                                }
                                if (!label_exist) {
                                    throw ScanningFailure("ERROR: The offset for beq is invalid");
                                }
                            }
                            print_bin(instru, 16);
                            bool success = pharse_word(&inputLine[i+5], &labels, 16, 1, index);
                            if (!success) {
                                throw ScanningFailure("ERROR: The offset for beq is invalid");
                            }
                        }
                        else {
                            throw ScanningFailure("ERROR: The tokens after beq are invalid");
                        }
                        index += 4;
                        break;
                    }
                    else if (inputLine[i].getLexeme() == "bne") {
                        if (i == (size - 6) && inputLine[i+1].getKind() == Token::REG
                            && inputLine[i+2].getKind() == Token::COMMA && inputLine[i+3].getKind() == Token::REG
                            && inputLine[i+4].getKind() == Token::COMMA) {
                            int64_t reg1 = inputLine[i+1].toLong();
                            int64_t reg2 = inputLine[i+3].toLong();
                            std::string instru = "";
                            obtain_regnum(reg1, instru);
                            obtain_regnum(reg2, instru);
                            instru = "000101" + instru;
                            // Output the offset
                            if (inputLine[i+5].getKind() == Token::INT) {
                                int64_t value = inputLine[i+5].toLong();
                                if (value < -32768 || value > 32767) {
                                    throw ScanningFailure("ERROR: Offset overflow (int)");
                                }
                            }
                            else if (inputLine[i+5].getKind() == Token::HEXINT) {
                                std::string hex = inputLine[i+5].getLexeme();
                                if (hex.length() > 6) {
                                    throw ScanningFailure("ERROR: Offset overflow (hex)");
                                }
                            }
                            if (inputLine[i+5].getKind() == Token::ID) {
                                bool label_exist = 0;
                                std::string label_name = inputLine[i+5].getLexeme();
                                for (auto &ele : labels) {
                                    if (label_name == ele.getname()) {
                                        label_exist = 1;
                                        break;
                                    }
                                }
                                if (!label_exist) {
                                    throw ScanningFailure("ERROR: The offset for bne is invalid");
                                }
                            }
                            print_bin(instru, 16);
                            bool success = pharse_word(&inputLine[i+5], &labels, 16, 1, index);
                            if (!success) {
                                throw ScanningFailure("ERROR: The offset for bne is invalid");
                            }
                        }
                        else {
                            throw ScanningFailure("ERROR: The tokens after bne are invalid");
                        }
                        index += 4;
                        break;
                    }
                    else if (inputLine[i].getLexeme() == "lis") {
                        if (i == (size - 2) && inputLine[i+1].getKind() == Token::REG) {
                            int64_t reg = inputLine[i+1].toLong();
                            std::string instru;
                            obtain_regnum(reg, instru);
                            instru = "0000000000000000" + instru + "00000010100";
                            print_bin(instru, 32);
                        }
                        else {
                            throw ScanningFailure("ERROR: The token after lis is invalid");
                        }
                        index += 4;
                        break;
                    }
                    else if (inputLine[i].getLexeme() == "mflo") {
                        if (i == (size - 2) && inputLine[i+1].getKind() == Token::REG) {
                            int64_t reg = inputLine[i+1].toLong();
                            std::string instru;
                            obtain_regnum(reg, instru);
                            instru = "0000000000000000" + instru + "00000010010";
                            print_bin(instru, 32);
                        }
                        else {
                            throw ScanningFailure("ERROR: The token after mflo is invalid");
                        }
                        index += 4;
                        break;
                    }
                    else if (inputLine[i].getLexeme() == "mfhi") {
                        if (i == (size - 2) && inputLine[i+1].getKind() == Token::REG) {
                            int64_t reg = inputLine[i+1].toLong();
                            std::string instru;
                            obtain_regnum(reg, instru);
                            instru = "0000000000000000" + instru + "00000010000";
                            print_bin(instru, 32);
                        }
                        else {
                            throw ScanningFailure("ERROR: The token after mfhi is invalid");
                        }
                        index += 4;
                        break;
                    }
                    else if (inputLine[i].getLexeme() == "lw") {
                        if (i == (size - 7) && inputLine[i+1].getKind() == Token::REG
                            && inputLine[i+2].getKind() == Token::COMMA && inputLine[i+4].getKind() == Token::LPAREN
                            && inputLine[i+5].getKind() == Token::REG && inputLine[i+6].getKind() == Token::RPAREN) {
                            int64_t reg1 = inputLine[i+1].toLong();
                            int64_t reg2 = inputLine[i+5].toLong();
                            std::string instru = "";
                            obtain_regnum(reg2, instru);
                            obtain_regnum(reg1, instru);
                            instru = "100011" + instru;
                            // Output the offset
                            if (inputLine[i+3].getKind() == Token::INT) {
                                int64_t value = inputLine[i+3].toLong();
                                if (value < -32768 || value > 32767) {
                                    throw ScanningFailure("ERROR: Offset overflow (int)");
                                }
                            }
                            else if (inputLine[i+3].getKind() == Token::HEXINT) {
                                std::string hex = inputLine[i+3].getLexeme();
                                if (hex.length() > 6) {
                                    throw ScanningFailure("ERROR: Offset overflow (hex)");
                                }
                            }
                            print_bin(instru, 16);
                            bool success = pharse_word(&inputLine[i+3], &labels, 16);
                            if (!success) {
                                throw ScanningFailure("ERROR: The offset for lw is invalid");
                            }
                            index += 4;
                            break;
                        }
                        else {
                            throw ScanningFailure("ERROR: The tokens after lw are invalid");
                        }
                    }
                    else if (inputLine[i].getLexeme() == "sw") {
                        if (i == (size - 7) && inputLine[i+1].getKind() == Token::REG
                            && inputLine[i+2].getKind() == Token::COMMA && inputLine[i+4].getKind() == Token::LPAREN
                            && inputLine[i+5].getKind() == Token::REG && inputLine[i+6].getKind() == Token::RPAREN) {
                            int64_t reg1 = inputLine[i+1].toLong();
                            int64_t reg2 = inputLine[i+5].toLong();
                            std::string instru = "";
                            obtain_regnum(reg2, instru);
                            obtain_regnum(reg1, instru);
                            instru = "101011" + instru;
                            // Output the offset
                            if (inputLine[i+3].getKind() == Token::INT) {
                                int64_t value = inputLine[i+3].toLong();
                                if (value < -32768 || value > 32767) {
                                    throw ScanningFailure("ERROR: Offset overflow (int)");
                                }
                            }
                            else if (inputLine[i+3].getKind() == Token::HEXINT) {
                                std::string hex = inputLine[i+3].getLexeme();
                                if (hex.length() > 6) {
                                    throw ScanningFailure("ERROR: Offset overflow (hex)");
                                }
                            }
                            print_bin(instru, 16);
                            bool success = pharse_word(&inputLine[i+3], &labels, 16);
                            if (!success) {
                                throw ScanningFailure("ERROR: The offset for lw is invalid");
                            }
                            index += 4;
                            break;
                        }
                        else {
                            throw ScanningFailure("ERROR: The tokens after sw are invalid");
                        }
                    }
                    else {
                        throw ScanningFailure("ERROR: Invalid Instruction");
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
    labels.clear();
    return 0;
}
