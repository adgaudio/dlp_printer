/* Stuff to help build the printer
 */

module extrusion_drill_guide(depth=10, width=20) {
  tot_length = max(40, depth+15);
  difference() {
    cube([tot_length, width+20, 10]);
    translate ([10, 10, 5]) {
      cube([tot_length, width, 6]);
      translate ([depth, width/2, 0]) {
        cylinder(r=r_bolt, h=40, center=true, $fn=15);
      }
    }
  }
}
//extrusion_drill_guide(); // make me
