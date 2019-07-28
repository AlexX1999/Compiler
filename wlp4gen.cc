#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include <set>
#include <vector>
#include <utility>

using namespace std;

struct Tree {
    string rule;
    vector<string> tokens;
    vector<Tree> children;

    friend ostream& operator<<(ostream &out, Tree &t);
};

bool error_flag = 0;
Tree ParseTree;
// map(scope, map(name, pair(type, offset)))
map<string, map<string, pair<string, int>>> SymbolTable;
set<string> Terminals = {"BOF", "BECOMES", "COMMA", "ELSE", "EOF", "EQ", "GE", 
                         "GT", "ID", "IF", "INT", "LBRACE", "LE", "LPAREN", "LT", 
                         "MINUS", "NE", "NUM", "PCT", "PLUS", "PRINTLN", "RBRACE", 
                         "RETURN", "RPAREN", "SEMI", "SLASH", "STAR", "WAIN", 
                         "WHILE", "AMP", "LBRACK", "RBRACK", "NEW", "DELETE", "NULL"};

bool is_leaf(Tree *node) {
    string ele = node->tokens[0];
    if (Terminals.find(ele) == Terminals.end()) {
        return 0;
    }
    return 1;
}


ostream &operator<<(ostream &out, Tree &t) {
    out << t.rule << endl;
    if (!is_leaf(&t)) {
        for (auto &ele : t.children) {
            out << ele;
        }
    }
    return out;
}


void build_parsetree(Tree *t) {
    string s;
    if (getline(cin, s)) {
        stringstream iss(s);
        t->rule = s;
        while (iss >> s) {
            t->tokens.emplace_back(s);
        }
        if (!is_leaf(t)) {
            int children_num = t->tokens.size() - 1; // LHS does not count
            t->children.resize(children_num);
            for (int i = 0; i < children_num; ++i) {
                build_parsetree(&(t->children[i]));
            }
        }
    } else {
        cerr << "Warning: Invalid input. Fail to build the ParseTree" << endl;
    }
}

// Must be dcl
string get_type(Tree *t) {
    if (t->tokens[0] != "dcl") {
        cerr << "Warning: must be dcl" << endl;
        return "error";
    } else {
        if (t->children[0].children.size() == 1) {
            return "int";
        } else {
            return "int*";
        }
    }
}

// Must be paramlist
string get_param(Tree *t) {
    if (t->tokens[0] != "paramlist") {
        cerr << "Warning: must be paramlist" << endl;
        return "error";
    } else {
        if (t->children.size() == 1) {
            return get_type(&(t->children[0]));
        } else {
            string result = get_type(&(t->children[0]));
            result = result + " " + get_param(&(t->children[2]));
            return result;
        }
    }
}


string get_lvalue_type(Tree *t, string scope);
string get_factor_type(Tree *t, string scope);
string get_term_type(Tree *t, string scope);
string get_expr_type(Tree *t, string scope);

// Require the LHS of the rule of the input node to be lvalue
string get_lvalue_type(Tree *t, string scope) {
    if (t->tokens[0] == "lvalue" && t->children.size() == 1) {
        string variable_name = t->children[0].tokens[1];
        auto it = SymbolTable[scope].find(variable_name);
        if (it == SymbolTable[scope].end()) {
            cerr << "ERROR: ID not found: " << variable_name << endl;
            return "error";
        } else {
            return it->second.first;
        }
    } else if (t->tokens[0] == "lvalue" && t->children.size() == 2) {
        string ftype = get_factor_type(&(t->children[1]), scope);
        if (ftype == "int*") {
            return "int";
        } else {
            cerr << "ERROR: type check failed: try to dereference " << ftype << endl;
            return "error";
        }
    } else if (t->tokens[0] == "lvalue" && t->children.size() == 3) {
        return get_lvalue_type(&(t->children[1]), scope);
    } else {
        return "error";
    }
}

