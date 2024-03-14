\name{rgprof-package}
\alias{rgprof-package}
\alias{rgprof}
\docType{package}
\title{
  Package for shared libraries profiling
}
\description{
  You can dynamically start and stop profiling many times for different libraries. Profiled package should be compiled with -pg or -finstrument-functions options 
}
\details{
  See \href{http://redplait.blogspot.com/2024/03/profiling-shared-libraries-on-linux.html}{details}
}
\author{
Red Plait.

Maintainer: Red Plait <redplait@gmail.com>
}
\references{
  This optional section can contain literature or other references for
  background information.
}
\keyword{ package }
\seealso{
  Optional links to other man pages
}
\examples{
  \dontrun{
   library(rgprof)
   rgprof::lsmod()
   rgprog::prof_start(full_path_of_so)
   ...
   rgprog::prof_stop()
  }
}
