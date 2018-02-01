# CoPPer - Control Performance with Power

A library that meets application performance goals by manipulating power caps.

For details, please see the following and reference as appropriate:

* Connor Imes, Huazhe Zhang, Kevin Zhao, Henry Hoffmann. "Handing DVFS to Hardware: Using Power Capping to Control Software Performance". Technical Report [TR-2018-03](https://cs.uchicago.edu/research/publications/techreports/TR-2018-03). University of Chicago, Department of Computer Science. 2018.

If looking to reproduce published results, please see the integration project [copper-eval](https://github.com/powercap/copper-eval).

## Building

This project uses CMake.

To build, run:

``` sh
mkdir _build
cd _build
cmake ..
make
```

## Installing

To install, run with proper privileges:

``` sh
make install
```

On Linux, installation typically places libraries in `/usr/local/lib` and
header files in `/usr/local/include`.

## Uninstalling

Install must be run before uninstalling in order to have a manifest.
To uninstall, run with proper privileges:

``` sh
make uninstall
```
## Usage

See code in the `example` directory.

## Project Source

Find this and related project sources at the [powercap organization on GitHub](https://github.com/powercap).  
This project originates at: https://github.com/powercap/copper

Bug reports and pull requests for bug fixes and enhancements are welcome.
