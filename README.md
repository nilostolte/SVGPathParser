# SVGPathParser
A compact parser for SVG path shape definitions. It allows to generate the same path in absolute coordinates for easy manipulations by hand or in relative coordinates, allowing easy manual placement. The parser is based on [Mikko Mononen](https://github.com/memononen)'s NanoSVG parser. The new parser uses a simpler and more accurate path format, using SVG commands as its internal representation. 

## Differences with NanoSVG parser
While NanoSVG parser generates only cubic Bezier curves, the new parser stores the path with SVG's original commands and points as much as possible. The only difficulty was with HLINETO and VLINETO single coordinate commands that had to be substituted by LINETO commands in order to allow rotations. Since the matrix is systematically applied to all shapes, two coordinates are necessary to allow the calculation. Another difficulty was with ARCTO commands which also requires storing the angle of the rotation besides the matrix.

## SVG generation
The code (source code and compiled executable a.exe) implements an application that needs inline input arguments to be executed correctly. According to the parameters supplied, it will print on the console a complete SVG path command, either in absolute coordinates or in relative coordinates. In relative coordinates, the SVG generated is optimized substituting "l" commands by "h" or "v" commands when appropriate, and further optimized when either commands would be reduced to commands with zero as a parameter (as for example, "h0" or "v0"). In this case they are ignored, since they are equivalent to no operations.

Note that in the relative coordinates mode the first path command is always an "M" which supplies the initial coordinates of the path in absolute coordinates. This allows the path to be placed at the coordinates declared there, whereas all the other elements of the path will follow flawlessly, since they are all in relative coordinates. Even if the first command in a path is "m" it will always be interpreted as an "M" commands according to [W3C SVG standard](https://www.w3.org/TR/SVG2/paths.html#PathDataMovetoCommands).

The source code generates traces when DEBUG is defined. It also generates numbers in full float precision when VERBOSE is defined, otherwise all results are truncated to a maximum of 3 digits after the decimal point.

### Application parameters
The only compulsory parameter for the application is the path to be parsed. It must appear between quotes and separated by spaces from the program executable name or from the other program parameters. For example, supposing the executable "a.exe" called from a shell window, the program can be called like this:

```
    ./a "M 100 0 A 100 50 0 1 1 100 -1"
```

Notice that one can drop the "./" prefix when calling from a batch file. Also notice that "a.exe" is the standard ouput of C/C++ compilers, when the executable is not explicitly named. One could name it in a more appropriate way, but for the sake of simplicity, only the source code has a proper name, while the executable, being unique in the directory containing it, is implictly refering to that source code, since it is also unique in the directory.

The following commands can be also included in the same line of the program call, all separated by spaces from one another:

| Command | Parameter | Description | <span>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</span>Example<span>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</span> |
|-| - | - | - |
|**`-r`**| none | generates a path with relative coordinates | `-r` |
|**`-m`**| matrix | transformation matrix with 6 elements separated by spaces inside a string | `-m"1 0 0 1 100 100"` |
|**`-p`**| string<sup>1</sup> | attributes to be included in the path such as color, stroke-width, etc.| `-p"stroke=\"#DB362D\" fill=\"none\""` |
|**`-e`**| string<sup>2</sup> | commands to be included at the end of the path | `-ez` |
|**`-a`**| number | the rotation angle in degrees (can be supplied instead of a matrix) | `-a12.5` |

1. a string with no white spaces (thus, with only one attribute) can appear without the external quotes</li>
2. a string with only one word can appear without quotes, but explict quotes are necessary if a command with arguments or several commands are used in this context

Since it is cumbersome to type commands each time one calls a program in a shell window, it is recommended to call the program using batch files. It is possible to generate entire SVG files only using batch files and calling the application from it, as illustrated in the following examples.

### Generating the SVG files with batch files
One should read the batch files to check how the examples are generated (please read the next section in order to decode some common expressions used in these batch files). Since the "echo" is off one will only see the resulting paths one after the other being displayed in the shell window. These paths can then be copied and pasted in a file containing already a standard SVG file header (`<svg>` tag at the start and `</svg>` tag at the end) and use it as an empty frame like in the examples below.

One could include the `<svg>` tags using the batch files as well, but for the sake of simplicity one is assumed to just copy-paste these tags in an empty file and copy-paste between them the resulting converted paths output by a batch file. This is such a trivial task that it probably doesn't worth automatizing. Also, proceding in this way one can easily paste the resulting SVG file paths into another SVG file directly.

Also of note, the viewport attribute of the `<svg>` tag is entirely the user's resposibility. The reasoning behind this is simplicity and compatibility with other vector formats and languages where the bounding box is rarely calculated automatically. In any case, the interested user can obtain the bounding box of a path by converting the path to a succession of cubic Bezier curves using the appropriate functions given in the original [parser](https://raw.githubusercontent.com/memononen/nanosvg/master/src/nanosvg.h).

The goal of this parser is to be simple and to faithfully reproduce the original path. To transform the path into cubic Bezier curves is a completely different task and it cannot be considered as part of the parser as has been originally done. The decision to code these convertions was probably to simplify the representation in order to display them. However, this simplification requires quite long and complex functions which would contribute very little to the tasks addressed here.

### Reading Windows batch files
The Windows batch files use some conventions that are specific to them. The examples cannot be entirely understood without reading these batch files. Programmers and users coming from other platforms will be able to understand them by using the following table of common expressions used in them:

| Batch<span>&nbsp;</span>expression | Description | 
| - | - |
|**@echo off**| command that says the commands executed by the batch file are not going to be shown, including this command (because it's preceded by a "@") |
|**%~dp0..\a**| "%~dp0" indicates the current directory in which the batch file is located. "..\a" following "%~dp0" means to call executable "a.exe" in the parent of the current directory |

## Examples

### Example 1: [NASA Logo](https://github.com/nilostolte/SVGPathParser/blob/main/src/NASA)
<p align="center">
    <img src="https://github.com/nilostolte/SVGPathParser/blob/main/src/NASA/NASA-relative.svg" width="500">
</p>

The NASA logo above is a good example on how to parse the original SVG that mixes absolute and relative coordinates, to generate a file that contains either only absolute coordinates or only relative coordinates (except for the initial "M" command which must be in absolute coordinates).

The [NASA](https://github.com/nilostolte/SVGPathParser/blob/main/src/NASA) directory within this repository shows several files (described in the table below) which work assuming the hierarchy in the repository, that is, the executable is located in the parent directory and the files using the program are located in [NASA](https://github.com/nilostolte/SVGPathParser/blob/main/src/NASA) directory.

[NASA](https://github.com/nilostolte/SVGPathParser/blob/main/src/NASA) directory, thus contain the following files

| <span>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</span>Filename<span>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</span> | Description | 
| - | - |
|**NASA_logo.svg**| the original NASA logo in SVG |
|**NASA-absolute.svg**| the NASA logo converted to absolute coordinates using **NASAa.bat** |
|**NASA-relative.svg**| the NASA logo converted to relative coordinates using **NASA.bat** |
|**NASAa.bat**| Windows batch file that reuses each path of the original NASA SVG logo and that generates them in absolute coordinates |
|**NASA.bat**| Windows batch file that reuses each path of the original NASA SVG logo and that generates them in relative coordinates |

The batch files contain the three paths in the original SVG passed as parameters to the parser, except for small changes. The most notable is that the third path in the original file becomes the second path in the batch files. This is because this path corresponds to second and third letters of the logo, and they should appear right after the first path, not as the last one as in the original file. Another change was made in few coordinates to avoid the generation of useless commands. This corrections were made by observing the initial coordinates generated by the parser in relative coordinates. 

This is a another use of the parser: to use relative coordinates to simplify paths. The careful reader will be able to easily spot these changes by examining the data used in the batch files and comparing them with the original file. Comments about these changes are not going to be addressed further in this text.

### Example 2: [Rotated ellipses](https://github.com/nilostolte/SVGPathParser/tree/main/src/ellipses)
<p align="center">
    <img src="https://github.com/nilostolte/SVGPathParser/blob/main/src/ellipses/ellipse46.svg" width="500">
</p>

The ellipse is probably one of the most particular path commands in SVG and this example was required to test "A" or "a" commands in the parser to be used to draw rotated ellipses. On the other hand, rotated SVG ellipses are easier to be generated by on the fly by _ad hoc_ functions in JavaScript. An experienced JavaScript programmer could produce the same paths on the fly and then use inspect to copy the SVG paths generated. The purpose of this example is to demonstrate an alternative way to generate rotated static SVG ellipses without the need of programming using an initial trivial ellipse centered at the origin defined in absolute coordinates aligned with both axes, and to use transformation matrices to rotate it and to translate it elsewhere.

The [ellipses](https://github.com/nilostolte/SVGPathParser/blob/main/src/ellipses) directory within this repository include two simpler examples and a more complex one containing both examples in the same svg file (the image above is a rendering of the file mentioned here that shows both examples side by side).

The first simple example generates [4 ellipses](https://github.com/nilostolte/SVGPathParser/blob/main/src/ellipses/ellipse4.svg), each one rotated 45 degrees from the previous one. One uses [ellipse4.bat](https://github.com/nilostolte/SVGPathParser/blob/main/src/ellipses/ellipse4.bat):

```batchfile
@echo off
%~dp0..\a -r -p"fill=\"none\" stroke=\"red\" stroke-width=\"2\"" -ez -m"1 0 0 1 100 100" "M 100 0 A 100 50 0 1 1 100 -1"
%~dp0..\a -r -p"fill=\"none\" stroke=\"green\" stroke-width=\"2\"" -ez -m".70710678 .70710678 -0.70710678 .70710678 100 100" "M 100 0 A 100 50 0 1 1 100 -1"
%~dp0..\a -r -p"fill=\"none\" stroke=\"blue\" stroke-width=\"2\"" -ez -m"0 1 -1 0 100 100" "M 100 0 A 100 50 0 1 1 100 -1"
%~dp0..\a -r -p"fill=\"none\" stroke=\"magenta\" stroke-width=\"2\"" -ez -m"-.70710678 .70710678 -0.70710678 -.70710678 100 100" "M 100 0 A 100 50 0 1 1 100 -1"
```
The batch file requires all four ellipses to be generated in relative coordinates ("-r"), with no fill, stroke width of 2, and each one with a different color ("-p"). Notice that the initial ellipse path is always the same and given in absolute coordinates. The ellipse is centered at the origin with initial point at (100, 0), end point at (100, -1), with x axis radius of 100, y axis radius of 50, and a zero degrees angle:

```svg
"M 100 0 A 100 50 0 1 1 100 -1"
```

Notice that this path is not closed. This is done because ellipses in SVG cannot be closed, since the initial and end point must be different. One could have used the "z" command in the original ellipse to close it, but here the "-ez" command is used to close each ellipse. By removing this command the gap between the start point and the end point shows how angles grow in SVG. They do it in a clockwise manner because the y axis is inverted since it always points down. The produced paths give the following pattern:

<p align="center">
<img src="https://github.com/nilostolte/SVGPathParser/blob/main/src/ellipses/ellipse4.svg" width="250">
</p>

This example allows to easily understand the use of matrices to produce rotations of 0°, 45°, 90° and 135°.  The generated paths contain exactly these angles with start and end point rotated by these angles about the ellipses center point, the point (100, 100) as seen in the translation part of the matrices.

```svg
<svg xmlns="http://www.w3.org/2000/svg" viewBox="-2 -2 204 204">
<path fill="none" stroke="red" stroke-width="2" d="M200,100 a100,50 0 1 1 0,-1 z"/>
<path fill="none" stroke="green" stroke-width="2" d="M170.711,170.711 a100,50 45 1 1 0.707,-0.707 z"/>
<path fill="none" stroke="blue" stroke-width="2" d="M100,200 a100,50 90 1 1 1,0 z"/>
<path fill="none" stroke="magenta" stroke-width="2" d="M29.289,170.711 a100,50 135 1 1 0.707,0.707 z"/>
</svg>
```

The second simple example generates [6 ellipses](https://github.com/nilostolte/SVGPathParser/blob/main/src/ellipses/ellipse6.svg), each one rotated 30 degrees from the previous one. One uses [ellipse6.bat](https://github.com/nilostolte/SVGPathParser/blob/main/src/ellipses/ellipse6.bat) to generate the paths. Here the matrices produce rotations of 0°, 30°, 60°, 90°, 120°, and 150°, resulting in the following pattern:

<p align="center">
<img src="https://github.com/nilostolte/SVGPathParser/blob/main/src/ellipses/ellipse6.svg" width="250">
</p>

The more complex example combine both simpler examples above together in a single [svg file](https://github.com/nilostolte/SVGPathParser/blob/main/src/ellipses/ellipse46.svg). One uses [ellipse46.bat](https://github.com/nilostolte/SVGPathParser/blob/main/src/ellipses/ellipse46.bat) to generate the paths. The result was shown above in the rendering of two examples side by side.
