# fsinodes
Filesystem using i-nodes

Seminar Work of KIV/ZOS

> University of West Bohemia, Pilsen

C++20 (Linux)

## Build

`$ cmake -Bbuild -Ssrc && cd build && make`

## Usage

`$ ./fsinodes <fs-data>`

	<fs-data> - File containing the filesystem data


## Example

### Server

`build$ ./fsinodes fs.dat`

	Filesystem is simulated with underlying data of fs.dat.
