# SVGPathParser
A compact parser for SVG path shape definitions. It allows to generate the same path in absolute coordinates for easy manipulations by hand or in relative coordinates, allowing easy manual placement. The parser is based on [Mikko Mononen](https://github.com/memononen)'s NanoSVG parser. The new parser uses a simpler and more accurate path format, using SVG commands as its internal representation. 

## Differences with NanoSVG parser
While NanoSVG parser generates only cubic Bezier curves, the new parser stores the path with SVG's original commands and points as much as possible. The only difficulty was with HLINETO and VLINETO single coordinate commands that had to be substituted by LINETO commands in order to allow rotations. Since the matrix is systematically applied to all shapes, two coordinates are necessary to allow the calculation. Another difficulty was with ARCTO commands which also requires storing the angle of the rotation besides the matrix.

## SVG generation
The code (source code and compiled executable a.exe) implements an application that needs inline imput arguments to be executed correctly. According to the parameters supplied, it will print on the console a complete SVG path command, either in absolute coordinates or in relative coordinates. In relative coordinates, the SVG generated is optimized substituting "l" commands by "h" or "v" commands when appropriate, and further optimized when either commands would be reduced to commands with zero as a parameter (as for example, "h0" or "v0"). In this case they are ignored, since they are equivalent to no operations.

Note that in the relative coordinates mode the first path command is always an "M" which supplies the initial coordinates of the path in absolute coordinates. This allows the path to be placed at the coordinates declared there, whereas all the other elements of the path will follow flawlessly, since they are all in relative coordinates. Even if the first command in a path is "m" it will always be interpreted as an "M" commands according to [W3C SVG standard](https://www.w3.org/TR/SVG2/paths.html#PathDataMovetoCommands).

The source code generates traces when DEBUG is defined. It also generates numbers in full float precision when VERBOSE is defined, otherwise all results are truncated to a maximum of 3 dgits after the decimal point.

## Application parameters
The only compulsory parameter for the application is the path to be parsed. It must appear between quotes and separated by spaces from the program executable name or from the other program paramenters. For example, supposing the executable "a.exe" called from a shell window, the program can be called like this:

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
|**`-e`**| string<sup>2</sup> | commands to be included at the end of the path | `-pz` |
|**`-a`**| number | the rotation angle in degrees (can be supplied instead of a matrix) | `-a12.5` |

1. a string with no white spaces (thus, with only one attribute) can appear without the external quotes</li>
2. a string with only one word can appear without quotes, but explict quotes are necessary if a command with arguments or several commands are used in this context

Since it is cumbersome to type commands each time one calls a program in a shell window, it is recommended to call the program using batch files. It is possible to generate entire SVG files only using batch files and calling the application from it, as illustrated in the following examples.

## Examples

### Example 1: [NASA Logo](https://github.com/nilostolte/SVGPathParser/blob/main/src/NASA)
<img src="https://github.com/nilostolte/SVGPathParser/blob/main/src/NASA/NASA-relative.svg" width="500">

The NASA logo above is a good example on how to parse the original SVG that mixes absolute and relative coordinates, to generate a file that contains either only absolute coordinates or only relative coordinates (except for the initial "M" command which must be in absolute coordinates).

The [NASA](https://github.com/nilostolte/SVGPathParser/blob/main/src/NASA) directory within this repository shows several files (described in the table below) which work assuming the hierarchy in the repository, that is, the executable is located in the parent directory and the files using the program are located in [NASA](https://github.com/nilostolte/SVGPathParser/blob/main/src/NASA) directory.

[NASA](https://github.com/nilostolte/SVGPathParser/blob/main/src/NASA) directory, thus contain the following files

| <span>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</span>Filename<span>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</span> | Description | 
| - | - |
|**NASA_logo.svg**| the original NASA logo in SVG |
|**NASA-absolute.svg**| the NASA logo converted to absolute coordinates using **NASAa.bat** |
|**NASA-relative.svg**| the NASA logo converted to absolute coordinates using **NASA.bat** |
|**NASAa.bat**| Windows batch file that reuses each path of the original NASA SVG logo and that generates them in absolute coordinates |
|**NASA.bat**| Windows batch file that reuses each path of the original NASA SVG logo and that generates them in relative coordinates |

The batch files contain the three paths in the original SVG passed as parameters to the parser, except for small changes. The most notable is that the third path in the original file becomes the second path in the batch files. This is because this path corresponds to second and third letters of the logo, and they should appear right after the first path, not as the last one as in the original file. Another change was made in few coordinates to avoid the generation of useless commands. This corrections were made by observing the initial coordinates generated by the parser in relative coordinates. 

This is a another use of the parser: to use relative coordinates to simplify paths. The careful reader will be able to easily spot these changes by examining the data used in the batch files and comparing them to the original file. Comments about these changes are not going to be addressed further in this text.

One should read the batch files to check how this logo was generated (please read the next section in order to decode some common expressions used in these batch files). Since the "echo" is off one will only see the resulting paths one after the other being displayed in the shell window. These paths can then be copied and pasted in a file containing already a standard SVG file header (`<svg>` tag at the start and `</svg>` tag at the end) and use it as an empty frame like in the examples above.

One could include the `<svg>` tags using the batch files as well, but for sake of simplicity one is assumed to just copy-paste the empty frame in an empty file and copy-paste the resulting convereted paths output by the batch files between the `<svg>` and `</svg>` tags. This is such a pretty trivial task that it is probably doesn't worth automatizing.



### Reading Windows batch files
The Windows batch files use some conventions that are specific to them. The examples cannot be entirely understood without reading these batch files. Programmers and users coming from other platforms will be able to understand them by using the following table of common expressions used in them:

| Batch expression | Description | 
| - | - |
|**@echo off**| command that says the calls made inside the batch files are not going to be shown including this command (bacause it's precedded by a "@" |
|**%~dp0..\a**| "%~dp0" indicates the current directory in which the batch file is located. "..\a" following "%~dp0" means that to call executable "a.exe" in the parent of the current directory |

### Example 1: [Ellipses](https://github.com/nilostolte/SVGPathParser/tree/main/src/ellipses)
<img src="https://github.com/nilostolte/SVGPathParser/blob/main/src/ellipses/ellipse46.svg" width="500">

