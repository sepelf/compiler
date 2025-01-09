/**
 * simple calculator
 */

#include <string>
#include <cassert>

enum {
  tok_identifier, tok_number, tok_eof,
};
static int Value;
int getToken() {
  static char LastChar = getchar();

  while (isspace(LastChar))
    LastChar = getchar();

  if (isdigit(LastChar)) {
    std::string value;

    while (isdigit(LastChar)) {
      value += LastChar;
      LastChar = getchar();
    }

    Value = std::stod(value.c_str());

    return tok_number;
  }

  if (LastChar == EOF)
    return tok_eof;

  int ThisChar = LastChar;
  LastChar = getchar();
  return ThisChar;
}

int CurToken;

int getNextToken() {
  return CurToken = getToken();
}

int Error(const char *format, ...) {
  fprintf(stderr, "Error: ");

  va_list va;
  va_start(va, format);
  vfprintf(stderr, format, va);
  va_end(va);

  fprintf(stderr, "\n");

  return -1;
}

static int stack[10240];
static int stack_idx = 0;

void stack_push(const int &v) {
  assert(stack_idx < sizeof(stack));

  stack[stack_idx++] = v;
}

int stack_pop() {
  assert(stack_idx > 0);

  int v = stack[stack_idx - 1];
  stack_idx--;
  return v;
}

void stack_add() {
  assert(stack_idx >= 2);

  stack[stack_idx - 2] = stack[stack_idx - 2] + stack[stack_idx - 1];
  stack_idx--;
}

void stack_sub() {
  assert(stack_idx >= 2);

  stack[stack_idx - 2] = stack[stack_idx - 2] - stack[stack_idx - 1];
  stack_idx--;
}

void stack_mul() {
  assert(stack_idx >= 2);

  stack[stack_idx - 2] = stack[stack_idx - 2] * stack[stack_idx - 1];
  stack_idx--;
}

void stack_div() {
  assert(stack_idx >= 2);

  stack[stack_idx - 2] = stack[stack_idx - 2] / stack[stack_idx - 1];
  stack_idx--;
}

int expression();

int expr_number() {
  fprintf(stderr, " PUSH %d\n", Value);
  stack_push(Value);

  getNextToken();
  return 0;
}

int expr_neg_number() {
  getNextToken(); // eat -

  fprintf(stderr, " PUSH %d\n", -Value);
  stack_push(-Value);

  getNextToken();
  return 0;
}

int expr_paren() {
  getNextToken();
  int r = expression();
  if (r)
    return r;
  if (CurToken != ')')
    return Error("expected ')'");
  getNextToken();
  return 0;
}

int expr_primay() {
  switch (CurToken) {
    case tok_number:
      return expr_number();
    case '-':
      return expr_neg_number();
    case '(':
      return expr_paren();
    default:
      return Error("unsupport token, %c", CurToken);
  }
}

int expr_mul_or_div() {
  int r = expr_primay();
  if (r)
    return r;
  while (CurToken == '*' || CurToken == '/') {
    int Op = CurToken;
    getNextToken(); // eat * or /

    r = expr_primay();
    if (r)
      return r;
    fprintf(stderr, " %s\n", Op == '*' ? "MUL" : "DIV");

    Op == '*' ? stack_mul() : stack_div();
  }
  return 0;
}

int expr_add_or_sub() {
  int r = expr_mul_or_div();
  if (r)
    return r;
  while (CurToken == '+' || CurToken == '-') {
    int Op = CurToken;
    getNextToken(); // eat + or -

    r = expr_mul_or_div();
    if (r)
      return r;
    fprintf(stderr, " %s\n", Op == '+' ? "ADD" : "SUB");

    Op == '+' ? stack_add() : stack_sub();
  }
  return 0;
}

int expression() {
  int r = expr_add_or_sub();
  if (r)
    return r;

  return 0;
}

int mainLoop() {
  fprintf(stderr, "> ");
  getNextToken();

  while (true) {

    switch (CurToken) {
      case tok_eof:
        return 0;
      case ';':
        fprintf(stderr, "%d\n", stack_pop());
        fprintf(stderr, "> ");

        getNextToken();
        break;
      default:
        int r = expression();
        if (r) {
          return r;
        }
        break;
    }
  }

  return 0;
}

int main() {
  return mainLoop();
}
