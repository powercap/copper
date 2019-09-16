# CoPPer - Control Performance with Power

A library that meets application performance goals by manipulating power caps.

For details, please see the following and reference as appropriate:

* Connor Imes, Huazhe Zhang, Kevin Zhao, Henry Hoffmann. "CoPPer: Soft Real-time Application Performance Using Hardware Power Capping". In: IEEE International Conference on Autonomic Computing (ICAC). 2019. DOI: https://doi.org/10.1109/ICAC.2019.00015

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
