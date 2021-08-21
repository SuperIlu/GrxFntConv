# GrxFntConv
This tool (aka "dirty hack") can be used to convert TTF or BDF fonts to the GRX format used by [DOjS](https://github.com/SuperIlu/DOjS), [Allegro](https://liballeg.org/), [MGRX](http://www.fgrim.com/mgrx/) and [GRX](http://grx.gnu.de/).

It uses [libschrift](https://github.com/tomolt/libschrift) for loading and rendering TTFs and MGRX code to write out GRX, FNA and GRX-as-C-source output. The BDF parser was cobbled together by me w/o documentation just using some examples and the english Wikipedia page.

When converting fonts make sure the resuting filename ends with ".FNT", other extensions do not work with Allegro.

I might provide pre-compiled DOS, Linux and Win32 binaries later...

## Usage
```
Usage:
  fntconv [-cf] <TTF or BDF file> <size> <outfile>
```

## License
My code and everything MGRX is licensed under GPLv2 (see LICENSE).
Libschrift is licensed under ISC (see libschrift/LICENSE).

## Disclaimer
Because I have absolutely no clue about font rendering I took several shortcuts when calculation offsets, sizes and other stuff. So expect weird results and warning messages when using this!
