#include "monitor/watchpoint.h"
#include "debug.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

WP* new_wp(char *expr, uint32_t val) {
  if(free_ == NULL) {
    panic("No more free watchpoint");
  }
  WP *ret = free_;
  free_ = free_->next;
  ret->next = head;
  head = ret;
  memset(ret->expr, 0, sizeof(ret->expr));
  memcpy(ret->expr, expr, 128);
  ret->val = val;
  return ret;
}

void free_wp(int num) {
  if(head == NULL) return;
  if(head->NO == num) {
    WP* tmp = head->next;
    head->next = free_;
    free_=head;
    head = tmp;
    return;
  }
  for(WP *it = head; it != NULL; it = it->next) {
    if(it->next->NO == num) {
      WP *x = it->next;
      it->next = x->next;
      x->next = free_;
      free_ = x;
      return;
    }
  }
  return;
}

void print_wp() {
  for(WP *it = head; it != NULL; it = it->next) {
    printf("%-8d%-8s\n", it->NO, it->expr);
  }
}

bool WP_chk() {
  bool ul = true, flag = false;
  for(WP *it = head; it != NULL; it = it->next) {
    if(expr(it->expr, &ul) != it->val) {
      flag = true;
      printf("Watch point %d activated\n", it->NO);
    }
  }
  return flag;
}