
module servo(ang, loc, name) {
  rotate([0, 0, ang]) {
    translate([-4, -1.0, 0]) cube([4.0, 2.0, 4.05]);
    rotate([0,90,0]) translate([-3.5, 0, 0.0]) cylinder(h=3,r=0.5);
    linear_extrude(.1) translate([-2.5, -2.2, 0]) color([1,0,0]) text(loc, size=1);
    linear_extrude(.1) translate([-2.5, 1.3, 0]) color([1,0,0]) text(name, size=1);
  }
}
$fn = 32;
radius = 7.5;
translate([-10, -15, -2]) color("green") 	   cube([30, 30, .4]);
translate([8.5, -15, -1.3]) color("blue")			   cube([11.5, 14.6, .2]);
linear_extrude(.1) translate([8, -15, 5.14]) color([1,0,0]) rotate([0, 0, 90]) {text("PCB", size=1);}
translate([8.5, -0, 1.2]) color("pink")			   cube([11.5, 14.6, .2]);
linear_extrude(.1) translate([7.5, 11, 0]) color([1,0,0]) rotate([0, 0, 90]) {text("Panel", size=1);}
rotate([0, 0, 154.217]) translate ([radius, 0, 0])   servo(25, "-6.75, 3.26", "Servo 0");
rotate([0, 0, -154.217]) translate ([radius, 0, 0])   servo(-25, "-6.75, -3.26", "Servo 1");
rotate([0, 0, -85.7832]) translate ([radius, 0, 0])   servo(25, "0.551, -7.48", "Servo 2");
rotate([0, 0, -34.2169]) translate ([radius, 0, 0])   servo(-25, "6.2, -4.22", "Servo 3");
rotate([0, 0, 34.2169]) translate ([radius, 0, 0])   servo(25, "6.2, 4.22", "Servo 4");
rotate([0, 0, 85.7832]) translate ([radius, 0, 0])   servo(-25, "0.551, 7.48", "Servo 5");
