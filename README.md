# GrxFntConv
This tool (aka "dirty hack") can be used to convert TTF fonts to the GRX format used by [DOjS](https://github.com/SuperIlu/DOjS), [Allegro](https://liballeg.org/), [MGRX](http://www.fgrim.com/mgrx/) and [GRX](http://grx.gnu.de/).

It uses [libschrift](https://github.com/tomolt/libschrift) for loading and rendering TTFs and MGRX code to write out GRX, FNA and GRX-as-C-source output.

## License
My code and everything MGRX is licensed under GPLv2 (see LICENSE).
Libschrift is licensed under ISC (see libschrift/LICENSE).

## Disclaimer
Because I have absolutely no clue about font rendering I took several shortcuts when calculation offsets, sizes and other stuff. So expect weird results and warning messages when using this!
