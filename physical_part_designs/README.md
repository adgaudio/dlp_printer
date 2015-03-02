This section explains how build the physical components of this printer.


### The OpenSCAD designs

`scad_designs` is a collection of OpenSCAD designs.  There are a few
main files.

- `view.scad` shows an accurate model of what the entire printer looks like
- `print_parts.scad` defines all the plastic parts in a readily
  printable format (rotated and angled appropriately) that is suitable
  for a normal FFM printer
- The other files define the actual plastic pieces that make up the
  printer.

To load one of the scad files in openscad, follow this example:

```
OPENSCADPATH=./external openscad scad_designs/view.scad
```
