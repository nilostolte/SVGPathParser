# SVGPathParser
A compact parser for SVG path shape definitions. It allows to generate the same path in absolute coordinates for easy manipulations by hand or in relative coordinates, allowing easy manual placement. The parser is based on [Mikko Mononen](https://github.com/memononen)'s NanoSVG parser. The new parser uses a simpler and more accurate path format, using SVG commands as its internal representation. 

## Differences with NanoSVG parser
While NanoSVG parser generates only cubic Bezier curves, this parser stores the path with SVG's original commands and points as much as possible. The only difficulty was with HLINETO and VLINETO single coordinate commands that had to be substituted by LINETO commands in order to allow rotations. Since the matrix is systematically applied to all shapes, two coordinates are necessary to allow the calculation. Another difficulty was with ARCTO commands which also requires storing the angle of the rotation besides the matrix.

