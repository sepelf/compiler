/**
 * simple calculator
 */

#include <string>

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

int expression();

int expr_number() {
  fprintf(stderr, " PUSH %d\n", Value);
  stack[stack_idx++] = Value;

  getNextToken();
  return 0;
}

int expr_neg_number() {
  getNextToken(); // eat -

  fprintf(stderr, " PUSH %d\n", -Value);
  stack[stack_idx++] = -Value;

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

    stack[stack_idx - 2] =
        Op == '*' ? stack[stack_idx - 2] * stack[stack_idx - 1] : stack[stack_idx - 2] / stack[stack_idx - 1];
    stack_idx--;
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

    stack[stack_idx - 2] =
        Op == '+' ? stack[stack_idx - 2] + stack[stack_idx - 1] : stack[stack_idx - 2] - stack[stack_idx - 1];
    stack_idx--;
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
  while (true) {
    switch (CurToken) {
      case tok_eof:
        return 0;
      case ';':
        fprintf(stderr, " Result=%d\n", stack[stack_idx - 1]);

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
  getNextToken();

  return mainLoop();
}
