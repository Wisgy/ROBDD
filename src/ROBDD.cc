#include "Formula.hh"
#include "Parser.hh"
#include <cassert>
#include <cmath>
#include <fstream>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <vector>
using namespace std;
bool operator==(shared_ptr<formula> lhs, shared_ptr<formula> rhs) {
    return *lhs == *rhs;
}
struct State {
    list<bool> idx;
    set<string> vars;
    set<shared_ptr<State>> pre;
    set<shared_ptr<State>> suc;
};
map<string, shared_ptr<State>> states;

shared_ptr<formula> formula::TrueVal = make_shared<True>();
shared_ptr<formula> formula::FalseVal = make_shared<False>();

template <class type, class base> bool is_a(shared_ptr<base> node) {
    if (dynamic_pointer_cast<type>(node))
        return true;
    else
        return false;
}
template <class type, class base> shared_ptr<type> as_a(shared_ptr<base> node) {
    assert(is_a<type>(node));
    return dynamic_pointer_cast<type>(node);
}
inline bool is_bool(shared_ptr<formula> node) {
    if (is_a<True>(node) || is_a<False>(node))
        return true;
    else
        return false;
}
inline bool is_CTL(shared_ptr<formula> node) {
    if (is_a<EG>(node) || is_a<EX>(node) || is_a<EU>(node))
        return true;
    else
        return false;
}
shared_ptr<formula> ComputeEG(shared_ptr<formula> node);
shared_ptr<formula> ComputeEX(shared_ptr<formula> node);
shared_ptr<formula> ComputeEU(shared_ptr<formula> node);
shared_ptr<formula> Compute(shared_ptr<formula> node) {
    if (is_a<EX>(node)) return ComputeEX(node);
    if (is_a<EU>(node)) return ComputeEU(node);
    if (is_CTL(node)) return ComputeEG(node);
    if (is_a<True>(node)) return formula::TrueVal;
    if (is_a<False>(node)) return formula::FalseVal;
    if (is_a<Var>(node)) return node;
    shared_ptr<formula> lhs = Compute(node->ops[0]);
    shared_ptr<formula> rhs = Compute(node->ops[1]);
    OP op = node->op;
    if (is_bool(lhs) && is_bool(rhs)) {
        switch (op) {
            case OP::AND:
                if (is_a<True>(lhs) && is_a<True>(rhs))
                    return formula::TrueVal;
                else
                    return formula::FalseVal;
                break;
            case OP::OR:
                if (is_a<True>(lhs) || is_a<True>(rhs))
                    return formula::TrueVal;
                else
                    return formula::FalseVal;
                break;
            case OP::IMPLY:
                if (is_a<True>(lhs) && is_a<False>(rhs))
                    return formula::FalseVal;
                else
                    return formula::TrueVal;
                break;
            case OP::EQUAL:
                if (is_a<True>(lhs) == is_a<True>(rhs))
                    return formula::TrueVal;
                else
                    return formula::FalseVal;
                break;
            case OP::NONE:
                assert(false);
            default:
                assert(false);
                break;
        }
    } else if (is_bool(lhs) || (!is_bool(rhs) && lhs->name > rhs->name)) {
        auto new_lhs = Compute(node->create({lhs->copy(), rhs->ops[0]}));
        auto new_rhs = Compute(node->create({lhs->copy(), rhs->ops[1]}));
        if (new_lhs == new_rhs) return new_lhs;
        return rhs->create({new_lhs, new_rhs});
    } else if (is_bool(rhs) || lhs->name < rhs->name) {
        auto new_lhs = Compute(node->create({lhs->ops[0], rhs->copy()}));
        auto new_rhs = Compute(node->create({lhs->ops[1], rhs->copy()}));
        if (new_lhs == new_rhs) return new_lhs;
        return lhs->create({new_lhs, new_rhs});
    } else {
        auto new_lhs = Compute(node->create({lhs->ops[0], rhs->ops[0]}));
        auto new_rhs = Compute(node->create({lhs->ops[1], rhs->ops[1]}));
        if (new_lhs == new_rhs) return new_lhs;
        return lhs->create({new_lhs, new_rhs});
    }
}

void LoadDiagram(ifstream &in) {
    string state_str;
    auto get_state = [](string &name) {
        if (states.find(name) == states.end())
            return states[name] = make_shared<State>();
        else
            return states[name];
    };
    auto new_istream = [](istringstream &in) {
        string names;
        getline(in, names, ',');
        istringstream name_iss(names);
        return name_iss;
    };
    while (getline(in, state_str)) {
        istringstream out(state_str);

        string name;
        getline(out, name, ',');
        shared_ptr<State> node;
        node = get_state(name);

        string tmp_name;
        auto vars_in = new_istream(out);
        while (vars_in >> tmp_name)
            node->vars.insert(tmp_name);

        auto pres_in = new_istream(out);
        while (pres_in >> tmp_name)
            node->pre.insert(get_state(tmp_name));

        auto sucs_in = new_istream(out);
        while (sucs_in >> tmp_name)
            node->suc.insert(get_state(tmp_name));
    }
    int idx = 0;
    for (auto iter = states.begin(); iter != states.end(); iter++, idx++) {
        auto copy_idx = idx;
        for (int times = 1; times < states.size(); times *= 2) {
            iter->second->idx.push_front(copy_idx % 2);
            copy_idx /= 2;
        }
    }
}

