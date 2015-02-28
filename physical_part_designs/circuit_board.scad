include <../lib/shape_primitives.scad>;
include <shared_vars.scad>;


pcb_ri = m3_bolt_radius;
pcb_ro = pcb_ri + 2;
pcb_separator = thickness + pcb_ro;

pcb_extrusion_dist = max(m3_nut_height+1, 3);  // extra distance between pcb and extrusion

r_power_rocker_switch_mount = 21.5 / 2;
r_o_power_rocker_switch_mount = r_power_rocker_switch_mount + 4;
w_arm_prsm = xy_extrusion / 2;
h_arm_stabilizer_prsm = xy_extrusion;


module _extrusion_attachment() {
  // attach to extrusion with m5 screw
  difference() {
    cube([xy_extrusion, xy_extrusion, thickness], center=true);
    cylinder(r=m5_bolt_radius, h=thickness+.1, center=true);
  }
}

module _pcb_attachment() {
  // attach to the pcb with m3 screw
  difference(){
    hull() {

      cube([thickness, xy_extrusion, thickness], center=true);
      for (sign=[-1,1]) {
        translate([sign*pcb_separator, 0, 0])
          cylinder(r=pcb_ro, h=thickness, center=true);
      }
    }
    for (sign=[-1,1]) {
      translate([sign*pcb_separator, 0, 0]) cylinder(r=pcb_ri, h=thickness+1, center=true);
      translate([sign*pcb_separator, 0, 0]) cylinder(r=pcb_ri, h=thickness+1, center=true);
    }
  }
}


module pcb_screw_attachment() {
  // for pcbs with screw holes in corners, use this to attach them to aluminum
  // extrusions
  translate([xy_extrusion/2 + thickness/2, 0, 0])
    _extrusion_attachment();
  cube([pcb_extrusion_dist+thickness/2, xy_extrusion, thickness], center=true);
  translate([-pcb_extrusion_dist, 0, 0]) rotate([0, 90, 0])
    _pcb_attachment();
}


module power_rocker_switch_mount() {
  x = r_o_power_rocker_switch_mount + w_arm_prsm;
  y = 0;
  z = 0;
  difference() {
    hull() {
      // rocker switch 
      cylinder(r=r_o_power_rocker_switch_mount, h=thickness, center=true);
      // screw hole
      translate([x + m5_bolt_radius+2,y,z])
        cylinder(r=m5_bolt_radius+2, h=thickness, center=true);
    }
    //cutout switch and screw hole
    cylinder(r=r_power_rocker_switch_mount, h=thickness + 1, center=true);
    translate([x + m5_bolt_radius+2,y,z])
      cylinder(r=m5_bolt_radius, h=thickness + 1, center=true);
  }

  // stabilizing section + optional screw hole
  z2 = xy_extrusion/-2 - m5_bolt_radius+2;
  translate([r_o_power_rocker_switch_mount + m5_bolt_radius+2 - thickness/2,
             0, 0])
    difference() {
      // stabilizer
      hull() {
        cube(  // y is a hack estimate
          [thickness, r_o_power_rocker_switch_mount+thickness/2, thickness],
          center=true);
        translate([0, 0, z2])rotate([0, 90, 0]) {
          cylinder(r=m5_bolt_radius+2, h=thickness, center=true);
        }
      }
      // cutout screw hole
      translate([0, 0, z2])rotate([0, 90, 0])
        cylinder(r=m5_bolt_radius, h=thickness+1, center=true);
    }
}
