#include <Rcpp.h>
#include <regex>
#include "lditer.h"
#include "eread.h"
using namespace Rcpp;

extern "C" {
void moncontrol(int);
void _mcleanup (void);
void mcount(void);
void monstartup (char *, char *);
};

void my_cb(const char *fname, void *data)
{
  CharacterVector *cv = (CharacterVector *)data;
  cv->push_back(fname);
}

//' get list of loaded modules inside current process
//'
//'@return string vector of modules with full paths
// [[Rcpp::export]]
CharacterVector lsmod()
{
  CharacterVector res;
  ld_cbdata cb { my_cb, &res };
  ld_iter2( &cb );
  return res;
}

struct fcont
{
  CharacterVector *names, *base;
  NumericVector *size;
  std::regex *re;
};

void my_cbf(const char *fname, void *base, size_t size, void *data)
{
  if ( !base ) return;
  fcont *fc = (fcont *)data;
  if ( fc->re && !std::regex_search(fname, *fc->re) ) return;
  if ( fname )
    fc->names->push_back(fname);
  else
    fc->names->push_back("");
  char buf[40];
  snprintf(buf, sizeof(buf) - 1, "%p", base);
  buf[39] = 0;
  fc->base->push_back(buf);
  fc->size->push_back((double)size);
}

//' get frame of loaded modules inside current process
//'
//'@return frame consisted of modules full paths, base address and size
// [[Rcpp::export]]
DataFrame lsmodf()
{
  CharacterVector names, base;
  NumericVector size;
  fcont tmp{ &names, &base, &size, nullptr };
  ld_fcbdata fc { my_cbf, &tmp };
  ld_fiter(&fc);
  return DataFrame::create(Named("path") = names,
    Named("base") = base, Named("size") = size);
}

//' get frame of loaded modules inside current process matching regex
//'
//'@param re regex
//'@return frame consisted of modules full paths, base address and size
// [[Rcpp::export]]
DataFrame lsmodfr(std::string &re)
{
  CharacterVector names, base;
  NumericVector size;
  fcont tmp{ &names, &base, &size, new std::regex(re, std::regex_constants::ECMAScript) };
  ld_fcbdata fc { my_cbf, &tmp };
  ld_fiter(&fc);
  delete tmp.re;
  return DataFrame::create(Named("path") = names,
    Named("base") = base, Named("size") = size);
}

struct rdata
{
  CharacterVector *cv;
  std::regex *re;
};

void my_rcb(const char *fname, void *data)
{
  rdata *rd = (rdata *)data;
  if ( std::regex_search(fname, *rd->re) )
   rd->cv->push_back(fname);
}

//' get list of loaded modules inside current process matching regex
//'
//'@param re regex
//'@return string vector of modules with full paths
// [[Rcpp::export]]
CharacterVector lsmodr(std::string &re)
{
  CharacterVector res;
  rdata rd;
  rd.cv = &res;
  rd.re = new std::regex(re, std::regex_constants::ECMAScript);
  ld_cbdata cb { my_rcb, &rd };
  ld_iter2( &cb );
  delete rd.re;
  return res;
}

//' check if module can be profiles
//'
//'@param fname full path to module
//'@return -1 if no such module or it is not ELF, 0 when module cannot be profiles, 1 if it was conpuled with -pg option, 2 fir -finstrument-functions option
// [[Rcpp::export]]
int checkmod(std::string &fname)
{
  struct prof_data pd;
  int res = process_elf(fname.c_str(), &pd);
  if ( res < 0 ) return -1;
  if ( pd.m_mcount ) return 1;
  if ( pd.m_func_enter ) return 2;
  return 0;
}

//'Suspend or resume profiling. Actually this is just wrapper on (undocumented) function moncontrol
//'
//'@param v 0 to suspend, 1 to resume
// [[Rcpp::export]]
void control(int v)
{
  moncontrol(v);
}

// state
static char s_prefix[60];
static int s_profiled = 0;
static void **s_patch_addr = nullptr;
static void *s_old_val = nullptr;

//'Stop profiling. results will be saved in current directory in file gmon.base_address_of_module.PID
//'
//'@return 0 when no profiling happened, 1 otherwise
// [[Rcpp::export]]
int prof_stop()
{
  if ( !s_profiled ) return 0;
  if ( s_patch_addr )
  {
    *s_patch_addr = s_old_val;
    s_patch_addr = nullptr;
  }
  setenv("GMON_OUT_PREFIX", s_prefix, 1);
  _mcleanup();
  s_profiled = 0;
  return 1;
}

//'Start profiling of some module
//'
//'@param fname full path to module
//'@return -1 if no such module or it is not ELF, 0 when module cannot be profiles, 1 if it was conpuled with -pg option, 2 fir -finstrument-functions option
// [[Rcpp::export]]
int prof_start(std::string fname)
{
  auto v = getenv("LD_PROFILE");
  if ( v )
  {
    warning("Warning: LD_PROFILE set, cannot start dynamic profiling");
    return 0;
  }
  if ( s_profiled )
  {
    warning("Warning: already profiling, call stop first");
    return 0;
  }
  ld_data ld { fname.c_str() };
  if ( !ld_iter(&ld) )
  {
    warning("Warning: module %s is not loaded", ld.name);
    return 0;
  }
  // check if we can profile this module
  struct prof_data pd;
  int res = process_elf(fname.c_str(), &pd);
  if ( res < 0 ) return 0;
  // -pg
  if ( pd.m_mcount )
  {
    sprintf(s_prefix, "gmon.%p", ld.base);
    monstartup(ld.base, ld.x_start + ld.x_size);
    s_profiled = 1;
    return 1;
  }
  if ( !pd.m_func_enter ) return 0;
  // -finstrument-functions
  sprintf(s_prefix, "gmon.%p", (void *)ld.base);
  s_patch_addr = (void **)(ld.base + pd.m_func_enter);
  s_old_val = *s_patch_addr;
  void *real_m = (void *)&mcount;
  *s_patch_addr = real_m;
  monstartup(ld.base, ld.x_start + ld.x_size);
  s_profiled = 1;
  return 2;
}