// Require the LHS of the rule of the input node to be factor
string get_factor_type(Tree *t, string scope) {
    if (t->tokens[0] == "factor" && t->children.size() == 1) {
        if (t->tokens[1] == "NUM") {
            return "int";
        } else if (t->tokens[1] == "NULL") {
           return "int*";
        } else {
            string variable_name = t->children[0].tokens[1];
            auto it = SymbolTable[scope].find(variable_name);
            if (it == SymbolTable[scope].end()) {
                cerr << "ERROR: ID not found: " << variable_name << endl;
                return "error";
            } else {
                return it->second.first;
            }
        }
    } else if (t->tokens[0] == "factor" && t->children.size() == 2) {
        if (t->tokens[1] == "AMP") {
            string ltype = get_lvalue_type(&(t->children[1]), scope);
            if (ltype == "int") {
                return "int*";
            } else {
                cerr << "ERROR: Type check failed: try to obtain the address of" << ltype << endl;
                return "error";
            }
        } else {
            string ftype = get_factor_type(&(t->children[1]), scope);
            if (ftype == "int*") {
                return "int";
            } else {
                cerr << "ERROR: Type check failed: try to dereference " << ftype << endl;
                return "error";
            }
        }
    } else if (t->tokens[0] == "factor" && t->children.size() == 3) {
        if (t->tokens[1] == "LPAREN") {
            return get_expr_type(&(t->children[1]), scope);
        } else {
            // factor → ID LPAREN RPAREN
            string function_name = t->children[0].tokens[1];
            string x = function_name + "@";
            auto it = SymbolTable[function_name].find(x);
            if (it == SymbolTable[function_name].end()) {
                cerr << "ERROR: function not found: " << function_name << endl;
                return "error";
            }
            return "int";
        }
    } else if (t->tokens[0] == "factor" && t->children.size() == 4) {
        string function_name = t->children[0].tokens[1];
        string x = function_name + "@";
        auto it = SymbolTable[function_name].find(x);
        if (it == SymbolTable[function_name].end()) {
            cerr << "ERROR: function not found: " << function_name << endl;
            return "error";
        }
        return "int";
    } else if (t->tokens[0] == "factor" && t->children.size() == 5) {
        string etype = get_expr_type(&(t->children[3]), scope);
        if (etype == "int") {
            return "int*";
        } else {
            cerr << "ERROR: type check failed: new int [" << etype << "]" << endl;
            return "error";
        }
    } else {
        return "error";
    }
}
        
// Require the LHS of the rule of the input node to be term
string get_term_type(Tree *t, string scope) {
    if (t->tokens[0] == "term" && t->children.size() == 1) {
        return get_factor_type(&(t->children[0]), scope);
    } else if (t->tokens[0] == "term" && t->children.size() == 3) {
        string ttype = get_term_type(&(t->children[0]), scope);
        string ftype = get_factor_type(&(t->children[2]), scope);
        if (t->tokens[2] == "STAR") {
            if (ttype == "int" && ftype == "int") {
                return "int";
            } else {
                cerr << "ERROR: Type check failed: " << ttype << " * " << ftype << endl;
                return "error";
            }
        } else if (t->tokens[2] == "SLASH") {
            if (ttype == "int" && ftype == "int") {
                return "int";
            } else {
                cerr << "ERROR: Type check failed: " << ttype << " / " << ftype << endl;
                return "error";
            }
        } else { 
            // PCT Case
            if (ttype == "int" && ftype == "int") {
                return "int";
            } else {
                cerr << "ERROR: Type check failed: " << ttype << " % " << ftype << endl;
                return "error";
            }
        }
    } else {
        return "error";
    }
}

