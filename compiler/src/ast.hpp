#ifndef ast_HPP
#define ast_HPP

#include <iostream>
#include <string>
#include <variant>
#include <vector>
#include <unordered_map>
#include <memory>
#include <exception>

using string = std::string;

// ArrowRust specific types
enum class Type {
  I32,
  STR,
  BOOL,
  UNIT,
  UNKNOWN
};

struct VariableInfo {
  Type type;
  bool mutable_;
  VariableInfo() : type(Type::UNKNOWN), mutable_(false) {}
  VariableInfo(Type t, bool m) : type(t), mutable_(m) {}
};

struct UndefinedType {};

using Value = std::variant<int, bool, string, UndefinedType>;

// Exception used for early return from function
class ReturnValue : public std::exception {
public:
  Value value;
  ReturnValue(Value val) : value(val) {}
  
  const char* what() const noexcept override {
    return "Return statement executed";
  }
};

class Node {
 public:
  virtual ~Node() = default;
  virtual void print(int indent = 0) const = 0;
};

class BoolNode : public Node {
 public:
  bool value;
  BoolNode(bool val) : value(val) {}
  void print(int indent = 0) const override {
    std::cout << std::string(indent, ' ') << "BoolLiteral(" << (value ? "true" : "false") << ")" << std::endl;
  }
};

class NumberNode : public Node {
 public:
  int value;
  NumberNode(int val) : value(val) {}
  void print(int indent = 0) const override {
    std::cout << std::string(indent, ' ') << "IntLiteral(" << value << ")" << std::endl;
  }
};

class UnitNode : public Node {
 public:
  UnitNode() {}
  void print(int indent = 0) const override {
    std::cout << std::string(indent, ' ') << "Unit()" << std::endl;
  }
};

class StringNode : public Node {
 public:
  string value;
  StringNode(string val) : value(val) {}
  void print(int indent = 0) const override {
    std::cout << std::string(indent, ' ') << "StringLiteral(\"" << value << "\")" << std::endl;
  }
};

class VariableNode : public Node {
 public:
  string name;
  VariableNode(const string &n) : name(n) {}
  void print(int indent = 0) const override {
    std::cout << std::string(indent, ' ') << "Variable(" << name << ")" << std::endl;
  }
};

class BinaryNode : public Node {
 public:
  string op;
  std::unique_ptr<Node> left;
  std::unique_ptr<Node> right;

  BinaryNode(const string &o, Node *l, Node *r) 
    : op(o), left(l), right(r) {}

  void print(int indent = 0) const override {
    std::cout << std::string(indent, ' ') << "BinaryOp(" << op << ")" << std::endl;
    left->print(indent + 2);
    right->print(indent + 2);
  }
};

class UnaryNode : public Node {
 public:
  string op;
  std::unique_ptr<Node> right;

  UnaryNode(const string &o, Node *r) : op(o), right(r) {}

  void print(int indent = 0) const override {
    std::cout << std::string(indent, ' ') << "UnaryOp(" << op << ")" << std::endl;
    right->print(indent + 2);
  }
};

class AssignNode : public Node {
 public:
  string name;
  std::unique_ptr<Node> expression;
  
  AssignNode(const string &n, Node *expr) : name(n), expression(expr) {}
  void print(int indent = 0) const override {
    std::cout << std::string(indent, ' ') << "Assignment(" << name << ")" << std::endl;
    expression->print(indent + 2);
  }
};

class VarDeclNode : public Node {
 public:
  string name;
  bool mutable_;
  Type declaredType;
  std::unique_ptr<Node> expression;
  
  VarDeclNode(const string &n, bool mut, Type type, Node *expr) 
    : name(n), mutable_(mut), declaredType(type), expression(expr) {}
  
  void print(int indent = 0) const override {
    std::cout << std::string(indent, ' ') 
              << "VarDecl(" << name 
              << ", " << (mutable_ ? "mutable" : "immutable") 
              << ")" << std::endl;
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
  
  void print(int indent = 0) const override {
    std::cout << std::string(indent, ' ') << "IfStatement" << std::endl;
    std::cout << std::string(indent + 2, ' ') << "Condition:" << std::endl;
    condition->print(indent + 4);
    std::cout << std::string(indent + 2, ' ') << "Then:" << std::endl;
    thenBlock->print(indent + 4);
    if (elseBlock) {
      std::cout << std::string(indent + 2, ' ') << "Else:" << std::endl;
      elseBlock->print(indent + 4);
    }
  }
};

class BlockNode : public Node {
 public:
  std::vector<std::unique_ptr<Node>> statements;
  bool returnsValue;
  
  BlockNode() : returnsValue(false) {}
  
  void addStatement(Node *statement) { 
    statements.emplace_back(statement); 
  }
  
  void setReturnsValue(bool value) {
    returnsValue = value;
  }
  
  void print(int indent = 0) const override {
    std::cout << std::string(indent, ' ') << "Block" << std::endl;
    for (const auto& stmt : statements) {
      stmt->print(indent + 2);
    }
  }
};

class WhileNode : public Node {
 public:
  std::unique_ptr<Node> condition;
  std::unique_ptr<Node> block;
  
  WhileNode(Node *cond, Node *blk) 
    : condition(cond), block(blk) {}
  
  void print(int indent = 0) const override {
    std::cout << std::string(indent, ' ') << "WhileLoop" << std::endl;
    std::cout << std::string(indent + 2, ' ') << "Condition:" << std::endl;
    condition->print(indent + 4);
    std::cout << std::string(indent + 2, ' ') << "Body:" << std::endl;
    block->print(indent + 4);
  }
};

class FunctionParamNode {
 public:
  string name;
  Type type;
  
  FunctionParamNode(const string &n, Type t) : name(n), type(t) {}
};

class ReturnNode : public Node {
 public:
  std::unique_ptr<Node> expression;
  
  ReturnNode(Node *expr) : expression(expr) {}
  
  void print(int indent = 0) const override {
    std::cout << std::string(indent, ' ') << "Return" << std::endl;
    if (expression) {
      expression->print(indent + 2);
    }
  }
};

class FunctionCallNode : public Node {
 public:
  string functionName;
  std::vector<std::unique_ptr<Node>> arguments;
  
  FunctionCallNode(const string &name) : functionName(name) {}
  
  void addArgument(Node *arg) {
    arguments.emplace_back(arg);
  }
  
  void print(int indent = 0) const override {
    std::cout << std::string(indent, ' ') << "FunctionCall(" << functionName << ")" << std::endl;
    for (const auto& arg : arguments) {
      arg->print(indent + 2);
    }
  }
};

class PrintNode : public Node {
 public:
  std::unique_ptr<Node> arg;
  
  PrintNode(Node *a) : arg(a) {}
  
  void print(int indent = 0) const override {
    std::cout << std::string(indent, ' ') << "Print()" << std::endl;
    arg->print(indent + 2);
  }
};


#endif  // ast_HPP