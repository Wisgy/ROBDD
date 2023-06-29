#pragma once
#include "Formula.hh"
#include <cassert>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <utility>
#include <vector>
using namespace std;
class Parser {
  public:
    Parser(ifstream &is) { root = Equ(is); }
    shared_ptr<formula> root;
    // vector<string> stack;
    string token;

  private:
    string not_id = "¬";
    string and_id = "∧";
    string or_id = "∨";
    string imply_id = "→";
    string equal_id = "↔";
    bool next_token(ifstream &is) {
        if (is >> token) {
            return true;
        } else
            return false;
    }
    shared_ptr<formula> Equ(ifstream &is) {
        auto I = Imp(is);
        auto E_ = Equ_(is, I);
        if (E_ != nullptr) return E_;
        return I;
    }
    shared_ptr<formula> Equ_(ifstream &is, shared_ptr<formula> former) {
        if (token.empty())
            if (!next_token(is)) return nullptr;
        if (token == equal_id) {
            token.clear();
            auto I = Imp(is);
            auto E = create<Equal>(former, I);
            auto E_ = Equ_(is, E);
            if (E_ != nullptr) {
                E_->ops[0] = I;
                return E_;
            } else
                return E;
        }
        return nullptr;
    }
    shared_ptr<formula> Imp(ifstream &is) {
        auto D = Disj(is);
        auto I_ = Imp_(is, D);
        if (I_ != nullptr) return I_;
        return D;
    }
    shared_ptr<formula> Imp_(ifstream &is, shared_ptr<formula> former) {
        if (token.empty())
            if (!next_token(is)) return nullptr;
        if (token == imply_id) {
            token.clear();
            auto D = Disj(is);
            auto I = create<Imply>(former, D);
            auto I_ = Imp_(is, I);
            if (I_ != nullptr) return I_;
            return I;
        }
        return nullptr;
    }
    shared_ptr<formula> Disj(ifstream &is) {
        auto C = Conj(is);
        auto D_ = Disj_(is, C);
        if (D_ != nullptr) return D_;
        return C;
    }
    shared_ptr<formula> Disj_(ifstream &is, shared_ptr<formula> former) {
        if (token.empty())
            if (!next_token(is)) return nullptr;
        if (token == or_id) {
            token.clear();
            auto C = Conj(is);
            auto D = create<Or>(former, C);
            auto D_ = Disj_(is, D);
            if (D_ != nullptr) return D_;
            return D;
        }
        return nullptr;
    }
    shared_ptr<formula> Conj(ifstream &is) {
        auto S = Single(is);
        auto C_ = Conj_(is, S);
        if (C_ != nullptr) return C_;
        return S;
    }
    shared_ptr<formula> Conj_(ifstream &is, shared_ptr<formula> former) {
        if (token.empty())
            if (!next_token(is)) return nullptr;
        if (token == and_id) {
            token.clear();
            auto S = Single(is);
            auto C = create<And>(former, S);
            auto C_ = Conj_(is, C);
            if (C_ != nullptr) return C_;
            return C;
        }
        return nullptr;
    }
    shared_ptr<formula> Single(ifstream &is) {
        if (token.empty() && !next_token(is)) return nullptr;
        if (token == not_id) {
            token.clear();
            return create<Imply>(Single(is), formula::FalseVal);
        } else if (token == "EG") {
            token.clear();
            return create<EG>(Single(is));
        } else if (token == "(") {
            token.clear();
            auto E = Equ(is);
            if (token == ")")
                token.clear();
            else
                assert(false);
            return E;
        } else {
            auto name = token;
            token.clear();
            return create<Var>(move(name));
        }
    }
};