// Require the LHS of the rule of the input node to be expr
string get_expr_type(Tree *t, string scope) {
    if (t->tokens[0] == "expr") {
        if (t->children.size() == 1) {
            return get_term_type(&(t->children[0]), scope);
        } else if (t->children.size() == 3) {
            string etype = get_expr_type(&(t->children[0]), scope); 
            string ttype = get_term_type(&(t->children[2]), scope);
            if (t->tokens[2] == "PLUS") {
                if (ttype == "int" && etype == "int") {
                    return "int";
                } else if ((ttype == "int*" && etype == "int") || (ttype == "int" && etype == "int*")) {
                    return "int*";
                } else {
                    cerr << "ERROR: Type checking failed: " << etype << " + " << ttype << endl;
                    return "error";
                }
            } else if (t->tokens[2] == "MINUS") {
                if ((etype == "int" && ttype == "int") || (etype == "int*" && ttype == "int*")) {
                    return "int";
                } else if (etype == "int*" && ttype == "int") {
                    return "int*";
                } else {
                    cerr << "ERROR: Type checking failed: " << etype << " - " << ttype << endl;
                    return "error";
                }
            }
        }
    } else {
        return "error";
    }
}
        

// Require the LHS of the rule of the input node to be dcl or dcls
int current_offset = 0;
void add_variable_to_symboltable(Tree *t, string scope) {
    string LHS = t->tokens[0];
    if (LHS != "dcl" && LHS != "dcls") {
        cerr << "Warning: The rule of input node has to have dcl or dcls as LHS" << endl;
    } else {
        if (LHS == "dcl") {
            string name = t->children[1].tokens[1];
            if (SymbolTable[scope].find(name) != SymbolTable[scope].end()) {
                cerr << "ERROR: " << name << " has been declared before" << endl;
                error_flag = 1;
            }
            if (t->children[0].children.size() == 2) {
                SymbolTable[scope][name] = make_pair("int*", current_offset);
                current_offset -= 4;
            } else if (t->children[0].children.size() == 1) {
                SymbolTable[scope][name] = make_pair("int", current_offset);
                current_offset -= 4;
            }
        } else {
            if (t->children.size() != 0) {
                add_variable_to_symboltable(&(t->children[0]), scope);
                add_variable_to_symboltable(&(t->children[1]), scope);
            }
        }
    }
}

// Require the LHS of the rule of the input node to be paramlist
void add_param_to_symboltable(Tree *t, string scope) {
    if (t->tokens[0] != "paramlist") {
        cerr << "Warning: Require LHS to be paramlist" << endl;
    } else {
        add_variable_to_symboltable(&(t->children[0]), scope);
        if (t->children.size() == 3) {
            add_param_to_symboltable(&(t->children[2]), scope);
        }
    }
}

int get_param_num(Tree *t) {
    if (t->rule == "params") {
        return 0;
    } else if (t->rule == "params paramlist") {
        return get_param_num(&(t->children[0]));
    } else if (t->rule == "paramlist dcl") {
        return 1;
    } else if(t->rule == "paramlist dcl COMMA paramlist") {
        return 1 + get_param_num(&(t->children[2]));
    }
}
        

// Require the LHS of the rule of the input node to be procedures
void build_symboltable(Tree *t) {
    if (t->tokens[0] != "procedures") {
        cerr << "Warning: The rule of input node has to have procudures as LHS" << endl;
    }
    if (t->tokens[1] == "main") {
        current_offset = 0;
        string scope = "wain";
        string para1_type = get_type(&(t->children[0].children[3]));
        string para2_type = get_type(&(t->children[0].children[5]));
        string para_type = para1_type + " " + para2_type;
        // Use -1 as a dummy offset
        SymbolTable["wain"]["wain@"] = make_pair(para_type, -1);
        add_variable_to_symboltable(&(t->children[0].children[3]), scope);
        add_variable_to_symboltable(&(t->children[0].children[5]), scope);
        add_variable_to_symboltable(&(t->children[0].children[8]), scope);
        check_for_use(&(t->children[0].children[9]), scope);
        check_for_use(&(t->children[0].children[11]), scope);
    } else {
        int num_param = get_param_num(&(t->children[0].children[3]));
        current_offset = num_param * 4;
        string scope = t->children[0].children[1].tokens[1];
        if (SymbolTable.find(scope) != SymbolTable.end()) {
            cerr << "ERROR: Duplication function declaration: " << scope << endl;
            error_flag = 1;
            return;
        }
        if (t->children[0].children[3].tokens[0] == "params" && t->children[0].children[3].children.size() == 0) {
            // Use -1 as a dummy offset
            string x = scope + "@";
            SymbolTable[scope][x] = make_pair("", -1);
        } else if (t->children[0].children[3].tokens[0] == "params" && t->children[0].children[3].children.size() == 1) {
            // Note that to distinguish function signiture and variable name
            // We append '@' to the function name
            string function_name = scope + "@";
            string param = get_param(&(t->children[0].children[3].children[0]));
            SymbolTable[scope][function_name] = make_pair(param, -1);       
            add_param_to_symboltable(&(t->children[0].children[3].children[0]), scope);
        }
        add_variable_to_symboltable(&(t->children[0].children[6]), scope);
        check_for_use(&(t->children[0].children[7]), scope);
        check_for_use(&(t->children[0].children[9]), scope);
        build_symboltable(&(t->children[1]));
    }
}

