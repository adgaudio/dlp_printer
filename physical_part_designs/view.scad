include <print_parts.scad>;
rod_to_bar_dist = 10;

module system() {
  translate([vat_r_o + vat_holder_width + 10, 0, vat_h]) {
  // Motor Mount
    rotate([0, vat_holder_angle, 0])translate([0, 0, motor_z])
      motor_mount();

  // Motor Gear
    rotate([0, 0, 180])rotate([0, -vat_holder_angle, 0])
    eccentric_roller_shaft();
  }

  // Vat
  vat();

  translate([-20 + -vat_r_i - vat_hinge_r_o - vat_hinge_x_offset, 0, 0]) {
  // Hinge
    2Dhinge();

  // Hinge holder
      translate([-20 + -2*vat_hinge_r_o + vat_hinge_x_offset, 0, (-vat_hinge_h-2)/2])
        hinge_mount();
  }

  // Build Platform
  build_platform();
}


module bar(xyz, center=true, color="gray") {
  cutout = 1;
  color(color)
    cube([xyz[0] - cutout,
          xyz[1] - cutout,
          xyz[2] - cutout],
         center=center);
}

module build_platform_extrusion() {
translate([0, 0, z_offset_build_platform + z_platform_mount + h_platform_shaft + 20]) {
    bar([20, 400, 40], center=true);

    translate([xy_extrusion*-1.5, 400/2 - xy_extrusion, 0]) {
      lead_screw_nut_mount();
      translate([0, 0, 500/2])lead_screw_mount();
    }
    // printed part that stabilizes build platform with smooth rod
    translate([-20-thickness, -1*(400/2 - 20), 0])
    rod_to_extrusion_stabilizing_mount(true);
    translate([thickness+20, 400/2 - 20, 0]) rotate ([0, 180, 0])
    rod_to_extrusion_stabilizing_mount(true, -1);
    rotate([0, 0, -90])translate([400/2-xy_extrusion, 0, xy_extrusion])
    rod_to_extrusion_roller();
}
}

module _frame() {
  difference() {
    bar([420, 420, 20], center=true);
    bar([420-40, 420-40, 22], center=true);
  }
}

module frame() {
  // threaded and smooth rod
  for (sign=[0, 1]) { // for each side of printer:
    rotate([0, 0, sign*180]) {
      translate([0, 420/2, 0]) {
        translate([-30, -20 - 10, -(420 - 380)/2 - 10 + motor_z])
          cylinder(r=(3/8.)*25.4, h=380, center=true);
      translate([0, 0, -420/2])rotate([0, 0, 180])
        motor_mount2();
        translate([(r_smooth_rod + r_lm8uu + 10), -20 - 10, 10]) {
          cylinder(r=8/2, h=420 + 40, center=true);
          for (sign2=[0, 1]) {rotate([180*sign2, 0, sign2*-90]) {
          translate([0, 0, 420/2 - 20/2 + sign2*20]) {
          // parts that tie smooth rod to frame
          rotate([0, 0, 180])rod_fastening_mount(-1);
          rotate([0, 0, 180])translate([-5, 0, 0])rod_fastening_mount(1);
          }
          }}
          
          /*cube([20, 10, 10], center=true);*/
        }
      }
    }
  }

  // bottom
  translate([0, 0, -420/2]) {
    _frame();
    // bottom center extrusion
    translate([-420/2+180/2 - 20, 20, 60])
    bar([180, 20, 20]);
  }
  // top
  translate([0, 0, 420/2]) {
    bar([20, 420, 20]);
    /*_frame();*/

    // top extrusion in middle (adjustable for motor mount)
    translate([0, 0, 20])
    bar([460, 20, 20], center=true);

    // small vertical piece of extrusion for motor mount
    translate([20, 0, -(420/2 +20)/2 + 10])
    bar([20, 20, 230], center=true);

    // small horizontal piece of extrusion for motor mount
    translate([420/4 + 15, 0, -420/2 + 40]) {
      bar([180, 20, 20], center=true);
      // rods for motor mount
      for (sign=[-1, 1]) {
      translate([-420/4, (r_rod_holder + 10)*sign, 0]) {
        rotate([0, 90, 0])
          cylinder(r=8/2, h=420/2);
      }
      translate([0, sign*(10+r_rod_holder), -r_rod_holder + thickness])rotate([0, 90, 0])
        rod_to_extrusion_stabilizing_mount();
      }
    }
  }
  // bottoms to clamp printer onto another base (optional)
  for (rmirror=[0, 1]) rotate([0, 0, 180*rmirror])
  for (mirror=[-1, 1])
  translate([420/3*mirror, 420/-2, -420/2-xy_extrusion/2])
    extrusion_clamp();
  // side - motor (rod) mount extrusion
  translate([420/2 + 10, 0, 0])
    bar([20, 20, 420+20], center=true);
  translate([420/2, 0, -420/2])rotate([0, 0, -90])
    extrusion_support();

  // side - vat mount extrusion
  translate([-420/2 - 10, 0, 0])
    bar([20, 20, 420+20], center=true);
  translate([-420/2, 0, -420/2])rotate([0, 0, 90])
    extrusion_support();
  // vat mount extrusion
  translate([-420/2 + 71.5, 0, 10])
    bar([20, 20, 420], center=true);
  translate([0, -420/2, -420/2])rotate([0, 0, 180])
    extrusion_support();
  // other sides
  for (sign = [-1, 1])
  translate([0, sign*(420/2 + 10), 0])
  bar([20, 20, 420+20], center=true);
  translate([0, 420/2, -420/2])
    extrusion_support();
  // circuit board attachments
  translate([-420/2-thickness/2, 60, -420/2+xy_extrusion/2+thickness/2])
    rotate([0, 90, 0]) {
      pcb_screw_attachment();
      translate([0, 2*xy_extrusion, 0]) pcb_screw_attachment();
    }
  // power supply switch
  translate([-420/3+30, 420/2-40, -420/2+xy_extrusion/2+thickness/2])
    rotate([0, 0, 90])
    power_rocker_switch_mount();
}
frame();
translate([0, 0, -420/2 + 100]) {
system();
build_platform_extrusion();
}
