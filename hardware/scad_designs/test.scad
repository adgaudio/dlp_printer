include <print_parts.scad>;


// test lense holder
module test_vat_lense_holder() {
donut(vat_r_i, vat_r_lense_lip, 1);
}

// test 608zz
module test_608zz() {
  donut(r_608zz+2, r_608zz, .5);
  donut(r_608zz_hole+2, r_608zz_hole, .5);
}

// test motor shaft
module test_eccentric_roller_shaft() { // make me
  difference() {
  eccentric_roller_shaft();
  cylinder(r=100, h=34.5, center=true);
}}

// test motor mount
module test_motor_mount_bolt_holes() { // make me
  difference() {
  rotate([0, -vat_holder_angle, 0])
  motor_mount();
  translate([0, 0, .5+25])
  cube([100, 100, 100], center=true);
  for (sign=[-1, 1])
  translate([sign*(motor_x + 25 - 7), 0, 0])
  cube([100, 100, 52], center=true);
  cube([motor_x - m3_bolt_radius*2 - 16, motor_y - 1, 52], center=true);
  cube([motor_x - 5, motor_y - m3_bolt_radius*2 - 16, 52], center=true);
  }
}

module test_motor_mount_center_circle() { // make me
donut(motor_r+2, motor_r, .5);
}
