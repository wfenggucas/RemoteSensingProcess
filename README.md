# lisa
large image spatial analysis

estimate the carbon loss due to forest fragmentation using remote sensing images

supported input raster formats: ESRI ASCII Grid (.asc), PNM (.pgm), (Geo)TIFF (.tif), BinaryRemoteImage (.bri)

.bri is an original filetype specialised for binary images using variable run length encoding

lisa is able to do
- a single pass connected component analysis for all supported input formats
- blend the analysis with biomass-files to estimate carbon loss
- save a fully clustered compressed image in a second pass
- produce arbitrary resampled geo-referenced density maps of carbon loss

lisa is able to operate on very large images consuming as little resources as possible
