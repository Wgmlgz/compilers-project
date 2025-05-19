#ifndef COMPILER_HPP
#define COMPILER_HPP

#include <exception>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

using string = std::string;

enum class Type { I32, STR, UNKNOWN };

struct VariableInfo {
  Type type;
  int offset;
  VariableInfo(Type t, int o) : type(t), offset(o) {}
};

struct Ctx {
  std::string prefix = R"(
jal x0, main

# BEGIN MACROS
print_i32:
  addi x10, x0, 10
  addi x11, x0, 1023
  addi x12, x1, 0
  addi x13, x0, 0
  addi x14, x11, 0
  bge  x12, x0, producer_loop
  addi x13, x0, 1
  sub x12, x0, x12

producer_loop:
  div x15, x12, x10
  rem x16, x12, x10
  addi x20, x16, 48
  sw x14, 0, x20
  addi x14, x14, -1
  addi x12, x15, 0
  bne x12, x0, producer_loop

  beq x13, x0, after_minus
  addi x20, x0, 45
  ewrite x20

after_minus:
  addi x14, x14, 1
  lw x20, x14, 0
  ewrite x20
  bne x14, x11, after_minus

  addi x20, x0, 10
  ewrite x20
  jalr x0, x31, 0

print_str:
  lw x10, x1, 0 # load len to x10
  addi x1, x1, 1 # move x1 ptr to string begin 
  addi x3, x0, 1 # load 1 to x3
next_char:
  beq x10, x0, print_str_end # we are done
  lw x2, x1, 0 # load char
  ewrite x2
  addi x1, x1, 1
  sub x10, x10, x3
  jal x0, next_char
print_str_end:
  jalr x0, x31, 0 # return

print_char:
  ewrite x1
  jalr x0, x31, 0 # return

len_str:
  lw x1, x1, 0 # load len to x1
  jalr x0, x31, 0 # return

# END MACROS

)";

  std::string strings = R"(
# BEGIN STRINGS
)";

  std::string res = R"(
# BEGIN MAIN
main:
)";
  // x1 .. xxx - general
  // x31 ret address
  // x30 arg
  int usedReg = 0;
  int stack_begin = 0x800;
  std::vector<std::pair<int, std::unordered_map<string, VariableInfo>>> vars = {
      {stack_begin, {}}};

  std::vector<std::string> breakable;
  std::vector<std::string> continuable;

  int id = 0;
};

class Node {
 public:
  virtual ~Node() = default;
  virtual void gen() const = 0;
  virtual Type typeCheck() const { return Type::UNKNOWN; }
  virtual void print(int indent = 0) const = 0;
  void printHeader(const int indent = 0, const std::string &id = "",
                   const std::string &extra = "") const {
    std::cerr << std::string(indent, ' ') << id;
    if (extra.size()) {
      std::cerr << "(" << extra << ")";
    }
    std::cerr << std::endl;
  }
};

class NumberNode : public Node {
 public:
  int value;
  NumberNode(int val) : value(val) {}
  void gen() const override;
  Type typeCheck() const override { return Type::I32; }
  void print(int indent = 0) const override {
    printHeader(indent, "IntLiteral", std::to_string(value));
  }
};

class StringNode : public Node {
 public:
  string value;
  StringNode(string val) : value(val) {}
  void gen() const override;
  Type typeCheck() const override { return Type::STR; }
  void print(int indent = 0) const override {
    printHeader(indent, "StringLiteral", value);
  }
};

class VariableNode : public Node {
 public:
  string name;
  VariableNode(const string &n) : name(n) {}
  void gen() const override;
  Type typeCheck() const override;
  void print(int indent = 0) const override {
    printHeader(indent, "Variable", name);
  }
};

class BinaryNode : public Node {
 public:
  string op;
  std::unique_ptr<Node> left;
  std::unique_ptr<Node> right;

  BinaryNode(const string &o, Node *l, Node *r) : op(o), left(l), right(r) {}

  void gen() const override;
  Type typeCheck() const override;

