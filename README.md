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
