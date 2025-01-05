#include <string>
#include <vector>

/**
 * ============================================================================
 * Lexer
 * ============================================================================
 */
enum Token {
  tok_eof = -1,

  // commands
  tok_def = -2,
  tok_extern = -3,

  // primary
  tok_identifier = -4,
  tok_number = -5,
};

static std::string IdentifierStr;
static double NumVal;

static int gettok() {
  static int LastChar = ' ';

  // skip whitespace
  while (isspace(LastChar))
    LastChar = getchar();

  if (isalpha(LastChar)) { // identifier: [a-zA-Z][a-zA-Z0-9]*
    IdentifierStr = LastChar;
    while (isalnum(LastChar = getchar()))
      IdentifierStr += LastChar;

    if (IdentifierStr == "def")
      return tok_def;
    else if (IdentifierStr == "extern")
      return tok_extern;
    return tok_identifier;
  }

  if (isdigit(LastChar) || LastChar == '.') { // number: [0-9.]+
    std::string NumStr;
    do {
      NumStr += LastChar;
      LastChar = getchar();
    } while (isdigit(LastChar) || LastChar == '.');

    NumVal = strtod(NumStr.c_str(), 0);
    return tok_number;
  }

  if (LastChar == '#') { // comment until end of line
    do {
      LastChar = getchar();
    } while (LastChar != EOF && LastChar != '\n' && LastChar != '\r');

    if (LastChar != EOF)
      return gettok();
  }

  if (LastChar == EOF) // end of file
    return tok_eof;

  // otherwide, just return the character
  int ThisChar = LastChar;
  LastChar = getchar();
  return ThisChar;
}

/**
 * ============================================================================
 * Abstract Syntax Tree (aka Parse Tree)
 * ============================================================================
 */
// base class for all expression nodes
class ExprAST {
public:
  virtual ~ExprAST() {
  }
  virtual std::string ToString() const {
    return "";
  }
};

// a numeric
class NumberExprAST: public ExprAST {
  double Val;
public:
  NumberExprAST(double val) :
      Val(val) {
  }
  std::string ToString() const override {
    return "NumberExpr: " + std::to_string(Val);
  }
};

// a variable
class VariableExprAST: public ExprAST {
  std::string Name;
public:
  VariableExprAST(const std::string &name) :
      Name(name) {
  }
  std::string ToString() const override {
    return "Variable: " + Name;
  }
};

// a binary operator
class BinaryExprAST: public ExprAST {
  char Op;
  ExprAST *LHS, *RHS;
public:
  BinaryExprAST(char op, ExprAST *lhs, ExprAST *rhs) :
      Op(op), LHS(lhs), RHS(rhs) {
  }
  std::string ToString() const override {
    return "BinaryExpr: " + std::string(&Op) + " ( " + LHS->ToString() + ", " + RHS->ToString() + " ) ";
  }
};

// a function call
class CallExprAST: public ExprAST {
  std::string Callee;
  std::vector<ExprAST*> Args;
public:
  CallExprAST(const std::string &callee, const std::vector<ExprAST*> &args) :
      Callee(callee), Args(args) {
  }
  std::string ToString() const override {
    std::string s = "Call: " + Callee;
    for (const ExprAST *arg : Args) {
      s += ", Arg: " + arg->ToString();
    }
    return s;
  }
};

// function prototype
class PrototypeAST {
  std::string Name;
  std::vector<std::string> Args;
public:
  PrototypeAST(const std::string &name, const std::vector<std::string> &args) :
      Name(name), Args(args) {
  }
  std::string ToString() const {
    std::string s = "Prototype: " + Name;
    for (const std::string &arg : Args) {
      s += ", Arg: " + arg;
    }
    return s;
  }
};

// function definition itself
class FunctionAST {
  PrototypeAST *Proto;
  ExprAST *Body;
public:
  FunctionAST(PrototypeAST *proto, ExprAST *body) :
      Proto(proto), Body(body) {
  }
  std::string ToString() const {
    std::string s = "Function: \n";
    s += "	" + Proto->ToString() + "\n";
    s += "	" + Body->ToString() + "\n";
    return s;
  }
};

/**
 * ============================================================================
 * Parser
 * ============================================================================
 */
static int CurTok;
static int getNextToken() {
  return CurTok = gettok();
}

ExprAST* Error(const char *Str) {
  fprintf(stderr, "Error: %s\n", Str);
  return nullptr;
}
PrototypeAST* ErrorP(const char *Str) {
  Error(Str);
  return nullptr;
}
FunctionAST* ErrorF(const char *Str) {
  Error(Str);
  return nullptr;
}

static ExprAST* ParseExpression();

// numberexpr ::= number
static ExprAST* ParseNumberExpr() {
  ExprAST *Result = new NumberExprAST(NumVal);
  getNextToken(); // consume the number
  return Result;
}

// parenexpr ::= '(' expression ')'
static ExprAST* ParseParenExpr() {
  getNextToken(); // eat (.
  ExprAST *V = ParseExpression();
  if (!V)
    return nullptr;

  if (CurTok != ')')
    return Error("expected ')");
  getNextToken(); // eat ).
  return V;
}