  void print(int indent = 0) const override {
    printHeader(indent, "BinaryOp", op);
    left->print(indent + 2);
    right->print(indent + 2);
  }
};

class UnaryNode : public Node {
 public:
  string op;
  std::unique_ptr<Node> right;

  UnaryNode(const string &o, Node *r) : op(o), right(r) {}

  void gen() const override;
  Type typeCheck() const override;

  void print(int indent = 0) const override {
    printHeader(indent, "UnaryOp", op);
    right->print(indent + 2);
  }
};

class AssignNode : public Node {
 public:
  string name;
  std::unique_ptr<Node> expression;

  AssignNode(const string &n, Node *expr) : name(n), expression(expr) {}
  void gen() const override;
  Type typeCheck() const override;
  void print(int indent = 0) const override {
    printHeader(indent, "Assignment", name);
    expression->print(indent + 2);
  }
};

class VarDeclNode : public Node {
 public:
  string name;
  Type declaredType;
  std::unique_ptr<Node> expression;

  VarDeclNode(const string &n, Type type, Node *expr)
      : name(n), declaredType(type), expression(expr) {}

  void gen() const override;
  Type typeCheck() const override;
  void print(int indent = 0) const override {
    printHeader(indent, "VarDecl", name);
    expression->print(indent + 2);
  }
};

class IfNode : public Node {
 public:
  std::unique_ptr<Node> condition;
  std::unique_ptr<Node> thenBlock;
  std::unique_ptr<Node> elseBlock;

  IfNode(Node *cond, Node *thenB, Node *elseB = nullptr)
      : condition(cond), thenBlock(thenB), elseBlock(elseB) {}

  void gen() const override;
  Type typeCheck() const override;

  void print(int indent = 0) const override {
    printHeader(indent, "IfStatement");
    printHeader(indent + 2, "Condition");
    condition->print(indent + 4);
    printHeader(indent + 2, "Then");
    thenBlock->print(indent + 4);
    if (elseBlock) {
      printHeader(indent + 2, "Else");
      elseBlock->print(indent + 4);
    }
  }
};

class BlockNode : public Node {
 public:
  std::vector<std::unique_ptr<Node>> statements;
  bool returnsValue;

  BlockNode() : returnsValue(false) {}

  void gen() const override;
  Type typeCheck() const override;

  void addStatement(Node *statement) { statements.emplace_back(statement); }

  void setReturnsValue(bool value) { returnsValue = value; }

  void print(int indent = 0) const override {
    printHeader(indent, "Block");
    for (const auto &stmt : statements) {
      stmt->print(indent + 2);
    }
  }
};

class LoopNode : public Node {
 public:
  std::unique_ptr<Node> condition;
  std::unique_ptr<Node> init;
  std::unique_ptr<Node> after_loop;
  std::unique_ptr<Node> block;

  LoopNode(Node *cond, Node *blk, Node *ini = nullptr, Node *after = nullptr)
      : condition(cond), block(blk), init(ini), after_loop(after) {}
  void gen() const override;
  Type typeCheck() const override;
  void print(int indent = 0) const override {
    printHeader(indent, "WhileLoop");
    printHeader(indent + 2, "Condition");
    condition->print(indent + 4);
    printHeader(indent + 2, "Body");
    block->print(indent + 4);
  }
};

class MacroNode : public Node {
 public:
  std::string name;
  std::unique_ptr<Node> arg;

  MacroNode(const std::string &n, Node *a) : name(n), arg(a) {}
  void gen() const override;
  Type typeCheck() const override;
  void print(int indent = 0) const override {
    printHeader(indent, "Macro", name);
    arg->print(indent + 2);
  }
};

class BreakNode : public Node {
 public:
  BreakNode() {}
  void gen() const override;
  Type typeCheck() const override;
  void print(int indent = 0) const override { printHeader(indent, "Break"); }
};

class ContinueNode : public Node {
 public:
  ContinueNode() {}
  void gen() const override;
  Type typeCheck() const override;
  void print(int indent = 0) const override { printHeader(indent, "Continue"); }
};

std::string compile(BlockNode *);

#endif  // COMPILER_HPP