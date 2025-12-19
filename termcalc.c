// termcalc - fast terminal calculator
// C23 with modern usage patterns
// Usage: c <expr> or just 'c' for interactive mode

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <stdint.h>
#include <inttypes.h>
#include <readline/readline.h>
#include <readline/history.h>

// C23: bool, true, false are keywords
// C23: nullptr instead of NULL
// C23: constexpr for compile-time constants

// ============================================================================
// Constants
// ============================================================================

constexpr double PI = 3.14159265358979323846;
constexpr double E  = 2.71828182845904523536;

// Byte units (binary)
constexpr double KiB = 1024.0;
constexpr double MiB = 1024.0 * 1024.0;
constexpr double GiB = 1024.0 * 1024.0 * 1024.0;
constexpr double TiB = 1024.0 * 1024.0 * 1024.0 * 1024.0;

// Byte units (decimal)
constexpr double KB = 1000.0;
constexpr double MB = 1000000.0;
constexpr double GB = 1000000000.0;
constexpr double TB = 1000000000000.0;

// ============================================================================
// Configuration
// ============================================================================

constexpr int MAX_VARS = 64;
constexpr int MAX_NAME = 32;
constexpr int MAX_INPUT = 1024;

// Output format for current expression
typedef enum { FMT_DEC, FMT_HEX, FMT_BIN, FMT_OCT } OutputFormat;
static OutputFormat g_output_fmt = FMT_DEC;

// ============================================================================
// Variable storage
// ============================================================================

typedef struct {
    char name[MAX_NAME];
    double value;
} Variable;

static Variable vars[MAX_VARS];
static int var_count = 0;

static double get_var(const char *name) {
    for (int i = 0; i < var_count; ++i) {
        if (strcmp(vars[i].name, name) == 0) {
            return vars[i].value;
        }
    }
    // Built-in constants
    if (strcmp(name, "pi") == 0 || strcmp(name, "PI") == 0) return PI;
    if (strcmp(name, "e") == 0 || strcmp(name, "E") == 0) return E;
    if (strcmp(name, "ans") == 0) return var_count > 0 ? vars[0].value : 0.0;

    // Byte units
    if (strcmp(name, "KiB") == 0 || strcmp(name, "kib") == 0) return KiB;
    if (strcmp(name, "MiB") == 0 || strcmp(name, "mib") == 0) return MiB;
    if (strcmp(name, "GiB") == 0 || strcmp(name, "gib") == 0) return GiB;
    if (strcmp(name, "TiB") == 0 || strcmp(name, "tib") == 0) return TiB;
    if (strcmp(name, "KB") == 0 || strcmp(name, "kb") == 0) return KB;
    if (strcmp(name, "MB") == 0 || strcmp(name, "mb") == 0) return MB;
    if (strcmp(name, "GB") == 0 || strcmp(name, "gb") == 0) return GB;
    if (strcmp(name, "TB") == 0 || strcmp(name, "tb") == 0) return TB;

    fprintf(stderr, "undefined: %s\n", name);
    return NAN;
}

static void set_var(const char *name, double value) {
    for (int i = 0; i < var_count; ++i) {
        if (strcmp(vars[i].name, name) == 0) {
            vars[i].value = value;
            return;
        }
    }
    if (var_count < MAX_VARS) {
        strncpy(vars[var_count].name, name, MAX_NAME - 1);
        vars[var_count].value = value;
        ++var_count;
    }
}

// ============================================================================
// Tokenizer
// ============================================================================

typedef enum {
    TOK_NUM, TOK_ID, TOK_OP, TOK_LPAREN, TOK_RPAREN, TOK_END, TOK_ERR
} TokenType;

typedef struct {
    TokenType type;
    double num;
    char id[MAX_NAME];
    char op;
    char op2;  // for two-char operators like << >>
} Token;

typedef struct {
    const char *src;
    const char *pos;
    Token cur;
} Parser;

static void skip_ws(Parser *p) {
    while (isspace((unsigned char)*p->pos)) ++p->pos;
}