void check_for_use(Tree *t, string scope) {
    if (t->tokens[0] == "factor") {
        // Variable check
        if (t->children.size() == 1 && t->tokens[1] == "ID") {
            if (SymbolTable[scope].find(t->children[0].tokens[1]) == SymbolTable[scope].end()) {
                cerr << "ERROR: Detect the use of undeclared variable: " << t->children[0].tokens[1] << endl;
                error_flag = 1;
                return;
            }
        }
        // Function call check
        else if (t->children.size() > 1 && t->tokens[2] == "LPAREN") {
            string fn_name = t->children[0].tokens[1];
            if (SymbolTable[scope].find(fn_name) != SymbolTable[scope].end() ||
                    SymbolTable.find(fn_name) == SymbolTable.end()) {
                cerr << "ERROR: Dectect the use of undeclared function call: " << fn_name << endl;
                error_flag = 1;
                return;
            }
        }
    }
    if (t->tokens[0] == "lvalue" && t->tokens[1] == "ID") {
        if (SymbolTable[scope].find(t->children[0].tokens[1]) == SymbolTable[scope].end()) {
            cerr << "ERROR: Detect the use of undeclared variable: " << t->children[0].tokens[1] << endl;
            error_flag = 1;
            return;
        }
    }

    if (!is_leaf(t)) {
        int size = t->children.size();
        for (int i = 0; i < size; ++i) {
           check_for_use(&(t->children[i]), scope);
        }
    }
}


void check_test(Tree *t, string scope) {
    string LHS_type = get_expr_type(&(t->children[0]), scope);
    string RHS_type = get_expr_type(&(t->children[2]), scope);
    if (LHS_type != RHS_type) {
        cerr << "ERROR: Comparision cannot be made due to type difference" << endl;
    }
}

void check_statement(Tree *t, string scope) {
    if (t->tokens[1] == "lvalue") {
        string LHS_type = get_lvalue_type(&(t->children[0]), scope);
        string RHS_type = get_expr_type(&(t->children[2]), scope);
        if (LHS_type != RHS_type) {
            cerr << "ERROR: Assignment failed due to type difference" << endl;
        }
    } else if (t->tokens[1] == "PRINTLN") {
        string etype = get_expr_type(&(t->children[2]), scope);
        if (etype != "int") {
            cerr << "ERROR: Wrong input type for println: " << etype << endl;
        }
    } else if (t->tokens[1] == "DELETE") {
        string etype = get_expr_type(&(t->children[3]), scope);
        if (etype != "int*") {
            cerr << "ERROR: try to delete wrong stuff :" << etype << endl;
        }
    }
} 

