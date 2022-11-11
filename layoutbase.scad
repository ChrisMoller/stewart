
module servo(ang, loc, name) {
  rotate([0, 0, ang]) {
// body
    translate([-4, -1.0, 0]) cube([4.0, 2.0, 4.05]);

// shaft
    rotate([0,90,0]) translate([-3.5, 0, 0.0]) cylinder(h=3,r=0.5);

// location label
    linear_extrude(.1) translate([-2.5, -2.2, 0])
       color([1,0,0]) text(loc, size=1);

// name
    linear_extrude(.1) translate([-2.5, 1.3, 0])
      color([1,0,0]) text(name, size=1);
  }
}

module panel() {
       translate([0, 0, 2.5]) color("pink") cube([18.5, 14.6, .2]);
       linear_extrude(.1) translate([0, 11, 0]) color([1,0,0])
             rotate([0, 0, 90]) {text("Panel", size=1);}
       translate([0.5, 0.5, 0]) cylinder(h=2.5,r=0.1);
       translate([18, 0.5, 0]) cylinder(h=2.5,r=0.1);
       translate([18, 14, 0]) cylinder(h=2.5,r=0.1);
       translate([0.5, 14, 0]) cylinder(h=2.5,r=0.1);
}

$fn = 32;
radius = 7.5;

// base
translate([-10, -15, -.5]) color("green") cube([40, 30, .4]);

// PCB
translate([18, -15, 0.2]) color("blue") cube([11.5, 14.6, .2]);
linear_extrude(.1) translate([18, -15, 5.14]) color([1,0,0])
    rotate([0, 0, 90]) {text("PCB", size=1);}

// panel
//translate([11, -0, 2.5]) color("pink") cube([18.5, 14.6, .2]);
//linear_extrude(.1) translate([11, 11, 0]) color([1,0,0])
//   rotate([0, 0, 90]) {text("Panel", size=1);}

translate ([11, 0, 0]) panel();

// servvos
rotate([0, 0, 154.217]) translate ([radius, 0, 0])
   servo(25, "-6.75, 3.26", "Servo 0");
rotate([0, 0, -154.217]) translate ([radius, 0, 0])
   servo(-25, "-6.75, -3.26", "Servo 1");
rotate([0, 0, -85.7832]) translate ([radius, 0, 0])
   servo(25, "0.551, -7.48", "Servo 2");
rotate([0, 0, -34.2169]) translate ([radius, 0, 0])
   servo(-25, "6.2, -4.22", "Servo 3");
rotate([0, 0, 34.2169]) translate ([radius, 0, 0])
   servo(25, "6.2, 4.22", "Servo 4");
rotate([0, 0, 85.7832]) translate ([radius, 0, 0])
   servo(-25, "0.551, 7.48", "Servo 5");
