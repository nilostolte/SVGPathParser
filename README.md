# SVGPathParser
A compact parser for SVG path shape definitions. It allows to generate the same path in absolute coordinates for easy manipulations by hand or in relative coordinates, allowing easy manual placement. The parser is based on [Mikko Mononen](https://github.com/memononen)'s NanoSVG parser. The new parser uses a simpler and more accurate path format, using SVG commands as its internal representation. 

## Differences with NanoSVG parser
While NanoSVG parser generates only cubic Bezier curves, the new parser stores the path with SVG's original commands and points as much as possible. The only difficulty was with HLINETO and VLINETO single coordinate commands that had to be substituted by LINETO commands in order to allow rotations. Since the matrix is systematically applied to all shapes, two coordinates are necessary to allow the calculation. Another difficulty was with ARCTO commands which also requires storing the angle of the rotation besides the matrix.

## SVG generation
The code (source code and compiled executable a.exe) implements an application that needs inline imput arguments to be executed correctly. According to the parameters supplied, it will print on the console a complete SVG path command, either in absolute coordinates or in relative coordinates. In relative coordinates, the SVG generated is optimized substituting "l" commands by "h" or "v" commands when appropriate, and further optimized when either commands would be reduced to commands with zero as a parameter (as for example, "h0" or "v0"). In this case they are ignored, since they are equivalent to no operations.

The source code generates traces when DEBUG is defined. It also generates numbers in full float precision when VERBOSE is defined, otherwise all results are truncated to a maximum of 3 dgits after the decimal point.

## Application parameters
The only compulsory parameter for the application is the path to be parsed. It must appear between quotes and separated by spaces from the program executable name or from the other program paramenters. For example, one can call the program like this:

```
    a "M 100 0 A 100 50 0 1 1 100 -1"
```

The following commands can be also included in the same line separated by spaces from one another:

| Command | Parameter | Description | <span>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</span>Example<span>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</span> |
|-| - | - | - |
|**`-r`**| none | generates a path with relative coordinates | `-r` |
|**`-m`**| matrix | transformation matrix with 6 elements separated by spaces inside a string | `-m"1 0 0 1 100 100"` |
|**`-p`**| string<sup>*</sup> | attributes to be included in the path such as color, stroke-width, etc.| `-p"stroke=\"#DB362D\" fill=\"none\""` |
|**`-e`**| string<sup>**</sup> | commands to be included at the end of the path | `-pz` |
|**`-a`**| number | the rotation angle in degrees (can be supplied instead of a matrix) | `-a12.5` |

<b>*   </b> a string with only one attribute with no white spaces can appear without quotes<br>
<b>** </b> a string with only one word can appear without quotes