string current_scope;
void expr_type_check(Tree *t) {
    if (t->tokens[0] == "procedure") {
        current_scope = t->children[1].tokens[1];
        string etype = get_expr_type(&(t->children[9]), current_scope);
        if (etype != "int") {
            cerr << "ERROR: Wrong return type" << endl;
        }
    } else if (t->tokens[0] == "expr") {
        get_expr_type(t, current_scope);
    } else if (t->tokens[0] == "lvalue") {
        get_lvalue_type(t, current_scope);
    } else if (t->tokens[0] == "main") {
        current_scope = "wain";
        string etype = get_expr_type(&(t->children[11]), current_scope);
        if (etype != "int") {
            cerr << "ERROR: Wrong return type" << endl;
        }
        string second_param_type = SymbolTable[current_scope][t->children[5].children[1].tokens[1]];
        if (second_param_type == "int*") {
            cerr << "ERROR: The second parameter of wain cannot be int*" << endl;
        }
    } else if (t->tokens[0] == "test") {
        check_test(t, current_scope);
    } else if (t->tokens[0] == "statement") {
        check_statement(t, current_scope);
    } else if (t->tokens[0] == "dcls" && t->children.size() > 0) {
        string type = SymbolTable[current_scope][t->children[1].children[1].tokens[1]];
        if (t->tokens[4] == "NUM" && type != "int") {
            cerr << "ERROR: Wrong Assignment" << endl;
        } else if (t->tokens[4] == "NULL" && type != "int*") {
            cerr << "ERROR: Wrong Assignment" << endl;
        }
    }
    if (!is_leaf(t)) {
        int size = t->children.size();
        for (int i = 0; i < size; ++i) {
            expr_type_check(&(t->children[i]));
        }
    }
}


// Input: Register Number
void push(int i) {
    cout << "sw $" << i << ", -4($30)" << endl;
    cout << "sub $30, $30, $4" << endl;
}

// Input: Register Number
void pop(int i) {
    cout << "add $30, $30, $4" << endl;
    cout << "lw $" << i << ", -4($30)" << endl;
}

void out_of_scope(string scope) {
    int bytes = (SymbolTable[scope].size() - 1) * 4;
    cout << "lis $5" << endl;
    cout << ".word " << bytes << endl;
    cout << "add $30, $30, $5" << endl;
}

void factor_code(Tree *t, string scope);
void term_code(Tree *t, string scope);
void expr_code(Tree *t, string scope);


void lvalue_code(Tree *t, string scope) {
    if (t->rule == "lvalue ID") {
        string id = t->children[0].tokens[1];
        int offset = SymbolTable[scope][id].second;
        cout << "lis $5" << endl;
        cout << ".word " << offset << endl;
        cout << "add $3, $29, $5" << endl;
    } else if (t->rule == "lvalue STAR factor") {
        factor_code(&(t->children[1]), scope);
    } else if (t->rule == "lvalue LPAREN lvalue RPAREN") {
        lvalue_code(&(t->children[1]), scope);
    }
}

// IMPORTANT: this function modifies the stack pointer
int arglist_code(Tree *t, string scope, bool if_print) {
    if (t->rule == "arglist expr") {
        if (if_print) {
            expr_code(&(t->children[0]), scope);
            push(3);
        }
        return 1;
    } else if (t->rule == "arglist expr COMMA arglist") {
        if (if_print) {
            expr_code(&(t->children[0]), scope);
            push(3);
        }
        return 1 + arglist_code(&(t->children[2]), scope, if_print);
    }
}

