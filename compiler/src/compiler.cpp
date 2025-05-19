#include "compiler.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <ranges>
#include <regex>
#include <sstream>
#include <unordered_map>
#include <variant>
#include <vector>

#include "error.hpp"

Ctx ctx;

void reset() { ctx = Ctx(); }

std::string getLabel(const std::string &prefix) {
  ++ctx.id;
  return prefix + std::to_string(ctx.id);
}

int getTypeSize(Type t) {
  if (t == Type::I32) return 1;
  if (t == Type::STR) return 1;  // string is an addres
  throw std::runtime_error("unreachable");
}

void pushHelper(std::string &res, const std::vector<std::string> &asm_lines) {
  for (const auto &s : asm_lines) {
    if (s.back() != ':') res += "  ";
    res += s;
    res += "\n";
  }
}

void pushStrings(const std::vector<std::string> &asm_lines) {
  pushHelper(ctx.strings, asm_lines);
}

void pushCommands(const std::vector<std::string> &asm_lines) {
  pushHelper(ctx.res, asm_lines);
}

void enterScope() {
  auto cur_offset = ctx.vars.back().first;
  ctx.vars.push_back({});
  ctx.vars.back().first = cur_offset;
}

void exitScope() {
  if (ctx.vars.size() > 1) {
    ctx.vars.pop_back();
  }
}

std::string enterBreakable() {
  const auto label = getLabel("break_");
  ctx.breakable.push_back(label);
  return label;
}
void exitBreakable() {
  if (ctx.breakable.size() > 0) {
    ctx.breakable.pop_back();
  } else {
    nameError("Unreachable exitBreakable");
  }
}

std::string enterContinuable() {
  const auto label = getLabel("contnue_");
  ctx.continuable.push_back(label);
  return label;
}
void exitContinuable() {
  if (ctx.vars.size() > 0) {
    ctx.continuable.pop_back();
  } else {
    nameError("Unreachable exitContinuable");
  }
}

int useReg() {
  ++ctx.usedReg;
  return ctx.usedReg;
}

void dropReg() { --ctx.usedReg; }

bool hasVar(const string &name) {
  for (auto &[_, scope] : std::views::reverse(ctx.vars)) {
    if (scope.count(name)) return true;
  }
  return false;
}

VariableInfo getVar(const string &name) {
  for (auto &[_, scope] : std::views::reverse(ctx.vars)) {
    if (scope.count(name)) return scope.at(name);
  }

  nameError("Cannot find variable '" + name + "' in this scope",
            current_location);
  throw std::runtime_error("unreachable");
}

// Create a new variable with type information
const VariableInfo createVar(const string &name, Type type) {
  if (ctx.vars.back().second.count(name)) {
    nameError("Variable '" + name + "' already exists in this scope");
  }
  ctx.vars.back().first -= getTypeSize(type);
  auto info = VariableInfo(type, ctx.vars.back().first);
  ctx.vars.back().second.emplace(name, info);
  return info;
}

// Evaluate a variable node
void VariableNode::gen() const {
  auto info = getVar(name);
  auto reg = useReg();
  pushCommands({
      "lw x" + std::to_string(reg) + ", x0, " + std::to_string(info.offset),
  });
}

void NumberNode::gen() const {
  auto reg = useReg();
  pushCommands({
      "li x" + std::to_string(reg) + ", " + std::to_string(this->value),
  });
}

std::string unescape(const std::string &input) {
  // Regex pattern to match common escape sequences
  const std::regex escape_pattern(R"(\\([nrtbf'"\\]))");

  std::string result;
  std::sregex_iterator it(input.begin(), input.end(), escape_pattern);
  std::sregex_iterator end;
  size_t last_pos = 0;

  for (; it != end; ++it) {
    std::smatch match = *it;
    size_t match_pos = match.position();
    size_t match_len = match.length();

    // Append text between the last match and the current match
    result += input.substr(last_pos, match_pos - last_pos);

    // Determine the unescaped character
    char escaped_char = match[1].str()[0];
    char replacement;
    switch (escaped_char) {
      case 'n':
        replacement = '\n';
        break;
      case 't':
        replacement = '\t';
        break;
      case 'r':
        replacement = '\r';
        break;
      case 'b':
        replacement = '\b';
        break;
      case 'f':
        replacement = '\f';
        break;
      case '\'':
        replacement = '\'';
        break;
      case '"':
        replacement = '\"';
        break;
      case '\\':
        replacement = '\\';
        break;
      default:
        replacement = '\\';  // Fallback (should not happen)
    }
    result += replacement;

    // Update the last position to the end of the current match
    last_pos = match_pos + match_len;
  }

  // Append remaining text after the last match
  result += input.substr(last_pos);

  return result;
}

