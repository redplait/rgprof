# rgprof
R package for shared libraries profiling using dirty [hacks](http://redplait.blogspot.com/2024/03/profiling-shared-libraries-on-linux.html)

Usage
```R
library(rgprof)
rgprof::lsmod()
rgprog::prof_start(full_path_of_so)
...
rgprog::prof_stop()
```

Profiled package should be compiled with -pg or -finstrument-functions options (for example you can add then in [Makevars](https://rstudio.github.io/r-manuals/r-admin/Add-on-packages.html#customizing-package-compilation) file). Currently only aarch64 & x86_64 are supported
