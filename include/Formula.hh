#pragma once
#include <cassert>
#include <memory>
#include <string>
#include <utility>
#include <vector>
template <typename type, typename... Args>
std::shared_ptr<type> create(Args... args) {
    return std::shared_ptr<type>(new type{std::forward<Args>(args)...});
}
enum class OP { NONE, VAR, OR, AND, IMPLY, EQUAL };
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

    virtual bool operator==(const formula &other) {
        if (this->ops.size() != other.ops.size()) return false;
        for (unsigned i = 0; i < this->ops.size(); i++) {
            if (this->ops[i] != other.ops[i]) return false;
        }
        if (this->name == other.name)
            return true;
        else
            return false;
    }
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
        return std::shared_ptr<Var>(new Var(std::move(name)));
    }
};
class True final : public formula {
  public:
    True() : formula("1") {}
    virtual std::shared_ptr<formula>
    create(std::vector<std::shared_ptr<formula>> &&ops) override {
        assert(false);
    }
    virtual std::shared_ptr<formula> copy() override {
        return formula::TrueVal;
    }
};
class False final : public formula {
  public:
    False() : formula("0") {}
    virtual std::shared_ptr<formula>
    create(std::vector<std::shared_ptr<formula>> &&ops) override {
        assert(false);
    }
    virtual std::shared_ptr<formula> copy() override {
        return formula::FalseVal;
    }
};