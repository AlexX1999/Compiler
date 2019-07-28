#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include <set>
#include <vector>

using namespace std;


struct Tree {
    string rule;
    vector<string> tokens;
    vector<Tree> children;

    friend ostream& operator<<(ostream &out, Tree &t);
};

bool error_flag = 0;
Tree ParseTree;
map<string, map<string, string>> SymbolTable;
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
            return it->second;
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
                return it->second;
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
                SymbolTable[scope][name] = "int*";
            } else if (t->children[0].children.size() == 1) {
                SymbolTable[scope][name] = "int";
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


// Require the LHS of the rule of the input node to be procedures
void build_symboltable(Tree *t) {
    if (t->tokens[0] != "procedures") {
        cerr << "Warning: The rule of input node has to have procudures as LHS" << endl;
    }
    if (t->tokens[1] == "main") {
        string scope = "wain";
        string para1_type = get_type(&(t->children[0].children[3]));
        string para2_type = get_type(&(t->children[0].children[5]));
        string para_type = para1_type + " " + para2_type;
        SymbolTable["wain"]["wain@"] = para_type;
        add_variable_to_symboltable(&(t->children[0].children[3]), scope);
        add_variable_to_symboltable(&(t->children[0].children[5]), scope);
        add_variable_to_symboltable(&(t->children[0].children[8]), scope);
        check_for_use(&(t->children[0].children[9]), scope);
        check_for_use(&(t->children[0].children[11]), scope);
    } else {
        string scope = t->children[0].children[1].tokens[1];
        if (SymbolTable.find(scope) != SymbolTable.end()) {
            cerr << "ERROR: Duplication function declaration: " << scope << endl;
            error_flag = 1;
            return;
        }
        if (t->children[0].children[3].tokens[0] == "params" && t->children[0].children[3].children.size() == 0) {
            SymbolTable[scope][scope] = "";
        } else if (t->children[0].children[3].tokens[0] == "params" && t->children[0].children[3].children.size() == 1) {
            // Note that to distinguish function signiture and variable name
            // We append '@' to the function name
            string function_name = scope + "@";
            SymbolTable[scope][function_name] = get_param(&(t->children[0].children[3].children[0]));
            add_param_to_symboltable(&(t->children[0].children[3].children[0]), scope);
        }
        add_variable_to_symboltable(&(t->children[0].children[6]), scope);
        check_for_use(&(t->children[0].children[7]), scope);
        check_for_use(&(t->children[0].children[9]), scope);
        build_symboltable(&(t->children[1]));
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

int main(void) {
    // Build ParseTree and SymbolTable
    build_parsetree(&ParseTree);
    build_symboltable(&(ParseTree.children[1]));
    // check if error has occured during the building process
    if (error_flag) {
        return 1;
    }
    expr_type_check(&(ParseTree.children[1])); 
    return 0;
}