// identifierexpr
// 		::= identifier
//		::= identifier '(' expression* ')'
static ExprAST* ParseIdentifierExpr() {
  std::string IdName = IdentifierStr;

  getNextToken(); // eat identifier

  if (CurTok != '(') // simple variable ref
    return new VariableExprAST(IdName);

  // Call
  getNextToken(); // eat (
  std::vector<ExprAST*> Args;
  if (CurTok != ')') {
    while (true) {
      ExprAST *Arg = ParseExpression();
      if (!Arg)
        return nullptr;
      Args.emplace_back(Arg);

      if (CurTok == ')')
        break;

      if (CurTok != ',')
        return Error("Expected ')' or ',' in argument list");

      getNextToken();
    }
  }

  getNextToken(); // eat ')'

  return new CallExprAST(IdName, Args);
}

// primary
//	::= identifierexpr
//	::= numberexpr
// 	::= parenexpr
static ExprAST* ParsePrimary() {
  switch (CurTok) {
    case tok_identifier:
      return ParseIdentifierExpr();
    case tok_number:
      return ParseNumberExpr();
    case '(':
      return ParseParenExpr();
    default:
      return Error("unknow token when expecting an expression");
  }
}

// get precedence of operator
static int GetTokPrecedence() {
  if (!isascii(CurTok))
    return -1;

  int TokPrec;
  switch (CurTok) {
    case '+':
      TokPrec = 20;
      break;
    case '-':
      TokPrec = 20;
      break;
    case '*':
      TokPrec = 40;
      break;
    case '/':
      TokPrec = 40;
      break;
    default:
      TokPrec = -1;
      break;
  }
  return TokPrec;
}

// binoprhs
// 	::= ('+' primary)*
static ExprAST* ParseBinOpRHS(int ExprPrec, ExprAST *LHS) {
  while (true) {
    int TokPrec = GetTokPrecedence();

    if (TokPrec < ExprPrec)
      return LHS;

    // Okey, we know this is a binop
    int BinOp = CurTok;
    getNextToken(); // eat binop

    ExprAST *RHS = ParsePrimary();
    if (!RHS)
      return nullptr;

    int NextPrec = GetTokPrecedence();
    if (TokPrec < NextPrec) {
      RHS = ParseBinOpRHS(TokPrec + 1, RHS);
      if (!RHS)
        return nullptr;
    }

    // merge LHS/RHS
    LHS = new BinaryExprAST(BinOp, LHS, RHS);
  }
}

// expression
// 	::= primary binoprhs
static ExprAST* ParseExpression() {
  ExprAST *LHS = ParsePrimary();
  if (!LHS)
    return nullptr;

  return ParseBinOpRHS(0, LHS);
}

// prototype
// 	::= id '(' id* ')'
static PrototypeAST* ParsePrototype() {
  if (CurTok != tok_identifier)
    return ErrorP("Expected function name in prototype");

  std::string FnName = IdentifierStr;
  getNextToken();

  if (CurTok != '(')
    return ErrorP("Expected '(' in prototype");

  std::vector<std::string> ArgNames;
  while (getNextToken() == tok_identifier)
    ArgNames.emplace_back(IdentifierStr);

  if (CurTok != ')')
    return ErrorP("Expected ')' in prototype");

  // success
  getNextToken(); // eat ')'

  return new PrototypeAST(FnName, ArgNames);
}

// definition
// 	::= 'def' prototype expression
static FunctionAST* ParseDefinition() {
  getNextToken(); // eat def
  PrototypeAST *Proto = ParsePrototype();
  if (!Proto)
    return nullptr;

  ExprAST *Body = ParseExpression();
  if (!Body)
    return nullptr;

  return new FunctionAST(Proto, Body);
}

// external
// 	::= 'extern' prototype
static PrototypeAST* ParseExtern() {
  getNextToken(); // eat extern
  return ParsePrototype();
}

// toplevelexpr
// 	::= expression
static FunctionAST* ParseTopLevelExpr() {
  ExprAST *Body = ParseExpression();
  if (!Body)
    return nullptr;

  PrototypeAST *Proto = new PrototypeAST("", std::vector<std::string>());
  return new FunctionAST(Proto, Body);
}

/**
 * ============================================================================
 * Top-Level parsing
 * ============================================================================
 */
static void HandleDefinition() {
  FunctionAST *AST = ParseDefinition();
  if (AST) {
    fprintf(stderr, "Parsed a function definition.\n");
    fprintf(stderr, "%s\n", AST->ToString().c_str());
  } else {
    getNextToken();
  }
}

static void HandleExtern() {
  PrototypeAST *AST = ParseExtern();
  if (AST) {
    fprintf(stderr, "Parsed an extern.\n");
    fprintf(stderr, "%s\n", AST->ToString().c_str());
  } else {
    getNextToken();
  }
}

static void HandleTopLevelExpression() {
  FunctionAST *AST = ParseTopLevelExpr();
  if (AST) {
    fprintf(stderr, "Parsed a top-level expression.\n");
    fprintf(stderr, "%s\n", AST->ToString().c_str());
  } else {
    getNextToken();
  }
}

// top
// 	::= definition | external | expression | ';'
static void MainLoop() {
  while (true) {
    fprintf(stderr, "ready> ");
    switch (CurTok) {
      case tok_eof:
        return;
      case ';':
        getNextToken();
        break; // ignore top-level semicoluns
      case tok_def:
        HandleDefinition();
        break;
      case tok_extern:
        HandleExtern();
        break;
      default:
        HandleTopLevelExpression();
        break;
    }
  }
}

int main() {
  fprintf(stderr, "ready> ");
  getNextToken();

  MainLoop();

  return 0;
}
