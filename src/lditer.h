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

// for full modules info
typedef void (*ld_custom_fcb)(const char *, void *base, size_t size, void *data);

struct ld_fcbdata
{
  ld_custom_fcb cb;
  void *cb_data;
};

void ld_fiter(struct ld_fcbdata *);