// Parse number with hex (0x), binary (0b), octal (0o) support
static bool parse_number(Parser *p) {
    const char *start = p->pos;

    // Check for 0x, 0b, 0o prefixes
    if (p->pos[0] == '0' && p->pos[1] != '\0') {
        char prefix = p->pos[1];
        if (prefix == 'x' || prefix == 'X') {
            // Hexadecimal
            p->pos += 2;
            char *end;
            uint64_t val = strtoull(p->pos, &end, 16);
            if (end == p->pos) { p->pos = start; return false; }
            p->pos = end;
            p->cur.num = (double)val;
            p->cur.type = TOK_NUM;
            return true;
        }
        if (prefix == 'b' || prefix == 'B') {
            // Binary
            p->pos += 2;
            uint64_t val = 0;
            while (*p->pos == '0' || *p->pos == '1') {
                val = (val << 1) | (*p->pos++ - '0');
            }
            p->cur.num = (double)val;
            p->cur.type = TOK_NUM;
            return true;
        }
        if (prefix == 'o' || prefix == 'O') {
            // Octal
            p->pos += 2;
            char *end;
            uint64_t val = strtoull(p->pos, &end, 8);
            if (end == p->pos) { p->pos = start; return false; }
            p->pos = end;
            p->cur.num = (double)val;
            p->cur.type = TOK_NUM;
            return true;
        }
    }

    // Regular decimal/scientific number
    if (isdigit((unsigned char)*p->pos) ||
        (*p->pos == '.' && isdigit((unsigned char)p->pos[1]))) {
        char *end;
        p->cur.num = strtod(p->pos, &end);
        p->pos = end;
        p->cur.type = TOK_NUM;
        return true;
    }

    return false;
}

static void next_token(Parser *p) {
    skip_ws(p);

    if (*p->pos == '\0') {
        p->cur = (Token){.type = TOK_END};
        return;
    }

    // Try parsing a number first
    if (parse_number(p)) return;

    // Identifier (variable or function)
    if (isalpha((unsigned char)*p->pos) || *p->pos == '_') {
        int i = 0;
        while ((isalnum((unsigned char)*p->pos) || *p->pos == '_') && i < MAX_NAME - 1) {
            p->cur.id[i++] = *p->pos++;
        }
        p->cur.id[i] = '\0';
        p->cur.type = TOK_ID;
        return;
    }

    // Operators
    char c = *p->pos++;
    switch (c) {
        case '(': p->cur = (Token){.type = TOK_LPAREN}; return;
        case ')': p->cur = (Token){.type = TOK_RPAREN}; return;
        case '+': case '-': case '/': case '%': case '=':
        case '&': case '|': case '~': case ',':
            p->cur = (Token){.type = TOK_OP, .op = c};
            return;
        case '<':
            if (*p->pos == '<') {
                ++p->pos;
                p->cur = (Token){.type = TOK_OP, .op = 'L'};  // L for left shift
            } else {
                p->cur = (Token){.type = TOK_ERR};
            }
            return;
        case '>':
            if (*p->pos == '>') {
                ++p->pos;
                p->cur = (Token){.type = TOK_OP, .op = 'R'};  // R for right shift
            } else {
                p->cur = (Token){.type = TOK_ERR};
            }
            return;
        case '*':
            if (*p->pos == '*') {
                ++p->pos;
                p->cur = (Token){.type = TOK_OP, .op = '^'};
            } else {
                p->cur = (Token){.type = TOK_OP, .op = '*'};
            }
            return;
        case '^':
            p->cur = (Token){.type = TOK_OP, .op = '^'};
            return;
        default:
            p->cur = (Token){.type = TOK_ERR};
    }
}

// ============================================================================
// Recursive descent parser
// ============================================================================

static double parse_expr(Parser *p);

// Two-argument functions
static double call_func2(const char *name, double arg1, double arg2) {
    if (strcmp(name, "bxor") == 0) return (double)((uint64_t)arg1 ^ (uint64_t)arg2);
    if (strcmp(name, "band") == 0) return (double)((uint64_t)arg1 & (uint64_t)arg2);
    if (strcmp(name, "bor") == 0) return (double)((uint64_t)arg1 | (uint64_t)arg2);
    if (strcmp(name, "shl") == 0) return (double)((uint64_t)arg1 << (int)arg2);
    if (strcmp(name, "shr") == 0) return (double)((uint64_t)arg1 >> (int)arg2);
    if (strcmp(name, "pow") == 0) return pow(arg1, arg2);
    if (strcmp(name, "mod") == 0) return fmod(arg1, arg2);
    if (strcmp(name, "atan2") == 0) return atan2(arg1, arg2);
    if (strcmp(name, "max") == 0) return fmax(arg1, arg2);
    if (strcmp(name, "min") == 0) return fmin(arg1, arg2);
    return NAN;
}

