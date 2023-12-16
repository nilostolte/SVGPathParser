# SVGPathParser
A compact parser for SVG path shape definitions. It allows to generate the same path in absolute coordinates for easy manipulations by hand or in relative coordinates, allowing easy manual placement. The parser is based on [Mikko Mononen](https://github.com/memononen)'s NanoSVG parser. The new parser uses a simpler and more accurate path format, using SVG commands as its internal representation.

The source code containing the new parser is complemented with a `main` function that receives the string to be parsed, which is the "d" attribute of a path. The program can also receive other arguments as detailed [below](https://github.com/nilostolte/SVGPathParser/tree/main#application-parameters). Once the executable is run, a path with equivalent "d" attribute is displayed, but converted entirely to absolute coordinates or to relative coordinates. The absolute coordinates allow to easily debug it or to reuse parts of the path to contruct new paths with, while the relative coordinates allow to easily experiment with the path in diferent positions. 

The resulting application can be helpful to designers or programmers willing to resuse the path in different contexts or to experienced web programmers to redesign the path by hand. It is possible, for example, to redesign the entire path only using integers. The path can be repositioned and/or have its scale changed and/or rotated (passing a matrix with "-m" argument) to mix with other paths defined in different scales. This kind of activity was only possible using vector graphics editors like Illustrator or Inkscape, but even in these applications the automatization of this kind of task is hard. Here it only requires using a batch or a script file. 

The source code in itself shows how to use the parser programatically and allows to easily extract the parser to be used in other contexts. The main motivation that came into mind when rewriting this parser was the possibility to reuse it in transpilers, where a GUI defined in SVG can be translated to a specific language or library.

## Differences with NanoSVG parser
While NanoSVG parser generates only cubic Bezier curves, the new parser stores the path with SVG's original commands and points as much as possible. The only difficulty was with HLINETO and VLINETO single coordinate commands that had to be substituted by LINETO commands in order to allow rotations. Since the matrix is systematically applied to all shapes, two coordinates are necessary to allow the calculation. Another difficulty was with ARCTO commands which also requires storing the angle of the rotation besides the matrix.

## SVG generation
The code (source code and compiled executable a.exe) implements an application that needs inline input arguments to be executed correctly. According to the parameters supplied, it will print on the console a complete SVG path command, either in absolute coordinates or in relative coordinates (using "-r" - see [below](https://github.com/nilostolte/SVGPathParser/tree/main#application-parameters)). The SVG generated is optimized substituting "l" or "L" commands by "h", "H", "V" or "v" commands when appropriate, and further optimized when either of these commands would not change the current point (as for example, "h0" or "v0"). In this case they are ignored, since they are equivalent to no operations. The program also eliminates spaces between commands, since this allows to save space.

Notice that in the relative coordinates mode the first path command is always an "M" which supplies the initial coordinates of the path in absolute coordinates. This allows the path to be placed at the coordinates declared there, whereas all the other elements of the path will follow flawlessly, since they are all in relative coordinates. Even if the first command in a path is "m" it will always be interpreted as an "M" command according to [W3C SVG standard](https://www.w3.org/TR/SVG2/paths.html#PathDataMovetoCommands).

The source code generates traces when DEBUG is defined. It also generates numbers in full float precision when VERBOSE is defined, otherwise all results are truncated to a maximum of 3 digits after the decimal point.

### Application parameters
The only compulsory parameter for the application is the "d" attribute of a path that is to be parsed. This attribute completely describes the shape of the path without the need of other atributes. It must appear between quotes and separated by spaces from the program executable name or from the other program parameters. If no other parameters are given, the path to be parsed is converted to absolute coordinates and shown in the shell window. For example, supposing the executable "a.exe" called from a shell window, the program can be called like this:

```
    ./a "M100,0 a100,50 0 1 1 0,-1 "
```

The program will display in the next line the same path in absolute coordinates:

```
<path d="M100,0A100,50 0 1 1 100,-1"/>
```

Since only the "d" attribute of the path is passed as a parameter here, the program only adds the path tag, besides converting the path to absolute coordinates as shown. To add more path attributes besides "d" one can use the "-p" command explained below. Notice that one can drop the "./" prefix when calling from a batch file. Also notice that "a.exe" ("a.out" on Linux) is the standard output of C/C++ compilers, when the executable is not explicitly named. For the sake of simplicity and since the source and the executable files are both unique in the directory containing them, "a.exe" is implictly refering to the source file "SVGparser.c".

A summary of the commands that can be in the same line of the program call, all separated by spaces from one another, are shown in the table below:

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
One should read the batch (or the _bash_ scripts) files to check how the examples are generated. By running any of the batch files one will see the resulting paths one after the other being displayed in the shell window. These paths can then be copied and pasted in a file containing already a standard SVG file header (`<svg>` tag at the start and `</svg>` tag at the end) and use it as an empty frame like in the [examples](https://github.com/nilostolte/SVGPathParser#examples). See how to create an SVG file header and how to set a viewport [below](https://github.com/nilostolte/SVGPathParser#assembling-an-svg-and-using-viewport).

#### Compiling and using the _bash_ scripts with w64devkit (Windows)
One possibility for contructing batch files is using [w64devkit](https://github.com/skeeto/w64devkit/releases) _bash_ scripts. These scripts are supplied in this repository. They have the suffix `.sh` after their names and have `#!/bin/bash` in their first line. These scripts are prefered instead of Windows batch files because they are easier to understand and because they can also be used on **Linux** platforms. The only inconvenient in running them on Windows is that they cannot run in standard Windows command terminals without some previous preparation. To be able to run _bash_ scripts with w64devkit, the most innocuous way (w64devkit.exe is not advised) is following these steps:

1. Open a Windows command terminal, not a Powershell: right-click at `Start`, then click `Run`, type `cmd.exe` and click `OK` or press `Enter`.
2. Transform the Windows command terminal into a Linux command window: type `sh -l` and press `Enter`
3. Go to the directory where your bash script is: copy the location from the explorer window, type `cd` followed by a blank space, type `"`, right-click the command window header, choose `Edit > Paste` (Control-V doesn't work), type `"` again and press `Enter`.
4. Type the name of the bash and press `Enter`.

Steps 1 to 3 initialize any Windows command terminal to use any Linux commands with w64devkit. Many commands work on Powershell as well, as for example calling the `gcc` compiler, but not _bash_.

To compile on Windows with w64devkit one merely calls gcc:

```batchfile
gcc SVGparser.c
```
To run the _bash_ scripts to run the compiled program, one needs to go to the `NASA` and `ellipses` directories first. From those directories one simply calls the _bash_ scripts from there. For example, in `NASA` diectory:

```batchfile
./NASA.sh
```
Once the _bash_ scripts are run, the content can be copied by just selecting and pressing `Enter`.

#### Compiling and using the _bash_ scripts on Linux
To compile SVGparser.c on Linux use the script `build_linux.sh`. Linux requires to compile with C99 standard and to explicitly link math library because of some float functions that are needed by the parser (with w64devkit that's not necessary). Also, on Linux, the executable is called "a.out." To use the same _bash_ script on Windows and on Linux, one needs to change the name of the executable file from "a.out" to "a.exe". All this is done by the build script `build_linux.sh`. Also, on Linux one needs to explicitly type the ".sh" suffix in order to run the script.

Therefore, on Linux one needs to call the build script in this way:

```bash
./build_linux.sh
```

To run the _bash_ scripts to run the compiled program, one needs to proceed in the same way as described in the previous section.

Once the _bash_ scripts are run, the content can be copied by just selecting and right-cliking.

#### Reading Windows batch files
The Windows batch files use some conventions that are specific to them. A table of common expressions used in Windows batch files is shown below

| Batch<span>&nbsp;</span>expression | Description | 
| - | - |
|**@echo off**| command that says the commands executed by the batch file are not going to be shown, including this command (because it's preceded by a "@") |
|**%~dp0..\a**| "%~dp0" indicates the current directory in which the batch file is located. "..\a" following "%~dp0" means to call executable "a.exe" in the parent of the current directory |

### Assembling an SVG and using viewport
 The viewport is an essencial part of SVG graphics because it defines the bounding box of all the shapes between `<svg>` and `<\svg>`. Some SVG viewers require the viewport in order to display the SVG contents and the contents can be cut or be invisible if the viewport is incorrect.

 This is how an SVG with a viewport is defined:

```SVG
<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 230 130">
<!-- the paths are to be inserted here -->
</svg>
```
This viewport indicates that the SVG is to be viewed into a rectangle between points (0,0) and (230,130). Supposing the following graphics, this viewport is defined by the rectangle:

<p align="center">
    <img src="https://github.com/nilostolte/SVGPathParser/blob/main/viewport.svg" width="400">
</p>

The viewport attribute of the `<svg>` tag is entirely the user's resposibility, mainly because one is supposed to add elements by hand into the SVG at will. Therefore, only the user will know what will be the bounding box enveloping all the elements in an svg manipulated in this way. This is also simpler and more compatible with other vector formats and languages where the bounding box is rarely calculated automatically.


## Examples

### Example 1: [NASA Logo](https://github.com/nilostolte/SVGPathParser/blob/main/src/NASA)
<p align="center">
    <img src="https://github.com/nilostolte/SVGPathParser/blob/main/src/NASA/NASA-relative.svg" width="509">
</p>

The NASA logo above is a good example on how to parse the original SVG that mixes absolute and relative coordinates, to generate a file that contains either only absolute coordinates or only relative coordinates (except for the initial "M" command which must be in absolute coordinates).

The [NASA](https://github.com/nilostolte/SVGPathParser/blob/main/src/NASA) directory within this repository shows several files (described in the table below) which work assuming the hierarchy in the repository, that is, the executable is located in the parent directory and the files using the program are located in [NASA](https://github.com/nilostolte/SVGPathParser/blob/main/src/NASA) directory.

[NASA](https://github.com/nilostolte/SVGPathParser/blob/main/src/NASA) directory, thus contain the following files

| <span>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</span>Filename<span>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</span> | Description | 
| - | - |
|**NASA_logo.svg**| the original NASA logo in SVG |
|**NASA-absolute.svg**| the NASA logo converted to absolute coordinates using **NASAa.bat** or **NASAa.sh** |
|**NASA-relative.svg**| the NASA logo converted to relative coordinates using **NASA.bat** or **NASA.sh**|
|**NASAa.bat**| Windows batch file that reuses each path of the original NASA SVG logo and that generates them in absolute coordinates |
|**NASA.bat**| Windows batch file that reuses each path of the original NASA SVG logo and that generates them in relative coordinates |
|**NASAa.sh**| _bash_ script producing the same result as **NASAa.bat**   |
|**NASA.sh**| _bash_ script producing the same result as **NASA.bat**   |

The batch files contain the three paths in the original SVG passed as parameters to the parser, except for small changes. The most notable is that the third path in the original file becomes the second path in the batch files. This is because this path corresponds to second and third letters of the logo, and they should appear right after the first path, not as the last one as in the original file. Another change was made in few coordinates to avoid the generation of useless commands. This corrections were made by observing the initial coordinates generated by the parser in relative coordinates. 

This is a another use of the parser: to use relative coordinates to simplify paths. The careful reader will be able to easily spot these changes by examining the data used in the batch files and comparing them with the original file. Comments about these changes are not going to be addressed further in this text.

### Example 2: [Rotated ellipses](https://github.com/nilostolte/SVGPathParser/tree/main/src/ellipses)
<p align="center">
    <img src="https://github.com/nilostolte/SVGPathParser/blob/main/src/ellipses/ellipse46.svg" width="500">
</p>

The ellipse is probably one of the most particular path commands in SVG and this example was required to test "A" or "a" commands in the parser to be used to draw rotated ellipses. On the other hand, rotated SVG ellipses are easier to be generated on the fly by using _ad hoc_ functions in JavaScript. An experienced JavaScript programmer could produce the same paths on the fly and then use inspect to copy the SVG paths generated. The purpose of this example is to demonstrate an alternative way to generate rotated static SVG ellipses without the need of programming using an initial trivial ellipse centered at the origin defined in absolute coordinates aligned with both axes, and to use transformation matrices to rotate it and to translate it to the desired point.

The [ellipses](https://github.com/nilostolte/SVGPathParser/blob/main/src/ellipses) directory within this repository include two simpler examples and a more complex one containing both examples in the same svg file (the image above is a rendering of the file mentioned here that shows both examples side by side).

The first simple example generates [4 ellipses](https://github.com/nilostolte/SVGPathParser/blob/main/src/ellipses/ellipse4.svg), each one rotated 45 degrees from the previous one. One uses [ellipse4.bat](https://github.com/nilostolte/SVGPathParser/blob/main/src/ellipses/ellipse4.bat):

```batchfile
@echo off
%~dp0..\a -r -p"fill=\"none\" stroke=\"red\" stroke-width=\"2\"" -ez -m"1 0 0 1 100 100" "M 100 0 A 100 50 0 1 1 100 -1"
%~dp0..\a -r -p"fill=\"none\" stroke=\"green\" stroke-width=\"2\"" -ez -m".70710678 .70710678 -0.70710678 .70710678 100 100" "M 100 0 A 100 50 0 1 1 100 -1"
%~dp0..\a -r -p"fill=\"none\" stroke=\"blue\" stroke-width=\"2\"" -ez -m"0 1 -1 0 100 100" "M 100 0 A 100 50 0 1 1 100 -1"
%~dp0..\a -r -p"fill=\"none\" stroke=\"magenta\" stroke-width=\"2\"" -ez -m"-.70710678 .70710678 -0.70710678 -.70710678 100 100" "M 100 0 A 100 50 0 1 1 100 -1"
```

Or the [ellipse4.sh](https://github.com/nilostolte/SVGPathParser/blob/main/src/ellipses/ellipse4.sh) script:

```shell
#!/bin/bash
../a.exe -r -p"fill=\"none\" stroke=\"red\" stroke-width=\"2\"" -ez -m"1 0 0 1 100 100" "M 100 0 A 100 50 0 1 1 100 -1"
../a.exe -r -p"fill=\"none\" stroke=\"green\" stroke-width=\"2\"" -ez -m".70710678 .70710678 -0.70710678 .70710678 100 100" "M 100 0 A 100 50 0 1 1 100 -1"
../a.exe -r -p"fill=\"none\" stroke=\"blue\" stroke-width=\"2\"" -ez -m"0 1 -1 0 100 100" "M 100 0 A 100 50 0 1 1 100 -1"
../a.exe -r -p"fill=\"none\" stroke=\"magenta\" stroke-width=\"2\"" -ez -m"-.70710678 .70710678 -0.70710678 -.70710678 100 100" "M 100 0 A 100 50 0 1 1 100 -1"
```
The batch and script files force all four ellipses to be generated in relative coordinates ("-r"), with no fill, stroke width of 2, and each one with a different color ("-p"). Notice that the initial ellipse path is always the same and given in absolute coordinates. The ellipse is centered at the origin with initial point at (100, 0), end point at (100, -1), with x axis radius of 100, y axis radius of 50, and a zero degrees angle:

```svg
"M 100 0 A 100 50 0 1 1 100 -1"
```

Notice that this path is not closed. This is done because ellipses and circles inside SVG paths cannot be closed, since the initial and end point must be different. One could have used the "z" command in the original ellipse to close it, but here the "-ez" command is used to close each ellipse. By removing this command the gap between the start point and the end point shows how angles grow in SVG. They do it in a clockwise manner because the y axis is inverted since it always points down. The produced paths give the following pattern:

<p align="center">
<img src="https://github.com/nilostolte/SVGPathParser/blob/main/src/ellipses/ellipse4.svg" width="250">
</p>

This example allows to easily understand the use of matrices to produce rotations of 0°, 45°, 90° and 135°.  The generated paths contain exactly these angles with start and end point rotated by these angles about the ellipses center point, the point (100, 100) as seen in the translation part of the matrices.

```SVG
<svg xmlns="http://www.w3.org/2000/svg" viewBox="-2 -2 204 204">
<path fill="none" stroke="red" stroke-width="2" d="M200,100a100,50 0 1 1 0,-1z"/>
<path fill="none" stroke="green" stroke-width="2" d="M170.711,170.711a100,50 45 1 1 0.707,-0.707z"/>
<path fill="none" stroke="blue" stroke-width="2" d="M100,200a100,50 90 1 1 1,0z"/>
<path fill="none" stroke="magenta" stroke-width="2" d="M29.289,170.711a100,50 135 1 1 0.707,0.707z"/>
</svg>
```

The second simple example generates [6 ellipses](https://github.com/nilostolte/SVGPathParser/blob/main/src/ellipses/ellipse6.svg), each one rotated 30 degrees from the previous one. One uses [ellipse6.bat](https://github.com/nilostolte/SVGPathParser/blob/main/src/ellipses/ellipse6.bat) or [ellipse6.sh](https://github.com/nilostolte/SVGPathParser/blob/main/src/ellipses/ellipse6.sh) to generate the paths. Here the matrices produce rotations of 0°, 30°, 60°, 90°, 120°, and 150°, resulting in the following pattern:

<p align="center">
<img src="https://github.com/nilostolte/SVGPathParser/blob/main/src/ellipses/ellipse6.svg" width="250">
</p>

The more complex example combine both simpler examples above together in a single [svg file](https://github.com/nilostolte/SVGPathParser/blob/main/src/ellipses/ellipse46.svg). One uses [ellipse46.bat](https://github.com/nilostolte/SVGPathParser/blob/main/src/ellipses/ellipse46.bat) or [ellipse46.sh](https://github.com/nilostolte/SVGPathParser/blob/main/src/ellipses/ellipse46.sh) to generate the paths. The result was shown [above](https://github.com/nilostolte/SVGPathParser/tree/main#example-2-rotated-ellipses).
