% Generated by roxygen2 (4.1.0): do not edit by hand
% Please edit documentation in R/addImage.bsdoc.R
\name{addImage.bsdoc}
\alias{addImage.bsdoc}
\title{Insert an external image into a bsdoc object}
\usage{
\method{addImage}{bsdoc}(doc, filename, width, height,
  par.properties = parProperties(text.align = "center", padding = 5),
  ppi = 72, ...)
}
\arguments{
\item{doc}{\code{\link{bsdoc}} object where external image has to be added}

\item{filename}{\code{"character"} value, complete filename of the external image}

\item{width}{image width in inches}

\item{height}{image height in inches}

\item{par.properties}{paragraph formatting properties of the paragraph that contains images.
An object of class \code{\link{parProperties}}}

\item{ppi}{dot per inches, default to 72}

\item{...}{further arguments, not used.}
}
\value{
an object of class \code{\link{bsdoc}}.
}
\description{
Add an external image into a \code{\link{bsdoc}} object.
}
\details{
If arguments width and height are missing, values will be defined as
their respective number of pixels divide by \code{ppi}.
}
\examples{
#START_TAG_TEST
doc.filename = "addImage_bsdoc/example.html"

# set default font size to 11
options( "ReporteRs-fontsize" = 11 )

doc = bsdoc( )
# files 'logo.jpg' and 'logosm.jpg' only exist in R for Windows
img.file1 = file.path( Sys.getenv("R_HOME"), "doc", "html", "logo.jpg" )
img.file2 = file.path( Sys.getenv("R_HOME"), "doc", "html", "logosm.jpg" )

if( file.exists( img.file1 ) && file.exists( img.file2 ) ){
  doc = addTitle( doc, "Add images with defaut PPI (72)", level = 1)
  doc = addTitle( doc, "Image 1", level = 2)
  doc = addImage(doc, img.file1 )
  doc = addTitle( doc, "Image 2", level = 2)
  doc = addImage(doc, img.file2 )
  doc = addTitle( doc, "Add images with their respective PPI", level = 1)
  doc = addTitle( doc, "Image 1", level = 2)
  doc = addImage(doc, img.file1, ppi = 300 )
  doc = addTitle( doc, "Image 2", level = 2)
  doc = addImage(doc, img.file2, ppi = 96)
  doc = addTitle( doc, "Add images with width and height", level = 1)
  doc = addImage(doc, img.file1, width = 4, height = 2 )
}

# Write the object
writeDoc( doc, file = doc.filename )
#STOP_TAG_TEST
}
\seealso{
\code{\link{bsdoc}}, \code{\link{addPlot.bsdoc}}
, \code{\link{addImage}}
}