void StringNode::gen() const {
  const auto reg = useReg();
  const auto label = getLabel("str_");
  const auto raw = unescape(this->value);
  const auto len = raw.size();

  pushStrings({"# `" + this->value + "`", label + ":",
               "data " + std::to_string(len) + " * 1"});

  for (const auto ch : raw) {
    pushStrings({"data " + std::to_string(static_cast<int>(ch)) + " * 1"});
  }

  pushCommands({
      "li x" + std::to_string(reg) + ", " + label,
  });
}

Type VariableNode::typeCheck() const { return getVar(name).type; }

// op -> asm_op, swap
std::unordered_map<std::string, std::pair<std::string, bool>> bin_int_ops = {
    {"+", {"add", false}},   {"-", {"sub", false}},  {"^", {"xor", false}},
    {">>>", {"slr", false}}, {">>", {"sra", false}}, {"||", {"or", false}},
    {"&&", {"and", false}},  {"*", {"mul", false}},  {"/", {"div", false}},
    {"%", {"rem", false}},   {"<<", {"sll", false}}, {"<", {"slt", false}},
    {"==", {"seq", false}},  {"!=", {"sne", false}}, {">=", {"sge", false}},
    {"<=", {"sge", true}},   {">", {"slt", true}},
};

void BinaryNode::gen() const {
  Type leftType = left->typeCheck();
  Type rightType = right->typeCheck();

  left->gen();
  right->gen();

  dropReg();
  std::string left = "x" + std::to_string(ctx.usedReg);
  std::string right = "x" + std::to_string(ctx.usedReg + 1);

  if (leftType == Type::I32 && rightType == Type::I32 &&
      bin_int_ops.count(op)) {
    const auto [asm_command, swap] = bin_int_ops.at(op);
    if (swap) std::swap(left, right);
    pushCommands({asm_command + " x" + std::to_string(ctx.usedReg) + ", " +
                  left + ", " + right});
  } else if (leftType == Type::STR && rightType == Type::I32 && op == "[]") {
    pushCommands({"add " + left + ", " + left + ", " + right,
                  "lw " + left + ", " + left + ", 1"});
  } else {
    typeError("Invalid types: " + typeToString(static_cast<int>(leftType)) +
              " and " + typeToString(static_cast<int>(rightType)));
  }
}

Type BinaryNode::typeCheck() const {
  Type leftType = left->typeCheck();
  Type rightType = right->typeCheck();

  if (leftType == Type::I32 && rightType == Type::I32 &&
      bin_int_ops.count(op)) {
    return Type::I32;
  } else if (leftType == Type::STR && rightType == Type::I32 && op == "[]") {
    return Type::I32;
  } else {
    typeError("Invalid types: " + typeToString(static_cast<int>(leftType)) +
              " and " + typeToString(static_cast<int>(rightType)) +
              " for operator `" + op + "`");
  }

  return Type::UNKNOWN;
}

void UnaryNode::gen() const {
  right->gen();

  Type rightType = right->typeCheck();
  std::string right = "x" + std::to_string(ctx.usedReg);

  if (op == "-") {
    pushCommands({"sub " + right + ", x0, " + right});
  } else {
    pushCommands({"seq " + right + ", x0, " + right});
  }
}

Type UnaryNode::typeCheck() const {
  Type rightType = right->typeCheck();

  if (rightType != Type::I32) {
    typeError("Unary '" + this->op + "' requires i32 operand");
  }
  return rightType;
}

const std::unordered_map<std::string,
                         std::unordered_map<Type, std::pair<std::string, Type>>>
    macros = {{"print!",
               {{Type::I32, {"print_i32", Type::UNKNOWN}},
                {Type::STR, {"print_str", Type::UNKNOWN}}}},
              {"len!", {{Type::STR, {"len_str", Type::I32}}}},
              {"print_char!", {{Type::I32, {"print_char", Type::UNKNOWN}}}}};

void MacroNode::gen() const {
  this->typeCheck();
  this->arg->gen();
  dropReg();

  const auto type = arg->typeCheck();

  const auto macro = macros.at(this->name).at(type);
  pushCommands({"jal x31, " + macro.first});
  if (macro.second != Type::UNKNOWN) {
    useReg();
  }
}