// Built-in functions
static double call_func(const char *name, double arg) {
    // Math functions
    if (strcmp(name, "sin") == 0) return sin(arg);
    if (strcmp(name, "cos") == 0) return cos(arg);
    if (strcmp(name, "tan") == 0) return tan(arg);
    if (strcmp(name, "asin") == 0) return asin(arg);
    if (strcmp(name, "acos") == 0) return acos(arg);
    if (strcmp(name, "atan") == 0) return atan(arg);
    if (strcmp(name, "sinh") == 0) return sinh(arg);
    if (strcmp(name, "cosh") == 0) return cosh(arg);
    if (strcmp(name, "tanh") == 0) return tanh(arg);
    if (strcmp(name, "exp") == 0) return exp(arg);
    if (strcmp(name, "log") == 0) return log(arg);
    if (strcmp(name, "log10") == 0) return log10(arg);
    if (strcmp(name, "log2") == 0) return log2(arg);
    if (strcmp(name, "sqrt") == 0) return sqrt(arg);
    if (strcmp(name, "cbrt") == 0) return cbrt(arg);
    if (strcmp(name, "abs") == 0) return fabs(arg);
    if (strcmp(name, "floor") == 0) return floor(arg);
    if (strcmp(name, "ceil") == 0) return ceil(arg);
    if (strcmp(name, "round") == 0) return round(arg);
    if (strcmp(name, "ln") == 0) return log(arg);

    // Bitwise functions
    if (strcmp(name, "bnot") == 0) return (double)(~(uint64_t)arg);
    if (strcmp(name, "not8") == 0) return (double)((uint8_t)~(uint8_t)arg);
    if (strcmp(name, "not16") == 0) return (double)((uint16_t)~(uint16_t)arg);
    if (strcmp(name, "not32") == 0) return (double)((uint32_t)~(uint32_t)arg);

    // Programmer functions - format converters (set output format)
    if (strcmp(name, "hex") == 0) { g_output_fmt = FMT_HEX; return arg; }
    if (strcmp(name, "bin") == 0) { g_output_fmt = FMT_BIN; return arg; }
    if (strcmp(name, "oct") == 0) { g_output_fmt = FMT_OCT; return arg; }
    if (strcmp(name, "dec") == 0) { g_output_fmt = FMT_DEC; return arg; }

    // Byte conversions - convert TO these units
    if (strcmp(name, "toKiB") == 0 || strcmp(name, "tokib") == 0) return arg / KiB;
    if (strcmp(name, "toMiB") == 0 || strcmp(name, "tomib") == 0) return arg / MiB;
    if (strcmp(name, "toGiB") == 0 || strcmp(name, "togib") == 0) return arg / GiB;
    if (strcmp(name, "toTiB") == 0 || strcmp(name, "totib") == 0) return arg / TiB;
    if (strcmp(name, "toKB") == 0 || strcmp(name, "tokb") == 0) return arg / KB;
    if (strcmp(name, "toMB") == 0 || strcmp(name, "tomb") == 0) return arg / MB;
    if (strcmp(name, "toGB") == 0 || strcmp(name, "togb") == 0) return arg / GB;
    if (strcmp(name, "toTB") == 0 || strcmp(name, "totb") == 0) return arg / TB;

    // Bit manipulation
    if (strcmp(name, "popcount") == 0) return (double)__builtin_popcountll((uint64_t)arg);
    if (strcmp(name, "clz") == 0) return arg == 0 ? 64 : (double)__builtin_clzll((uint64_t)arg);
    if (strcmp(name, "ctz") == 0) return arg == 0 ? 64 : (double)__builtin_ctzll((uint64_t)arg);

    fprintf(stderr, "unknown function: %s\n", name);
    return NAN;
}

