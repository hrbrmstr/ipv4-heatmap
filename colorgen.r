#!/usr/bin/env Rscript --vanilla
suppressWarnings(suppressPackageStartupMessages(library(RColorBrewer)))
suppressWarnings(suppressPackageStartupMessages(library(viridis)))
suppressWarnings(suppressPackageStartupMessages(library(dplyr)))
suppressWarnings(suppressPackageStartupMessages(library(purrr)))

# get 256 viridis colors

v <- col2rgb(gsub("FF$", "", viridis(256)))

# but that creates duplicate entries which is a problem since that will
# cause the gd code in ipv4heatmap.c to not increment past them; so,
# we locate the dups and tweak them a bit

v <- data.frame(t(v))
dups <- which(duplicated(v))
v[dups,] <- v[dups,] - 1

# and generate colors.h

color_fil <- "colors.h"

cat(sprintf("/* GENERATED AUTOMATICALLY DO NOT EDIT BY HAND */\n/* SEE colorgen.r for details */\n\ndouble viridis_red[] = { %s } ;\ndouble viridis_green[] = { %s } ;\ndouble viridis_blue[] = { %s } ;\n",
            paste0(v$red, collapse=", "),
            paste0(v$green, collapse=", "),
            paste0(v$blue, collapse=", ")), file=color_fil)

# give some more choices

diverg <- c("BrBG", "PuOr", "RdBu", "RdGy", "RdYlBu", "Spectral")

walk(diverg, function(x) {
  b <- data.frame(t(col2rgb(colorRampPalette(brewer.pal(11, x))(256))))
  if (sum(duplicated(b))>0) {
    print(x)
    print(sum(duplicated(b)))
    print(b[duplicated(b),])
  }
  cat(sprintf("\ndouble %s_red[] = { %s } ;\ndouble %s_green[] = { %s } ;\ndouble %s_blue[] = { %s } ;\n",
              tolower(x),
              paste0(b$red, collapse=", "),
              tolower(x),
              paste0(b$green, collapse=", "),
              tolower(x),
              paste0(b$blue, collapse=", ")),
      file=color_fil, append=TRUE)
})

sequent <- c("BuPu", "Reds", "YlGnBu", "YlOrBr", "YlOrRd")
walk(sequent, function(x) {
  b <- data.frame(t(col2rgb(colorRampPalette(brewer.pal(9, x))(256))))
  if (sum(duplicated(b))>0) {
    print(x)
    print(sum(duplicated(b)))
    print(b[duplicated(b),])
    print("================")
  }
  cat(sprintf("\ndouble %s_red[] = { %s } ;\ndouble %s_green[] = { %s } ;\ndouble %s_blue[] = { %s } ;\n",
              tolower(x),
              paste0(b$red, collapse=", "),
              tolower(x),
              paste0(b$green, collapse=", "),
              tolower(x),
              paste0(b$blue, collapse=", ")),
      file=color_fil, append=TRUE)
})


cat("\ndouble red[256];\ndouble green[256];\ndouble blue[256];\n",
    file=color_fil, append=TRUE)

