#!/usr/bin/env Rscript --vanilla
library(viridis)

# get 256 viridis colors

v <- col2rgb(gsub("FF$", "", viridis(256)))

# but that creates duplicate entries which is a problem since that will
# cause the gd code in ipv4heatmap.c to not increment past them; so,
# we locate the dups and tweak them a bit

v <- data.frame(t(v))
dups <- which(duplicated(v))
v[dups,] <- v[dups,] - 1

# and generate viridis.h

cat(sprintf("/* GENERATED AUTOMATICALLY DO NOT EDIT BY HAND */\n/* SEE viridisgen.r for details */\n\ndouble red[] = { %s } ;\ndouble green[] = { %s } ;\ndouble blue[] = { %s } ;\n", 
            paste0(v$red, collapse=", "),
            paste0(v$green, collapse=", "),
            paste0(v$blue, collapse=", ")), file="viridis.h")

