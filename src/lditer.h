#pragma once

void ld_iter();

struct ld_data
{
  // in param
  const char *name;
  // output
  char *base;
  char *x_start;
  long x_size;
};

typedef void (*ld_custom_cb)(const char *, void *);

struct ld_cbdata
{
  ld_custom_cb cb;
  void *cb_data;
};

int cmp_sonames(const char *, const char *);
int ld_iter(struct ld_data *);
void ld_iter2(struct ld_cbdata *);