// primary: number | identifier | function(expr) | (expr) | -primary | ~primary
static double parse_primary(Parser *p) {
    // Unary minus/plus
    if (p->cur.type == TOK_OP && (p->cur.op == '-' || p->cur.op == '+')) {
        char op = p->cur.op;
        next_token(p);
        double val = parse_primary(p);
        return op == '-' ? -val : val;
    }

    // Bitwise NOT
    if (p->cur.type == TOK_OP && p->cur.op == '~') {
        next_token(p);
        double val = parse_primary(p);
        return (double)(~(uint64_t)val);
    }

    // Parentheses
    if (p->cur.type == TOK_LPAREN) {
        next_token(p);
        double val = parse_expr(p);
        if (p->cur.type == TOK_RPAREN) next_token(p);
        return val;
    }

    // Number
    if (p->cur.type == TOK_NUM) {
        double val = p->cur.num;
        next_token(p);
        return val;
    }

    // Identifier or function call
    if (p->cur.type == TOK_ID) {
        char name[MAX_NAME];
        strcpy(name, p->cur.id);
        next_token(p);

        // Function call
        if (p->cur.type == TOK_LPAREN) {
            next_token(p);
            double arg1 = parse_expr(p);

            // Check for second argument
            if (p->cur.type == TOK_OP && p->cur.op == ',') {
                next_token(p);
                double arg2 = parse_expr(p);
                if (p->cur.type == TOK_RPAREN) next_token(p);
                return call_func2(name, arg1, arg2);
            }

            if (p->cur.type == TOK_RPAREN) next_token(p);
            return call_func(name, arg1);
        }

        // Variable
        return get_var(name);
    }

    fprintf(stderr, "syntax error\n");
    return NAN;
}

// power: primary (^ power)?  (right associative)
static double parse_power(Parser *p) {
    double left = parse_primary(p);
    if (p->cur.type == TOK_OP && p->cur.op == '^') {
        next_token(p);
        double right = parse_power(p);  // right associative
        return pow(left, right);
    }
    return left;
}

// term: power ((*|/|%) power)*
static double parse_term(Parser *p) {
    double left = parse_power(p);
    while (p->cur.type == TOK_OP &&
           (p->cur.op == '*' || p->cur.op == '/' || p->cur.op == '%')) {
        char op = p->cur.op;
        next_token(p);
        double right = parse_power(p);
        switch (op) {
            case '*': left *= right; break;
            case '/': left /= right; break;
            case '%': left = fmod(left, right); break;
        }
    }
    return left;
}

// additive: term ((+|-) term)*
static double parse_additive(Parser *p) {
    double left = parse_term(p);
    while (p->cur.type == TOK_OP && (p->cur.op == '+' || p->cur.op == '-')) {
        char op = p->cur.op;
        next_token(p);
        double right = parse_term(p);
        left = (op == '+') ? left + right : left - right;
    }
    return left;
}

// shift: additive ((<<|>>) additive)*
static double parse_shift(Parser *p) {
    double left = parse_additive(p);
    while (p->cur.type == TOK_OP && (p->cur.op == 'L' || p->cur.op == 'R')) {
        char op = p->cur.op;
        next_token(p);
        double right = parse_additive(p);
        uint64_t l = (uint64_t)left;
        int r = (int)right;
        left = (op == 'L') ? (double)(l << r) : (double)(l >> r);
    }
    return left;
}

// bitand: shift (& shift)*
static double parse_bitand(Parser *p) {
    double left = parse_shift(p);
    while (p->cur.type == TOK_OP && p->cur.op == '&') {
        next_token(p);
        double right = parse_shift(p);
        left = (double)((uint64_t)left & (uint64_t)right);
    }
    return left;
}

// bitor: bitand (| bitand)*
static double parse_bitor(Parser *p) {
    double left = parse_bitand(p);
    while (p->cur.type == TOK_OP && p->cur.op == '|') {
        next_token(p);
        double right = parse_bitand(p);
        left = (double)((uint64_t)left | (uint64_t)right);
    }
    return left;
}

// expr: bitor
static double parse_expr(Parser *p) {
    return parse_bitor(p);
}

// ============================================================================
// Top-level: handle assignment or expression
// ============================================================================

static double evaluate(const char *input) {
    g_output_fmt = FMT_DEC;  // Reset format for each expression

    Parser p = {.src = input, .pos = input};
    next_token(&p);

    // Check for assignment: name = expr
    if (p.cur.type == TOK_ID) {
        char name[MAX_NAME];
        strcpy(name, p.cur.id);

        const char *saved = p.pos;
        Token saved_tok = p.cur;
        next_token(&p);

        if (p.cur.type == TOK_OP && p.cur.op == '=') {
            next_token(&p);
            double val = parse_expr(&p);
            set_var(name, val);
            return val;
        }

        // Not assignment, backtrack
        p.pos = saved;
        p.cur = saved_tok;
    }

    return parse_expr(&p);
}

// ============================================================================
// Output formatting
// ============================================================================

