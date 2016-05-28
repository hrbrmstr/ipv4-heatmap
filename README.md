# ipv4-heatmap

Update to The Measurement Factory ipv4-heatmap codebase (i.e. the seminal code to generate Hilbert curve IPv4 heatmaps)

This new one uses the [viridis](https://cran.r-project.org/web/packages/viridis/vignettes/intro-to-viridis.html) color palette which is much easier on the eyes. You can also used a few colorbrewer palettes: `brbg puor` `rdbu` `rdgy` `rdylbu` `spectral` `bupu` `reds` `ylgnbu` `ylorbr` `ylorrd` via the `-P palette` directive and invert the sequence with the `-i` option.

![](map.png)