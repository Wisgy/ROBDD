#pragma once
#include <cassert>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>
template <typename type, typename... Args>
std::shared_ptr<type> create(Args... args) {
    return std::shared_ptr<type>(new type{std::forward<Args>(args)...});
}
enum class OP { NONE, BOOL, VAR, OR, AND, IMPLY, EQUAL, EX, EG, EU };
class formula {
  public:
    formula(std::string &&name, OP op = OP::NONE,
            std::vector<std::shared_ptr<formula>> &&operands = {})
        : name(name), op(op), ops(operands) {}
    std::string name;
    OP op;
    std::vector<std::shared_ptr<formula>> ops;

    static std::shared_ptr<formula> TrueVal;
    static std::shared_ptr<formula> FalseVal;

    std::string print() {
        std::string tree = "\"" + name + "\"";
        if (ops.empty()) return tree;
        tree += ":[";
        for (auto op : ops) {
            tree += op->print();
            tree += ",";
        }
        tree.erase(tree.end() - 1);
        tree += "]";
        return "{" + tree + "}";
    }

    virtual std::shared_ptr<formula>
    create(std::vector<std::shared_ptr<formula>> &&ops) {
        assert(false);
    }
    virtual std::shared_ptr<formula> copy() { assert(false); }
    virtual bool run(std::set<std::string> &vars) { assert(false); }

    virtual bool operator==(const formula &other) {
        if (this->ops.size() != other.ops.size()) return false;
        for (unsigned i = 0; i < this->ops.size(); i++) {
            if (!(*this->ops[i] == *other.ops[i])) return false;
        }
        if (this->name == other.name)
            return true;
        else
            return false;
    }
    virtual bool operator!=(const formula &other) { assert(false); }
};
class CTL : public formula {
  public:
    CTL(std::string &&name, OP op, std::vector<std::shared_ptr<formula>> ops)
        : formula(std::move(name), op, std::move(ops)) {}
};
class EX final : public CTL {
  public:
    EX(std::shared_ptr<formula> oper) : CTL("EX", OP::EX, {oper}) {}
};
class EG final : public CTL {
  public:
    EG(std::shared_ptr<formula> oper) : CTL("EG", OP::EG, {oper}) {}
};
class EU final : public CTL {
  public:
    EU(std::shared_ptr<formula> lhs, std::shared_ptr<formula> rhs)
        : CTL("EU", OP::EU, {lhs, rhs}) {}
};
class Bin : public formula {
  public:
    Bin(std::shared_ptr<formula> lhs, std::shared_ptr<formula> rhs,
        std::string &&name, OP op)
        : formula(std::move(name), op, {lhs, rhs}) {}
};
class Or final : public Bin {
  public:
    Or(std::shared_ptr<formula> lhs, std::shared_ptr<formula> rhs)
        : Bin(lhs, rhs, "or", OP::OR) {}
    virtual std::shared_ptr<formula>
    create(std::vector<std::shared_ptr<formula>> &&ops) override {
        return std::shared_ptr<Or>(new Or(ops[0], ops[1]));
    }
    virtual std::shared_ptr<formula> copy() override {
        return std::shared_ptr<Or>(new Or(ops[0]->copy(), ops[1]->copy()));
    }
};
class And final : public Bin {
  public:
    And(std::shared_ptr<formula> lhs, std::shared_ptr<formula> rhs)
        : Bin(lhs, rhs, "and", OP::AND) {}
    virtual std::shared_ptr<formula>
    create(std::vector<std::shared_ptr<formula>> &&ops) override {
        return std::shared_ptr<And>(new And(ops[0], ops[1]));
    }
    virtual std::shared_ptr<formula> copy() override {
        return std::shared_ptr<And>(new And(ops[0]->copy(), ops[1]->copy()));
    }
};
class Imply final : public Bin {
  public:
    Imply(std::shared_ptr<formula> lhs, std::shared_ptr<formula> rhs)
        : Bin(lhs, rhs, "->", OP::IMPLY) {}
    virtual std::shared_ptr<formula>
    create(std::vector<std::shared_ptr<formula>> &&ops) override {
        return std::shared_ptr<Imply>(new Imply(ops[0], ops[1]));
    }
    virtual std::shared_ptr<formula> copy() override {
        return std::shared_ptr<Imply>(
            new Imply(ops[0]->copy(), ops[1]->copy()));
    }
};
class Equal final : public Bin {
  public:
    Equal(std::shared_ptr<formula> lhs, std::shared_ptr<formula> rhs)
        : Bin(lhs, rhs, "=", OP::EQUAL) {}
    virtual std::shared_ptr<formula>
    create(std::vector<std::shared_ptr<formula>> &&ops) override {
        return std::shared_ptr<Equal>(new Equal(ops[0], ops[1]));
    }
    virtual std::shared_ptr<formula> copy() override {
        return std::shared_ptr<Equal>(
            new Equal(ops[0]->copy(), ops[1]->copy()));
    }
};
class Var final : public formula {
  public:
    Var(std::string &&name)
        : id(std::string(name)),
          formula(std::move(name), OP::VAR,
                  {formula::TrueVal, formula::FalseVal}) {}
    Var(std::string &&name, std::vector<std::shared_ptr<formula>> &&ops)
        : id(std::string(name)),
          formula(std::move(name), OP::VAR, std::move(ops)) {}
    std::string id;
    virtual std::shared_ptr<formula>
    create(std::vector<std::shared_ptr<formula>> &&ops) override {
        auto name = this->id;
        auto var = std::shared_ptr<Var>(new Var(std::move(name)));
        var->ops = ops;
        return var;
    }
    virtual std::shared_ptr<formula> copy() override {
        auto name = this->id;
        auto copy_var = std::shared_ptr<Var>(new Var(std::move(name)));
        copy_var->ops = this->ops;
        return copy_var;
    }
    virtual bool run(std::set<std::string> &vars) override {
        if (vars.find(this->name) == vars.end()) // the set of vars doesn't have
                                                 // the current var so -> false
            return ops[1]->run(vars);
        else
            return ops[0]->run(vars);
    }
};
class True final : public formula {
  public:
    True() : formula("1", OP::BOOL) {}
    virtual std::shared_ptr<formula>
    create(std::vector<std::shared_ptr<formula>> &&ops) override {
        assert(false);
    }
    virtual std::shared_ptr<formula> copy() override {
        return formula::TrueVal;
    }
    virtual bool run(std::set<std::string> &vars) override { return true; }
};
class False final : public formula {
  public:
    False() : formula("0", OP::BOOL) {}
    virtual std::shared_ptr<formula>
    create(std::vector<std::shared_ptr<formula>> &&ops) override {
        assert(false);
    }
    virtual std::shared_ptr<formula> copy() override {
        return formula::FalseVal;
    }
    virtual bool run(std::set<std::string> &vars) override { return false; }
};