static void print_binary(uint64_t val) {
    if (val == 0) { puts("0b0"); return; }

    char buf[65];
    int i = 64;
    buf[i--] = '\0';

    while (val && i >= 0) {
        buf[i--] = '0' + (val & 1);
        val >>= 1;
    }
    printf("0b%s\n", &buf[i + 1]);
}

static void print_result(double val) {
    if (isnan(val)) return;

    switch (g_output_fmt) {
        case FMT_HEX:
            printf("0x%" PRIX64 "\n", (uint64_t)val);
            break;
        case FMT_BIN:
            print_binary((uint64_t)val);
            break;
        case FMT_OCT:
            printf("0o%" PRIo64 "\n", (uint64_t)val);
            break;
        case FMT_DEC:
        default:
            // Check if it's effectively an integer
            if (fabs(val) < 1e15 && val == floor(val)) {
                printf("%.0f\n", val);
            } else {
                printf("%.12g\n", val);
            }
            break;
    }
}

// ============================================================================
// Interactive mode
// ============================================================================

static const char *get_history_path(void) {
    static char path[512];
    const char *home = getenv("HOME");
    if (home) {
        snprintf(path, sizeof(path), "%s/.c_history", home);
        return path;
    }
    return nullptr;
}

static void repl(void) {
    // Load history
    const char *hist_path = get_history_path();
    if (hist_path) read_history(hist_path);

    // Enable prefix search with up/down arrows
    rl_bind_keyseq("\\e[A", rl_history_search_backward);  // Up
    rl_bind_keyseq("\\e[B", rl_history_search_forward);   // Down

    char *line;
    while ((line = readline("> ")) != nullptr) {
        // Skip empty lines
        if (line[0] == '\0') {
            free(line);
            continue;
        }

        // Exit commands
        if (strcmp(line, "q") == 0 || strcmp(line, "quit") == 0 ||
            strcmp(line, "exit") == 0) {
            free(line);
            break;
        }

        // Help
        if (strcmp(line, "help") == 0 || strcmp(line, "?") == 0) {
            puts("termcalc - fast terminal calculator");
            puts("");
            puts("OPERATORS");
            puts("  arithmetic:  + - * / % ^ **");
            puts("  bitwise:     & | ~ << >>");
            puts("");
            puts("NUMBERS");
            puts("  decimal:     42, 3.14, 1e-9");
            puts("  hex:         0xFF, 0x1A2B");
            puts("  binary:      0b1010, 0b11110000");
            puts("  octal:       0o755, 0o644");
            puts("");
            puts("FUNCTIONS");
            puts("  math:        sin cos tan asin acos atan sinh cosh tanh");
            puts("               exp log log10 log2 ln sqrt cbrt abs floor ceil round");
            puts("               pow(x,y) atan2(y,x) max(a,b) min(a,b) mod(a,b)");
            puts("  bitwise:     popcount clz ctz bnot not8 not16 not32");
            puts("               bxor(a,b) band(a,b) bor(a,b) shl(x,n) shr(x,n)");
            puts("  format:      hex() bin() oct() dec()");
            puts("  bytes:       toKiB toMiB toGiB toTiB toKB toMB toGB toTB");
            puts("");
            puts("CONSTANTS");
            puts("  pi e ans");
            puts("  KiB MiB GiB TiB  (1024-based)");
            puts("  KB MB GB TB     (1000-based)");
            puts("");
            puts("EXAMPLES");
            puts("  0xFF & 0b1111        -> 15");
            puts("  1 << 10              -> 1024");
            puts("  hex(255)             -> 0xFF");
            puts("  bxor(0xF0, 0xFF)     -> 15");
            puts("  not8(0xF0)           -> 15");
            puts("  4*GiB                -> 4294967296");
            puts("  toMiB(4*GiB)         -> 4096");
            puts("");
            puts("exit: q, quit, exit, or Ctrl+D");
            free(line);
            continue;
        }

        // Add to history
        add_history(line);

        double result = evaluate(line);
        print_result(result);
        set_var("ans", result);
        free(line);
    }

    // Save history
    if (hist_path) write_history(hist_path);
}

// ============================================================================
// Main
// ============================================================================

int main(int argc, char *argv[]) {
    if (argc == 1) {
        repl();
        return 0;
    }

    // Concatenate all arguments into one expression
    char expr[MAX_INPUT] = {};
    for (int i = 1; i < argc; ++i) {
        if (i > 1) strcat(expr, " ");
        strcat(expr, argv[i]);
    }

    double result = evaluate(expr);
    print_result(result);

    return isnan(result) ? 1 : 0;
}
