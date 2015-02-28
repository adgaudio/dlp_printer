include <vat.scad>;
include <motor.scad>;
include <build_platform.scad>;
include <circuit_board.scad>;

/*$fn=10;*/
$fn=100;

module print_motor_mount() { // make me
  motor_mount();
}
module print_motor_mount2() { // make me
  rotate([090, 0, 0])motor_mount2();
}
module print_eccentric_roller_shaft() { // make me
  eccentric_roller_shaft();
}
module print_vat() { // make me
  vat();
}
module print_2Dhinge() { // make me
  2Dhinge();
}
module print_hinge_mount() { // make me
  rotate([0, -90, 0])
  hinge_mount();
}

module print_build_platform() { // make me
  build_platform();
}

module print_rod_to_extrusion_stabilizing_mount() { // make me
  rotate([0, 90, 0])
  rod_to_extrusion_stabilizing_mount();
}
module print_rod_to_extrusion_stabilizing_mount_with_flap() { // make me
  rotate([0, 90, 0])
  rod_to_extrusion_stabilizing_mount(true);
}

module print_rod_to_extrusion_roller() {
  rod_to_extrusion_roller();
}

module print_rod_mount() { // make me
  rotate([0, 90, 0])
  rod_mount(0);
}

module print_rod_mount90() { // make me
  rotate([0, 90, 0])
  rod_mount(90);
}
module print_rod_fastening_mount_female() { // make me
  rotate([0, 90, 0])
  rod_fastening_mount(-1);
}
module print_rod_fastening_mount_male() { // make me
  rotate([0, 90, 0])
  rod_fastening_mount(1);
}

module print_lead_screw_nut_mount() { // make me
  lead_screw_nut_mount();
}
module print_lead_screw_mount() { // make me
  rotate([0, 90, 0])
  lead_screw_mount();
}

module print_extrusion_vertical_support() {
  rotate([90, 0, 0])
  extrusion_vertical_support();
}

module print_extrusion_support() { // make me
  extrusion_support();
}

module print_extrusion_corner_support() { // make me
  extrusion_corner_support(chirality=-1);
}
module print_extrusion_corner_support2() { // make me
  extrusion_corner_support(chirality=1);
}
module print_extrusion_T_support() { // make me
  rotate([90, 0, 0])
  extrusion_T_support();
}
module print_extrusion_clamp() { // make me
  rotate([90, 0, 0])
  extrusion_clamp();
}
module print_connector_bar() { // make me
  rotate([90, 0, 0])
  connector_bar();
}
module print_pcb_screw_attachment() { // make me
  rotate([0, -90, 0])
    pcb_screw_attachment();
}

module print_power_rocker_switch_mount() { // make me
  rotate([0, 180, 0])power_rocker_switch_mount();
}
