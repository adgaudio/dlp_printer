include <../shared_vars.scad>;

motor_z = 2 + 55;
motor_x = 2 + 56.25;
motor_y = 2 + 56.25;
motor_r = (38+1)/2;
motor_mount_z = 8;
motor_mount_inset = 2.0 + 3;
motor_mount_bolt_size = m5_bolt_radius;
thickness_motor_mount = 5;

thickness = 3; // general thickness of things// TODO: used anywhere?

h_motor_shaft = 10;
thickness_motor_shaft = 5;
r_motor_shaft = 6.34/2 + .5;

xy_extrusion = 20.8; // extrusion cutout
r_smooth_rod = 8/2;  // TODO

eccentric_roller_rim_width = 3;
eccentric_roller_offset = thickness_motor_shaft + r_608zz + r_motor_shaft ;
eccentric_roller_r = eccentric_roller_rim_width + 
                     max(r_608zz, eccentric_roller_offset) + r_608zz;
eccentric_roller_r_o_shaft = thickness_motor_shaft + r_motor_shaft;
roller_h = 2*h_608zz + 1;
roller_nut_inset = .5*h_motor_shaft -1;


hinge_h = 2*h_608zz;
hinge_r_o = r_608zz + thickness;;
hinge_r_i = r_608zz;
hinge_thick = hinge_r_o - hinge_r_i;


vat_r_i = 54/2+2;
vat_r_o = vat_r_i + thickness;
vat_h = 30; // TODO
vat_r_lense_lip = vat_r_i - 7;
vat_h_lense_lip = 5;  // thickness of lip holding glass to vat
vat_z_lense_lip_offset = 7;
vat_z_holder = 10; // defines maximum possible z movement there can be when tilting vat
vat_holder_width = 30; // TODO - just seems right.  
vat_holder_angle = asin(vat_z_holder / vat_holder_width);
vat_hinge_r_o = hinge_r_o;
vat_hinge_h = 2*h_608zz;
vat_hinge_y_offset = 2/2*vat_hinge_h;
_y = (vat_hinge_y_offset- vat_hinge_h/2);
vat_hinge_x_offset = vat_r_o - sqrt(pow(vat_r_o, 2) - pow(_y, 2));  // via geometric translation & pythagorean theorum
vat_wing_nut_inset = vat_h - 25; // needs to fit my shoulder screw


// build platform vars
thickness_platform = 5;
r_platform = vat_r_i - eccentric_roller_offset/2; // made up number - just seems like a good value
y_offset_platform_mount = 10;
r_platform_shaft = 8;
h_platform_shaft = 20 + vat_h-vat_z_lense_lip_offset - vat_h_lense_lip;
z_platform_mount = 3;
xyz_platform_mount = [2*r_platform_shaft, 30, z_platform_mount];
dist_between_platform_mount_screws = 20;

z_offset_build_platform = vat_z_lense_lip_offset; // TODO: 50
y_offset_build_platform = vat_r_i - r_platform; // make vat disc eccentricly place

r_lm8uu = 16/2; // I'm actually using sdp-si press-fit bearings
r_rod_holder = r_lm8uu + thickness;
h_rod_holder = 20;
z_offset_rod_holder = 2*xy_extrusion; // vertical distance from center to top/bottom
x_offset_rod_holder = thickness; // adjust the angle made by this piece
length_rod_holder_flaps = 23;
rod_mount_length = r_rod_holder + 2*xy_extrusion;

r_lead_screw_sleeve_bearing = 12.72/2;
h_lead_screw_sleeve_bearing = 12.72;

r_lead_screw_nut_flange = 40/2;
r_i_lead_screw_nut_flange = 17.5/2;
r_screwhole_lead_screw_nut_flange = 14.27;

extrusion_support_length = 4*xy_extrusion;
extrusion_support_angle = 45;