// Require t->tokens[0] to be factor, otherwise do nothing
void factor_code(Tree *t, string scope) {
    if (t->rule == "factor ID") {
        string name = t->children[0].tokens[1];
        int offset = SymbolTable[scope][name].second;
        cout << "lw $3, " << offset << "($29)" << endl;
    } else if (t->rule == "factor LPAREN expr RPAREN") {
        expr_code(&(t->children[1]), scope);
    } else if (t->rule == "factor NUM") {
        cout << "lis $3" << endl;
        cout << ".word " << t->children[0].tokens[1] << endl;
    } else if (t->rule == "factor NULL") {
        cout << "add $3, $0, $11" << endl;
    } else if (t->rule == "factor STAR factor") {
        factor_code(&(t->children[1]), scope);
        cout << "lw $3, 0($3)" << endl;
    } else if (t->rule == "factor AMP lvalue") {
        lvalue_code(&(t->children[1]), scope);
    } else if (t->rule == "factor NEW INT LBRACK expr RBRACK") {
        expr_code(&(t->children[3]), scope);
        cout << "add $1, $3, $0" << endl;
        cout << "lis $5" << endl;
        cout << ".word new" << endl;
        push(31);
        cout << "jalr $5" << endl;
        pop(31);
        cout << "bne $3, $0, 1" << endl;
        cout << "add $3, $11, $0" << endl;
    } else if (t->rule == "factor ID LPAREN RPAREN") {
        string id = t->children[0].tokens[1];
        push(29);
        push(31);
        cout << "lis $5" << endl;
        cout << ".word F" << id << endl;
        cout << "jalr $5" << endl;
        pop(31);
        pop(29);
    } else if (t->rule == "factor ID LPAREN arglist RPAREN") {
        string id = t->children[0].tokens[1];
        push(29);
        push(31);
        int param_num = arglist_code(&(t->children[2]), scope, 1);
        cout << "lis $5" << endl;
        cout << ".word F" << id << endl;
        cout << "jalr $5" << endl;
        // Update stack pointer at once
        param_num *= 4;
        cout << "lis $5" << endl;
        cout << ".word " << param_num << endl;
        cout << "add $30, $30, $5" << endl;
        pop(31);
        pop(29);
    }
}   


// Require t->tokens[0] to be term
void term_code(Tree *t, string scope) {
    if (t->rule == "term factor") {
        factor_code(&(t->children[0]), scope);
    } else if (t->rule == "term term STAR factor") {
        term_code(&(t->children[0]), scope);
        push(3);
        factor_code(&(t->children[2]), scope);
        pop(5);
        cout << "mult $5, $3" << endl;
        cout << "mflo $3" << endl;
    } else if (t->rule == "term term SLASH factor") {
        term_code(&(t->children[0]), scope);
        push(3);
        factor_code(&(t->children[2]), scope);
        pop(5);
        cout << "div $5, $3" << endl;
        cout << "mflo $3" << endl;
    } else if (t->rule == "term term PCT factor") {
        term_code(&(t->children[0]), scope);
        push(3);
        factor_code(&(t->children[2]), scope);
        pop(5);
        cout << "div $5, $3" << endl;
        cout << "mfhi $3" << endl;
    }
}

// Require t->tokens[0] to be expr
void expr_code(Tree *t, string scope) {
    if (t->rule == "expr term") {
        term_code(&(t->children[0]), scope);
    } else if (t->rule == "expr expr PLUS term") {
        string LHS_type = get_expr_type(&(t->children[0]), scope);
        string RHS_type = get_term_type(&(t->children[2]), scope);
        if (LHS_type == "int" && RHS_type == "int") {
            expr_code(&(t->children[0]), scope);
            push(3);
            term_code(&(t->children[2]), scope);
            pop(5);
            cout << "add $3, $5, $3" << endl;
        } else if (LHS_type == "int*" && RHS_type == "int") {
            expr_code(&(t->children[0]), scope);
            push(3);
            term_code(&(t->children[2]), scope);
            cout << "mult $3, $4" << endl;
            cout << "mflo $3" << endl;
            pop(5);
            cout << "add $3, $5, $3" << endl;
        } else if (LHS_type == "int" && RHS_type == "int*") {
            expr_code(&(t->children[0]), scope);
            cout << "mult $3, $4" << endl;
            cout << "mflo $3" << endl;
            push(3);
            term_code(&(t->children[2]), scope);
            pop(5);
            cout << "add $3, $5, $3" << endl;
        }
    } else if (t->rule == "expr expr MINUS term") {
        string LHS_type = get_expr_type(&(t->children[0]), scope);
        string RHS_type = get_term_type(&(t->children[2]), scope);
        if (LHS_type == "int" && RHS_type == "int") {
            expr_code(&(t->children[0]), scope);
            push(3);
            term_code(&(t->children[2]), scope);
            pop(5);
            cout << "sub $3, $5, $3" << endl;
        } else if (LHS_type == "int*" && RHS_type == "int") {
            expr_code(&(t->children[0]), scope);
            push(3);
            term_code(&(t->children[2]), scope);
            cout << "mult $3, $4" << endl;
            cout << "mflo $3" << endl;
            pop(5);
            cout << "sub $3, $5, $3" << endl;
        } else if (LHS_type == "int*" && RHS_type == "int*") {
            expr_code(&(t->children[0]), scope);
            push(3);
            term_code(&(t->children[2]), scope);
            pop(5);
            cout << "sub $3, $5, $3" << endl;
            cout << "div $3, $4" << endl;
            cout << "mflo $3" << endl;
        }
    }
}


