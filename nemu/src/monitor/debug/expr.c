#include "cpu/reg.h"
#include "debug.h"
#include "memory/memory.h"
#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <setjmp.h>
#include <stdlib.h>
#include <sys/types.h>
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ, TK_NEQ, TK_AND, TK_OR, TK_NOT, TK_DEC, TK_HEX, TK_REG, TK_NEG, TK_DEREF, 

  /* TODO: Add more token types */

};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"==", TK_EQ},        // equal
  {"!=", TK_NEQ},
  {"&&", TK_AND},
  {"\\|\\|", TK_OR},
  {"\\+", '+'},         // plus
  {"-", '-'},
  {"\\*", '*'},
  {"/", '/'},
  {"\\(", '('},
  {"\\)", ')'},
  {"(0(x|X)[1-9a-eA-E][0-9a-eA-E]*)|(0(x|X)0)", TK_HEX},
  {"(-?[1-9][0-9]*)|0", TK_DEC},
  {"\\$[a-z]+", TK_REG},
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */
        switch (rules[i].token_type) {
          case TK_NOTYPE :
            break;
          case TK_HEX :
          case TK_REG :
          case TK_DEC :
            memset(tokens[nr_token].str, 0, sizeof(char) * 32);
            if(substr_len > 32) {
              printf("number is too long at position %d\n%s\n%*.s^\n", position, e, position, "");
              return false;
            }
            memcpy(tokens[nr_token].str, substr_start, substr_len);
          default :
            tokens[nr_token].type = rules[i].token_type;
            ++nr_token;
            break;
        }
        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

bool is_bin_op(int type) {
  return type == '+' || type == '-' || type == '*' || type == '/' 
        || type == TK_AND || type == TK_EQ || type == TK_NEQ || type == TK_OR;
}

bool is_1_op(int type) {
  return type == TK_NEG || type == TK_DEREF || type == TK_NOT;
}

bool is_num(int type) {
  return type == TK_DEC || type == TK_HEX || type == TK_REG;
}

bool chk_pat(int l, int r) {
  if(tokens[l].type != '(' || tokens[r].type != ')') return false;
  int num = 0;
  for(int i = l; i <= r; ++i) {
    if(tokens[i].type == '(') ++num;
    if(tokens[i].type == ')') --num;
    if(num <= 0 && i != r) return false;
  }
  return num == 0;
}

uint32_t reg_val(int p, bool *success) {
  if(!strcmp(tokens[p].str + 1, "eip")) {
    return cpu.eip;
  }
  for(int i = 0; i < 8; ++i) {
    if(!strcmp(tokens[p].str + 1, regsl[i])) {
      return cpu.gpr[i]._32;
    } else if(!strcmp(tokens[p].str, regsw[i])) {
      return cpu.gpr[i]._16;
    } else if(!strcmp(tokens[i].str, regsb[i])) {
      return cpu.gpr[i & 3]._8[i >> 2];
    }
  }
  *success = false;
  return 0;
}

uint32_t eval(int l, int r, bool *success) {
  if(!(*success)) return 0;
  if(l > r) return 0;
  if(is_bin_op(tokens[l].type) || is_bin_op(tokens[r].type) || is_1_op(tokens[r].type)) {
    *success = false;
    return 0;
  } else if(l == r) {
    if(tokens[l].type == TK_DEC || tokens[l].type == TK_HEX) {
      return strtol(tokens[l].str, NULL, 0);
    } else if(tokens[l].type == TK_REG) {
      return reg_val(l, success);
    } else {
      *success = false;
      return 0;
    }
  } else if(chk_pat(l, r)) {
    return eval(l + 1, r - 1, success);
  } else {
    int num = 0;
    // && and ||
    for(int i = l; i <= r; ++i) {
      if(tokens[i].type == '(') ++num;
      else if(tokens[i].type == ')') --num;
      if(num > 0) continue;
      if(tokens[i].type == TK_AND) {
        return eval(l, i - 1, success) && eval(i + 1, r, success);
      } else if(tokens[i].type == TK_OR) {
        return eval(l, i - 1, success) || eval(i + 1, r, success);
      }
    }
    // == and !=
    for(int i = l; i <= r; ++i) {
      if(tokens[i].type == '(') ++num;
      else if(tokens[i].type == ')') --num;
      if(num > 0) continue;
      if(tokens[i].type == TK_EQ) {
        return eval(l, i - 1, success) == eval(i + 1, r, success);
      } else if(tokens[i].type == TK_NEQ) {
        return eval(l, i - 1, success) != eval(i + 1, r, success);
      }
    }
    // + and -
    for(int i = l; i <= r; ++i) {
      if(tokens[i].type == '(') ++num;
      else if(tokens[i].type == ')') --num;
      if(num > 0) continue;
      if(tokens[i].type == '+') {
        return eval(l, i - 1, success) + eval(i + 1, r, success);
      } else if(tokens[i].type == '-') {
        return eval(l, i - 1, success) - eval(i + 1, r, success);
      }
    }
    // * and /
    for(int i = l; i <= r; ++i) {
      if(tokens[i].type == '(') ++num;
      else if(tokens[i].type == ')') --num;
      if(num > 0) continue;
      if(tokens[i].type == '/') {
        int a = eval(l, i - 1, success);
        int b = eval(i + 1, r, success);
        if(!b) {
          *success = false;
          return 0;
        }
        return a / b;
      } else if(tokens[i].type == '*') {
        return eval(l, i - 1, success) * eval(i + 1, r, success);
      }
    }
    // Deref, Neg and Not
    if(tokens[l].type == TK_DEREF) {
      int x = eval(l + 1, r, success);
      return *success ? vaddr_read(x, 4) : 0;
    } else if(tokens[l].type == TK_NOT) {
      return !eval(l + 1, r, success);
    } else if(tokens[l].type == TK_NEG) {
      return -eval(l + 1, r, success);
    }
  }
  *success = false;
  return 0;
}

uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  for(int i = 0, num = 0; i < nr_token; ++i) {
    if(!i || is_bin_op(tokens[i - 1].type) || is_1_op(tokens[i - 1].type)) {
      if(tokens[i].type == '-') tokens[i].type = TK_NEG;
      if(tokens[i].type == '*') tokens[i].type = TK_DEREF;
    }
    if(tokens[i].type == '(') ++num;
    else if(tokens[i].type == ')') --num;
    if(num < 0) {
      *success = false;
      return 0;
    }
  }
  return eval(0, nr_token - 1, success);
}
