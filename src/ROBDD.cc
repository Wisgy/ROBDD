#include "Formula.hh"
#include "Parser.hh"
#include <cassert>
#include <fstream>
#include <memory>
#include <vector>
using namespace std;

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

shared_ptr<formula> Compute(shared_ptr<formula> node) {
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
int main(int argv, char *argc[]) {
    if (argv != 3) assert(false);
    ifstream file(argc[1]);
    Parser parser(file);
    auto decision_root = Compute(parser.root);
    ofstream output(argc[2]);
    output << decision_root->print() << endl;
}