void test_code(Tree *t, string scope) {
    if (t->rule == "test expr LT expr") {
        expr_code(&(t->children[0]), scope);
        push(3);
        expr_code(&(t->children[2]), scope);
        pop(5);
        cout << "slt $3, $5, $3" << endl;
    } else if (t->rule == "test expr EQ expr") {
        expr_code(&(t->children[0]), scope);
        push(3);
        expr_code(&(t->children[2]), scope);
        pop(5);
        cout << "slt $6, $5, $3" << endl;
        cout << "slt $3, $3, $5" << endl;
        cout << "add $3, $3, $6" << endl;
        cout << "sub $3, $11, $3" << endl;
    } else if (t->rule == "test expr NE expr") {
        expr_code(&(t->children[0]), scope);
        push(3);
        expr_code(&(t->children[2]), scope);
        pop(5);
        cout << "slt $6, $5, $3" << endl;
        cout << "slt $3, $3, $5" << endl;
        cout << "add $3, $3, $6" << endl;
    } else if (t->rule == "test expr GE expr") {
        expr_code(&(t->children[0]), scope);
        push(3);
        expr_code(&(t->children[2]), scope);
        pop(5);
        cout << "slt $3, $5, $3" << endl;
        cout << "sub $3, $11, $3" << endl;
    } else if (t->rule == "test expr GT expr") {
        expr_code(&(t->children[0]), scope);
        push(3);
        expr_code(&(t->children[2]), scope);
        pop(5);
        cout << "slt $3, $3, $5" << endl;
    } else if (t->rule == "test expr LE expr") {
        expr_code(&(t->children[0]), scope);
        push(3);
        expr_code(&(t->children[2]), scope);
        pop(5);
        cout << "slt $3, $3, $5" << endl;
        cout << "sub $3, $11, $3" << endl;
    }
}

// Can be called on statements and statement node
int loop_label_counter = 0;
int if_label_counter = 0;
int delete_label_counter = 0;
void statements_code(Tree *t, string scope) {
    if (t->rule == "statements statements statement") {
        statements_code(&(t->children[0]), scope);
        statements_code(&(t->children[1]), scope);
    } else if (t->rule == "statement PRINTLN LPAREN expr RPAREN SEMI") {
        push(1);
        expr_code(&(t->children[2]), scope);
        cout << "add $1, $3, $0" << endl;
        push(31);
        cout << "jalr $10" << endl;
        pop(31);
        pop(1);
    } else if (t->rule == "statement lvalue BECOMES expr SEMI") {
        lvalue_code(&(t->children[0]), scope);
        push(3);
        expr_code(&(t->children[2]), scope);
        pop(5);
        cout << "sw $3, 0($5)" << endl;
    } else if (t->rule == "statement WHILE LPAREN test RPAREN LBRACE statements RBRACE") {
        int store_counter = loop_label_counter;
        ++loop_label_counter;
        cout << "loop" << store_counter << ":" << endl;
        test_code(&(t->children[2]), scope);
        cout << "beq $3, $0, endloop" << store_counter << endl;
        statements_code(&(t->children[5]), scope);
        cout << "beq $0, $0, loop" << store_counter << endl;
        cout << "endloop" << store_counter << ":" << endl;
    } else if (t->rule == "statement IF LPAREN test RPAREN LBRACE statements RBRACE ELSE LBRACE statements RBRACE") {
        int store_counter = if_label_counter;
        ++if_label_counter;
        test_code(&(t->children[2]), scope);
        cout << "beq $3, $0, else" << store_counter << endl;
        statements_code(&(t->children[5]), scope);
        cout << "beq $0, $0, endif" << store_counter << endl;
        cout << "else" << store_counter << ":" << endl;
        statements_code(&(t->children[9]), scope);
        cout << "endif" << store_counter << ":" << endl;
    } else if (t->rule == "statement DELETE LBRACK RBRACK expr SEMI") {
        int store_counter = delete_label_counter;
        ++delete_label_counter;
        expr_code(&(t->children[3]), scope);
        cout << "beq $3, $11, skipDelete" << store_counter << endl;
        cout << "add $1, $3, $0" << endl;
        cout << "lis $5" << endl;
        cout << ".word delete" << endl;
        push(31);
        cout << "jalr $5" << endl;
        pop(31);
        cout << "skipDelete" << store_counter << ":" << endl;
    }
}

