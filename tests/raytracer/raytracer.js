// Name: Floris Creyf
// Date: June 2017
// A quick script for visually checking intersection tests.

window.onload = main;

function main()
{
	var canvas = document.getElementById("canvas");
	var ctx = canvas.getContext('2d');
	var img = ctx.createImageData(canvas.width, canvas.height);
	var width = canvas.width;
	var height = canvas.height;
	var i = 0;

	// variables for drawing checkerboard background
	var cx = 0;
	var ccx = false;
	var cy = 0;
	var ccy = false;

	for (var y = 0; y < height; y++) {
		if (++cy > 10) {
			ccy = !ccy;
			cy = 0;
		}
		cx = 0;
		ccx = ccy;

		for (var x = 0; x < width; x++) {
			if (++cx > 10) {
				ccx = !ccx;
				cx = 0;
			}

			var orig = {
				x: 2 * (x / width) - 1,
				z: 1 - 2 * (y / height),
				y: -1.0
			};

			var focal = {x: 0, y: 100, z: 0};
			var dir = normalize(sub(focal, orig));

			var t = intersection(orig, dir);

			if (t != null) {
				t = 255 - (t / 2) * 255;

				img.data[i  ] = t;
				img.data[i+1] = t;
				img.data[i+2] = t;
				img.data[i+3] = 255;
				i += 4;
			} else {
				if (ccx) {
					img.data[i  ] = 80;
					img.data[i+1] = 80;
					img.data[i+2] = 80;
					img.data[i+3] = 255;
				} else {
					img.data[i  ] = 50;
					img.data[i+1] = 50;
					img.data[i+2] = 50;
					img.data[i+3] = 255;
				}
				i += 4;
			}
		}
	}

	ctx.putImageData(img, 0, 0);
}

function intersection(orig, dir)
{
	return taperedCylinderIntersection(orig, dir);
}

// This uses the quadratic equation for a cone, which is transformed
// and then truncated to resemble a tapered cylinder.
// (a*x)^2 + (b*y)^2 - (c*z-1)^2 = 0
function taperedCylinderIntersection(orig, dir)
{
	var s = {x: 1/.2, y:1/.2, z:1};
	var p = {x: 0, y:0, z:-1};

	var a = s.x*s.x;
	var b = s.y*s.y;
	var c = -s.z*s.z;
	var g = 2*s.x*p.x;
	var h = 2*s.y*p.y;
	var i = -2*s.z*p.z;
	var j = p.x*p.x + p.y*p.y - p.z*p.z;

	var aq = 0, bq = 0, cq = 0;
	aq += a*dir.x*dir.x;
	aq += b*dir.y*dir.y;
	aq += c*dir.z*dir.z;
	bq += 2*a*dir.x*orig.x + g*dir.x;
	bq += 2*b*dir.y*orig.y + h*dir.y;
	bq += 2*c*dir.z*orig.z + i*dir.z;
	cq += a*orig.x*orig.x + g*orig.x;
	cq += b*orig.y*orig.y + h*orig.y;
	cq += c*orig.z*orig.z + i*orig.z;
	cq += j;

	var t = 0;
	var roots = findRoots(aq, bq, cq, roots);
	if (roots[0] != null) {
		if (roots[0] < roots[1])
			t = roots[0];
		else
			t = roots[1];
	} else
		t = 0;

	return t;
}

function coneIntersection(orig, dir)
{
	var a = dir.x*dir.x + dir.y*dir.y - dir.z*dir.z*0.25;
	var b = 2*dir.x*orig.x + 2*dir.y*orig.y - 2*dir.z*orig.z*0.25;
	var c = orig.x*orig.x + orig.y*orig.y - orig.z*orig.z*0.25;
	var roots = findRoots(a, b, c);

	if (roots[0] === null)
		return null;
	else {
		var t;
		if (roots[0] < roots[1])
			t = roots[0];
		else
			t = roots[1];

		var z = dir.z*t + orig.z;
		if (z < 0)
			return null;
		if (z > 0.5)
			return null;

		return t;
	}
}

function cylinderIntersection(orig, dir)
{
	var radius = 0.5;
	var a = dir.x*dir.x + dir.y*dir.y;
	var b = 2*dir.x*orig.x + 2*dir.y*orig.y;
	var c = orig.x*orig.x + orig.y*orig.y - radius*radius;
	var roots = findRoots(a, b, c);

	if (roots[0] === null)
		return null;
	else {
		var t;
		if (roots[0] < roots[1])
			t = roots[0];
		else
			t = roots[1];

		var z = dir.z*t + orig.z;
		if (z < -0.5)
			return null;
		if (z > 0.5)
			return null;

		return t;
	}
}

function sphereIntersection(orig, dir)
{
	var center = {x: 0.0, y: 0, z: 0.0};
	var radius = 0.4;

	var l = sub(orig, center);
	var a = dot(dir, dir);
	var b = 2 * dot(dir, l);
	var c = dot(l, l) - radius;
	var roots = findRoots(a, b, c);

	if (roots[0] === null)
		return null;
	else {
		var t;
		if (roots[0] < roots[1])
			t = roots[0];
		else
			t = roots[1];

		return t;
	}
}

function findRoots(a, b, c)
{
	var discr = b*b - 4.0*a*c;
	if (discr >= 0.0) {
		var sq = Math.sqrt(discr);
		var s1 = (-b + sq) / (2.0*a);
		var s2 = (-b - sq) / (2.0*a);
		return [s1, s2];
	} else {
		return [null, null];
	}
}

function dot(a, b)
{
	return a.x*b.x + a.y*b.y + a.z*b.z;
}

function sub(a, b)
{
	var c = {
		x: a.x - b.x,
		y: a.y - b.y,
		z: a.z - b.z
	};
	return c;
}

function add(a, b)
{
	var c = {
		x: a.x + b.x,
		y: a.y + b.y,
		z: a.z + b.z
	};
	return c;
}

function normalize(a)
{
	var l = Math.sqrt(a.x*a.x + a.y*a.y + a.z*a.z);
	var b = {
		x: a.x/l,
		y: a.y/l,
		z: a.z/l
	};
	return b;
}