Type MacroNode::typeCheck() const {
  if (macros.count(this->name) == 0) {
    nameError("Unknown macro '" + name + "'");
  }
  const auto type = arg->typeCheck();
  if (macros.at(this->name).count(type) == 0) {
    nameError("Macro '" + name + "' doesn't support type `" +
              typeToString(static_cast<int>(type)));
  }
  const auto macro = macros.at(this->name).at(type);
  return macro.second;
}

void AssignNode::gen() const {
  this->typeCheck();
  if (!hasVar(name)) {
    nameError("Undefined variable '" + name + "'");
  }

  VariableInfo info = getVar(name);

  expression->gen();
  pushCommands({// sw 0x, <var_offset>, <reg>
                "sw x0, " + std::to_string(info.offset) + ", x" +
                std::to_string(ctx.usedReg)});
  dropReg();
}

Type AssignNode::typeCheck() const {
  Type exprType = expression->typeCheck();
  Type varType = getVar(name).type;

  if (varType != exprType && varType != Type::UNKNOWN) {
    typeError("Cannot assign " + typeToString(static_cast<int>(exprType)) +
              " to variable '" + name + "' of type " +
              typeToString(static_cast<int>(varType)));
  }

  return exprType;
}

void VarDeclNode::gen() const {
  this->typeCheck();
  expression->gen();
  const auto info =
      createVar(name, declaredType == Type::UNKNOWN ? expression->typeCheck()
                                                    : declaredType);
  pushCommands({// sw 0x, <var_offset>, <reg>
                "sw x0, " + std::to_string(info.offset) + ", x" +
                std::to_string(ctx.usedReg)});
  dropReg();
}

Type VarDeclNode::typeCheck() const {
  Type exprType = expression->typeCheck();

  if (declaredType != Type::UNKNOWN && declaredType != exprType) {
    typeError("Cannot initialize " +
              typeToString(static_cast<int>(declaredType)) + " variable '" +
              name + "' with " + typeToString(static_cast<int>(exprType)) +
              " value");
  }

  return declaredType == Type::UNKNOWN ? exprType : declaredType;
}

void IfNode::gen() const {
  this->typeCheck();
  condition->gen();
  std::string else_label = getLabel("else_");
  std::string if_end = getLabel("if_end_");
  dropReg();
  pushCommands({
      "bne x1, x0, 1",
      "jal x0, " + else_label,
  });
  thenBlock->gen();
  pushCommands({
      "jal x0, " + if_end,
      else_label + ":",
  });
  if (elseBlock) {
    elseBlock->gen();
  }
  pushCommands({
      if_end + ":",
  });
}

Type IfNode::typeCheck() const {
  Type condType = condition->typeCheck();
  if (condType != Type::I32) {
    typeError("If condition must be a i32");
  }

  return Type::UNKNOWN;
}

// Evaluate a block of statements
void BlockNode::gen() const {
  enterScope();

  for (size_t i = 0; i < statements.size(); i++) {
    statements[i]->gen();
  }

  exitScope();
}

Type BlockNode::typeCheck() const { return Type::UNKNOWN; }

// Evaluate a while loop
void LoopNode::gen() const {
  enterScope();
  if (init) {
    init->gen();
  }
  this->typeCheck();
  const auto break_label = enterBreakable();
  const auto continue_label = enterContinuable();

  pushCommands({
      continue_label + ":",
  });

  condition->gen();
  dropReg();
  pushCommands({
      "bne x1, x0, 1",
      "jal x0, " + break_label,
  });
  this->block->gen();
  if (this->after_loop) {
    this->after_loop->gen();
  }
  pushCommands({
      "jal x0, " + continue_label,
      break_label + ":",
  });

  exitBreakable();
  exitContinuable();
  exitScope();
}

Type LoopNode::typeCheck() const {
  if (this->condition) {
    Type condType = condition->typeCheck();
    if (condType != Type::I32) {
      typeError("If condition must be a i32");
    }
  }

  return Type::UNKNOWN;
}

void BreakNode::gen() const {
  if (ctx.breakable.size()) {
    pushCommands({
        "jal x0, " + ctx.breakable.back(),
    });
  } else {
    nameError("Not in context to break");
  }
}
Type BreakNode::typeCheck() const { return Type::UNKNOWN; }

void ContinueNode::gen() const {
  if (ctx.continuable.size()) {
    pushCommands({
        "jal x0, " + ctx.continuable.back(),
    });
  } else {
    nameError("Not in context to break");
  }
}

Type ContinueNode::typeCheck() const { return Type::UNKNOWN; }

std::string compile(BlockNode *block) {
  reset();
  block->gen();
  pushCommands({"ebreak"});
  return ctx.prefix + ctx.strings + ctx.res;
}