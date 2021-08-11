window.onload = main;

function main()
{
	var canvas = document.getElementById("canvas");
	var ctx = canvas.getContext('2d');
	var img = ctx.createImageData(canvas.width, canvas.height);
	var width = canvas.width;
	var height = canvas.height;
	var i = 0;
	for (var y = 0; y < height; y++) {
		for (var x = 0; x < width; x++) {
			var focal = {x: 0, y: 100, z: 0};
			var origin = {
				x: 2.0 * (x/width) - 1.0,
				y: -1.0,
				z: 1.0 - 2.0*(y/height)
			};
			var direction = normalize(sub(focal, origin));
			var t = taperedCylinderIntersection(origin, direction);
			var s = sphereIntersection(origin, direction);
			if (!t || (s && s < t))
				t = s;

			if (t) {
				t = Math.pow(t, 4.0) * 255;
				img.data[i++] = t;
				img.data[i++] = t;
				img.data[i++] = t;
				img.data[i++] = 255;
			} else {
				img.data[i++] = 50;
				img.data[i++] = 50;
				img.data[i++] = 50;
				img.data[i++] = 255;
			}
		}
	}
	ctx.putImageData(img, 0, 0);
}

function taperedCylinderIntersection(o, d)
{
	var r1 = 0.5;
	var r2 = 0.0;
	var l = 0.9;
	var h = (r1-r2) / (r1*l);
	var r = 1.0 / (r1*r1);
	var a = r*(d.x*d.x + d.y*d.y) - h*h*d.z*d.z;
	var b = r*(d.x*o.x + d.y*o.y) - h*d.z*(h*o.z - 1.0);
	var c = r*(o.x*o.x + o.y*o.y) - h*o.z*(h*o.z - 2.0) - 1.0;
	var roots = findRoots(a, 2.0*b, c);
	if (roots[0] === null)
		return null;
	else {
		var t;
		if (roots[0] < roots[1])
			t = roots[0];
		else
			t = roots[1];
		var z = d.z*t + o.z;
		if (z < 0.0 || z > l)
			return null;
		return t;
	}
}

function sphereIntersection(o, d)
{
	var r = 0.5;
	var a = d.x*d.x + d.y*d.y + d.z*d.z;
	var b = d.x*o.x + d.y*o.y + d.z*o.z;
	var c = o.x*o.x + o.y*o.y + o.z*o.z - r*r;
	var roots = findRoots(a, 2.0*b, c);
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