void dcls_code(Tree *t, string scope) {
    if (t->rule == "dcls dcls dcl BECOMES NUM SEMI") {
        dcls_code(&(t->children[0]), scope);
        string value = t->children[3].tokens[1];
        cout << "lis $5" << endl;
        cout << ".word " << value << endl;
        push(5);
    } else if (t->rule == "dcls dcls dcl BECOMES NULL SEMI") {
        dcls_code(&(t->children[0]), scope);
        push(11);
    }
}

void code(Tree *t) {
    if (t->rule == "procedures procedure procedures") {
        string function_name = t->children[0].children[1].tokens[1];
        cout << "F" << function_name << ":" << endl;
        cout << "sub $29, $30, $4" << endl;
        dcls_code(&(t->children[0].children[6]), function_name);
        // Save used registers
        push(5);
        statements_code(&(t->children[0].children[7]), function_name);
        expr_code(&(t->children[0].children[9]), function_name);
        pop(5);
        cout << "add $30, $29, $4" << endl;
        cout << "jr $31" << endl;
        code(&(t->children[1]));
    }
}

Tree *main_ptr = NULL;
void find_main(Tree *t) {
    if (t->rule == "procedures main") {
        main_ptr = t;
    }
    if (!(is_leaf(t))) {
        int size = t->children.size();
        for (int i = 0; i < size; ++i) {
            find_main(&(t->children[i]));
        }
    }
}

void main_code(Tree *t) {
    if (t->rule == "procedures main") {
        cout << "main: " << endl;
        // Prologue starts
        cout << ".import print" << endl;
        cout << ".import init" << endl;
        cout << ".import new" << endl;
        cout << ".import delete" << endl;
        cout << "lis $4" << endl;
        cout << ".word 4" << endl;
        cout << "lis $10" << endl;
        cout << ".word print" << endl;
        cout << "lis $11" << endl;
        cout << ".word 1" << endl;
        // Call init to initialize the heap
        cout << "lis $3" << endl;
        cout << ".word init" << endl;
        push(1);
        push(2);
        push(31);
        push(29);
        if (SymbolTable["wain"]["wain@"].first == "int int") {
            cout << "add $2, $0, $0" << endl;
        }
        cout << "jalr $3" << endl;
        pop(29);
        pop(31);
        pop(2);
        pop(1);
        // Prologue ends
        cout << "sub $29, $30, $4" << endl;
        push(1);
        push(2);
        dcls_code(&(t->children[0].children[8]), "wain");
        statements_code(&(t->children[0].children[9]), "wain");
        expr_code(&(t->children[0].children[11]), "wain");
        out_of_scope("wain");
        cout << "jr $31" << endl;
    }
}

int main(void) {
    // Build ParseTree and SymbolTable
    build_parsetree(&ParseTree);
    build_symboltable(&(ParseTree.children[1]));

    // check if error has occured during the building process
    if (error_flag) {
        return 1;
    }
    expr_type_check(&(ParseTree.children[1]));

    // Start to output mips instructions
    find_main(&(ParseTree.children[1]));
    main_code(main_ptr);
    code(&(ParseTree.children[1]));

    return 0;
}