bool StateInPhi(shared_ptr<State> s, shared_ptr<formula> phi) {
    return phi->run(s->vars);
}

shared_ptr<formula> init_tree(int depth, int idx) {
    string basic_name = "a";
    if (depth == 1) {
        std::vector<shared_ptr<formula>> tmp = {formula::FalseVal,
                                                formula::FalseVal};
        return create<Var>(basic_name + to_string(idx), std::move(tmp));
    } else {
        std::vector<shared_ptr<formula>> tmp = {init_tree(depth - 1, idx + 1),
                                                init_tree(depth - 1, idx + 1)};
        return create<Var>(basic_name + to_string(idx), std::move(tmp));
    }
}
void modify_tree(shared_ptr<formula> node, list<bool> &idx) {
    auto flag = idx.front();
    idx.pop_front();
    if (flag) {
        if (is_bool(node->ops[0]))
            node->ops[0] = formula::TrueVal;
        else
            modify_tree(node->ops[0], idx);
    } else {
        if (is_bool(node->ops[1]))
            node->ops[1] = formula::TrueVal;
        else
            modify_tree(node->ops[1], idx);
    }
}
shared_ptr<formula> prune_tree(shared_ptr<formula> node) {
    if (is_bool(node)) return node;
    node->ops[0] = prune_tree(node->ops[0]);
    node->ops[1] = prune_tree(node->ops[1]);
    if (node->ops[0] == node->ops[1])
        return node->ops[0];
    else
        return node;
}
shared_ptr<formula> Compute(set<shared_ptr<State>> s) {
    int depth = log2(states.size());
    depth = depth == (int)log2(states.size() - 1) ? depth + 1 : depth;
    auto root = init_tree(depth, 0);
    int idx = 0;
    for (auto iter = states.begin(); iter != states.end(); iter++, idx++) {
        if (s.count(iter->second)) {
            auto idx = iter->second->idx;
            modify_tree(root, idx);
        }
    }
    return prune_tree(root);
}
shared_ptr<formula> ComputeEG(shared_ptr<formula> node) {
    auto phi = Compute(node->ops[0]);
    set<shared_ptr<State>> T;
    for (auto iter = states.begin(); iter != states.end(); iter++) {
        if (StateInPhi(iter->second, phi)) T.insert(iter->second);
    }
    set<shared_ptr<State>> U = T;
    set<shared_ptr<State>> V;
    while (1) {
        for (auto s : T) {
            for (auto t : s->suc) {
                if (U.count(t)) {
                    V.insert(s);
                }
            }
        }
        auto tn = Compute(U);
        auto tn1 = Compute(create<And>(tn->copy(), Compute(V)));
        U = V;
        V.clear();
        if (*tn == *tn1) return tn;
    }
}
shared_ptr<formula> ComputeEX(shared_ptr<formula> node) {
    auto phi = Compute(node->ops[0]);
    set<shared_ptr<State>> U;
    set<shared_ptr<State>> V;
    for (auto iter = states.begin(); iter != states.end(); iter++) {
        if (StateInPhi(iter->second, phi)) U.insert(iter->second);
    }
    for (auto iter = states.begin(); iter != states.end(); iter++) {
        for (auto suc : iter->second->suc) {
            if (U.count(suc)) V.insert(iter->second);
        }
    }
    return Compute(V);
}
shared_ptr<formula> ComputeEU(shared_ptr<formula> node) {
    auto phi1 = Compute(node->ops[0]);
    auto phi2 = Compute(node->ops[1]);
    set<shared_ptr<State>> U1;
    set<shared_ptr<State>> U2;
    for (auto iter = states.begin(); iter != states.end(); iter++) {
        if (StateInPhi(iter->second, phi1)) U1.insert(iter->second);
        if (StateInPhi(iter->second, phi2)) U2.insert(iter->second);
    }
    set<shared_ptr<State>> V;
    for (auto s : U1) {
        for (auto t : s->suc) {
            if (U2.count(t)) {
                V.insert(s);
            }
        }
    }
    set<shared_ptr<State>> tmp = V;
    while (1) {
        for (auto s : U1) {
            for (auto t : s->suc) {
                if (V.count(t)) {
                    V.insert(s);
                }
            }
        }
        if (tmp == V)
            return Compute(create<Or>(Compute(V), Compute(U2)));
        else
            tmp = V;
    }
}
int main(int argv, char *argc[]) {
    if (argv != 6) assert(false);
    auto input_file_path = [](char *path, char *num) -> string {
        string input = "input";
        string suffix = ".txt";
        return path + input + num + suffix;
    };
    auto output_file_path = [](char *path, char *num) -> string {
        string output = "output";
        string suffix = ".json";
        return path + output + num + suffix;
    };
    auto diagram_file_path = [](char *path, char *num) -> string {
        string output = "diagram";
        string suffix = ".txt";
        return path + output + num + suffix;
    };
    ifstream diagram(diagram_file_path(argc[5], argc[2]));
    ifstream file(input_file_path(argc[3], argc[1]));
    Parser parser(file);
    LoadDiagram(diagram);
    auto decision_root = Compute(parser.root);
    ofstream output(output_file_path(argc[4], argc[1]));
    output << decision_root->print() << endl;
}