#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <array>
#include <map>
#include <set>
#include <utility>
#include <algorithm>

using namespace std;

struct State {
    int state_num;
    string input;
    string type;
    int next_state;
};

struct Grammar {
    int id;
    string LHS;
    vector<string> RHS;
};

struct Tree {
    Grammar g;
    vector<Tree> children;
};

vector<string> alphabets;
vector<string> non_terminals;
map<string, int> n_index;
vector<Grammar> grammars;
string start;
int total_states;
vector<vector<State>> LR_machine;
vector<pair<string, string>> sequence;
vector<pair<string, string>> unread;
vector<string> stack;
vector<Grammar> output;
vector<Grammar> output_r;


void build_tree(Tree *node, int *index) {
    node->g = output_r[*index];
    int children_size = output_r[*index].RHS.size();
    node->children.resize(children_size);
    *index = *index + 1;
    if (output_r[*index - 1].id != -1) {
        for (int i = 0; i < children_size; ++i) {
            build_tree(&(node->children[i]), index);
        }
    }
}
    
void preorder_print(Tree *node) {
    cout << node->g.LHS;
    for (auto &ele : node->g.RHS) {
        cout << " " << ele;
    }
    if (node->g.id != -1) {
        cout << endl;
    }
    for (auto it = node->children.rbegin(); it != node->children.rend(); ++it) {
        preorder_print(&(*it));
    }
}

int main(void) {
    int i;
    string s;
    ifstream file("wlp4input.txt");
    // Build up CFG
    if (file >> i) {
        int j = 0;
        while (j < i && file >> s) {
            alphabets.emplace_back(s);
            ++j;
        }
    } else {
        cerr << "Invalid Input" << endl;
    }
    if (file >> i) {
        int j = 0;
        while (j < i && file >> s) {
            non_terminals.emplace_back(s);
            n_index[s] = j;
            ++j;
        }
    } else {
        cerr << "Invalid Input" << endl;
    }
    file >> start;
    if (file >> i) {
        int j = 0;
        getline(file, s);
        while (j < i && getline(file, s)) {
            Grammar g;
            stringstream iss(s);
            string word;
            iss >> word;
            g.id = j;
            g.LHS = word;
            while (iss >> word) {
                g.RHS.emplace_back(word);
            }
            grammars.emplace_back(g);
            ++j;
        }
    } else {
        cerr << "Invalid Input" << endl;
    }
    file >> total_states;
    if (file >> i) {
        LR_machine.resize(total_states);
        int j = 0;
        getline(file, s);
        while (j < i && getline(file, s)) {
            State state;
            stringstream iss(s);
            string word;
            iss >> word;
            state.state_num = stoi(word);
            iss >> word;
            state.input = word;
            iss >> word;
            state.type = word;
            iss >> word;
            state.next_state = stoi(word);
            LR_machine[state.state_num].emplace_back(state);
            ++j;
        }
    } else {
        cerr << "Invalid Input" << endl;
    }
    // At this point, all wlp4 grammars have been stored

    // Stores all the tokens
    string line;
    pair<string, string> token = make_pair("BOF", "BOF");
    sequence.emplace_back(token);
    while(getline(cin, line)) {
       stringstream iss(line);
       iss >> token.first;
       iss >> token.second;
       sequence.emplace_back(token);
    }
    token = make_pair("EOF", "EOF");
    sequence.emplace_back(token);

    // reverse sequence
    for (auto it = sequence.rbegin(); it != sequence.rend(); ++it) {
        unread.emplace_back(*it);
    }

    // Process starts
    stack.emplace_back("0");
    int counter = 0;
    while (unread.size() > 0) {
        int current_state = stoi(stack.back());
        string token = unread.back().first;
        bool transition_exists = 0;
        for (auto &transition : LR_machine[current_state]) {
            if (transition.type == "reduce" && token == transition.input) {
                transition_exists = 1;
                int rule = transition.next_state;
                for (auto it = grammars[rule].RHS.rbegin(); it != grammars[rule].RHS.rend(); ++it) {
                    stack.pop_back();
                    if (stack.back() != *it) {
                        cerr << "ERROR at " << counter << endl;
                        return 1;
                    }
                    stack.pop_back();
                }
                output.emplace_back(grammars[rule]);
                current_state = stoi(stack.back());
                bool shift_exist = 0;
                for (auto &tr : LR_machine[current_state]) {
                    if (tr.type == "shift" && tr.input == grammars[rule].LHS) {
                        stack.emplace_back(tr.input);
                        stack.emplace_back(to_string(tr.next_state));
                        shift_exist = 1;
                        break;
                    }
                }
                if (!shift_exist) {
                    cerr << "ERROR at " << counter << endl;
                    return 1;
                }
            }
            else if (transition.type == "shift" && token == transition.input) {
                Grammar gg;
                gg.id = -1;
                gg.LHS = unread.back().first;
                gg.RHS.emplace_back(unread.back().second);
                output.emplace_back(gg);
                transition_exists = 1;
                ++counter;
                string temp = unread.back().first;
                unread.pop_back();
                stack.emplace_back(temp);
                stack.emplace_back(to_string(transition.next_state));
            }
        }
        if (!transition_exists) {
            cerr << "ERROR at " << counter << endl;
            return 1;
        }
    }
    int ss = grammars[n_index[start]].RHS.size();
    bool start_remain = 1;
    for (int i = 0; i < ss; ++i) {
        start_remain = start_remain && (grammars[n_index[start]].RHS[i] == stack[2*i + 1]);
    }
    if (stack[0] == "0" && start_remain) {
        output.emplace_back(grammars[n_index[start]]);
    } else {
        cerr << "ERROR at " << counter << endl;
        return 1;
    }
    
    // Reverse order
    for (auto it = output.rbegin(); it != output.rend(); ++it) {
        output_r.emplace_back(*it);
    }
    output.clear();

    // Build Tree
    Tree root;
    int num = 0;
    build_tree(&root, &num);

    // Print in preorder traversal
    preorder_print(&root);

    return 0;
}
