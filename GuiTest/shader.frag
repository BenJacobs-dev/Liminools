#version 450



////////////////////////////////////////////////////////////////
//
//                           HG_SDF
//
//     GLSL LIBRARY FOR BUILDING SIGNED DISTANCE BOUNDS
//
//     version 2021-07-28
//
//     Check https://mercury.sexy/hg_sdf for updates
//     and usage examples. Send feedback to spheretracing@mercury.sexy.
//
//     Brought to you by MERCURY https://mercury.sexy/
//
//
//
// Released dual-licensed under
//   Creative Commons Attribution-NonCommercial (CC BY-NC)
// or
//   MIT License
// at your choice.
//
// SPDX-License-Identifier: MIT OR CC-BY-NC-4.0
//
// /////
//
// CC-BY-NC-4.0
// https://creativecommons.org/licenses/by-nc/4.0/legalcode
// https://creativecommons.org/licenses/by-nc/4.0/
//
// /////
//
// MIT License
//
// Copyright (c) 2011-2021 Mercury Demogroup
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// /////
//
////////////////////////////////////////////////////////////////
//
// How to use this:
//
// 1. Build some system to #include glsl files in each other.
//   Include this one at the very start. Or just paste everywhere.
// 2. Build a sphere tracer. See those papers:
//   * "Sphere Tracing" https://link.springer.com/article/10.1007%2Fs003710050084
//   * "Enhanced Sphere Tracing" http://diglib.eg.org/handle/10.2312/stag.20141233.001-008
//   * "Improved Ray Casting of Procedural Distance Bounds" https://www.bibsonomy.org/bibtex/258e85442234c3ace18ba4d89de94e57d
//   The Raymnarching Toolbox Thread on pouet can be helpful as well
//   http://www.pouet.net/topic.php?which=7931&page=1
//   and contains links to many more resources.
// 3. Use the tools in this library to build your distance bound f().
// 4. ???
// 5. Win a compo.
// 
// (6. Buy us a beer or a good vodka or something, if you like.)
//
////////////////////////////////////////////////////////////////
//
// Table of Contents:
//
// * Helper functions and macros
// * Collection of some primitive objects
// * Domain Manipulation operators
// * Object combination operators
//
////////////////////////////////////////////////////////////////
//
// Why use this?
//
// The point of this lib is that everything is structured according
// to patterns that we ended up using when building geometry.
// It makes it more easy to write code that is reusable and that somebody
// else can actually understand. Especially code on Shadertoy (which seems
// to be what everybody else is looking at for "inspiration") tends to be
// really ugly. So we were forced to do something about the situation and
// release this lib ;)
//
// Everything in here can probably be done in some better way.
// Please experiment. We'd love some feedback, especially if you
// use it in a scene production.
//
// The main patterns for building geometry this way are:
// * Stay Lipschitz continuous. That means: don't have any distance
//   gradient larger than 1. Try to be as close to 1 as possible -
//   Distances are euclidean distances, don't fudge around.
//   Underestimating distances will happen. That's why calling
//   it a "distance bound" is more correct. Don't ever multiply
//   distances by some value to "fix" a Lipschitz continuity
//   violation. The invariant is: each fSomething() function returns
//   a correct distance bound.
// * Use very few primitives and combine them as building blocks
//   using combine opertors that preserve the invariant.
// * Multiply objects by repeating the domain (space).
//   If you are using a loop inside your distance function, you are
//   probably doing it wrong (or you are building boring fractals).
// * At right-angle intersections between objects, build a new local
//   coordinate system from the two distances to combine them in
//   interesting ways.
// * As usual, there are always times when it is best to not follow
//   specific patterns.
//
////////////////////////////////////////////////////////////////
//
// FAQ
//
// Q: Why is there no sphere tracing code in this lib?
// A: Because our system is way too complex and always changing.
//    This is the constant part. Also we'd like everyone to
//    explore for themselves.
//
// Q: This does not work when I paste it into Shadertoy!!!!
// A: Yes. It is GLSL, not GLSL ES. We like real OpenGL
//    because it has way more features and is more likely
//    to work compared to browser-based WebGL. We recommend
//    you consider using OpenGL for your productions. Most
//    of this can be ported easily though.
//
// Q: How do I material?
// A: We recommend something like this:
//    Write a material ID, the distance and the local coordinate
//    p into some global variables whenever an object's distance is
//    smaller than the stored distance. Then, at the end, evaluate
//    the material to get color, roughness, etc., and do the shading.
//
// Q: I found an error. Or I made some function that would fit in
//    in this lib. Or I have some suggestion.
// A: Awesome! Drop us a mail at spheretracing@mercury.sexy.
//
// Q: Why is this not on github?
// A: Because we were too lazy. If we get bugged about it enough,
//    we'll do it.
//
// Q: Your license sucks for me.
// A: Oh. What should we change it to?
//
// Q: I have trouble understanding what is going on with my distances.
// A: Some visualization of the distance field helps. Try drawing a
//    plane that you can sweep through your scene with some color
//    representation of the distance field at each point and/or iso
//    lines at regular intervals. Visualizing the length of the
//    gradient (or better: how much it deviates from being equal to 1)
//    is immensely helpful for understanding which parts of the
//    distance field are broken.
//
////////////////////////////////////////////////////////////////






////////////////////////////////////////////////////////////////
//
//             HELPER FUNCTIONS/MACROS
//
////////////////////////////////////////////////////////////////

#define PI 3.14159265
#define TAU (2*PI)
#define PHI (sqrt(5)*0.5 + 0.5)

// Clamp to [0,1] - this operation is free under certain circumstances.
// For further information see
// http://www.humus.name/Articles/Persson_LowLevelThinking.pdf and
// http://www.humus.name/Articles/Persson_LowlevelShaderOptimization.pdf
#define saturate(x) clamp(x, 0, 1)

// Sign function that doesn't return 0
float sgn(float x) {
	return (x<0.0)?-1:1;
}

vec2 sgn(vec2 v) {
	return vec2((v.x<0)?-1:1, (v.y<0)?-1:1);
}

float square (float x) {
	return x*x;
}

vec2 square (vec2 x) {
	return x*x;
}

vec3 square (vec3 x) {
	return x*x;
}

float lengthSqr(vec3 x) {
	return dot(x, x);
}


// Maximum/minumum elements of a vector
float vmax(vec2 v) {
	return max(v.x, v.y);
}

float vmax(vec3 v) {
	return max(max(v.x, v.y), v.z);
}

float vmax(vec4 v) {
	return max(max(v.x, v.y), max(v.z, v.w));
}

float vmin(vec2 v) {
	return min(v.x, v.y);
}

float vmin(vec3 v) {
	return min(min(v.x, v.y), v.z);
}

float vmin(vec4 v) {
	return min(min(v.x, v.y), min(v.z, v.w));
}




////////////////////////////////////////////////////////////////
//
//             PRIMITIVE DISTANCE FUNCTIONS
//
////////////////////////////////////////////////////////////////
//
// Conventions:
//
// Everything that is a distance function is called fSomething.
// The first argument is always a point in 2 or 3-space called <p>.
// Unless otherwise noted, (if the object has an intrinsic "up"
// side or direction) the y axis is "up" and the object is
// centered at the origin.
//
////////////////////////////////////////////////////////////////

float fSphere(vec3 p, float r) {
	return length(p) - r;
}

// Plane with normal n (n is normalized) at some distance from the origin
float fPlane(vec3 p, vec3 n, float distanceFromOrigin) {
	return dot(p, n) + distanceFromOrigin;
}

// Cheap Box: distance to corners is overestimated
float fBoxCheap(vec3 p, vec3 b) { //cheap box
	return vmax(abs(p) - b);
}

// Box: correct distance to corners
float fBox(vec3 p, vec3 b) {
	vec3 d = abs(p) - b;
	return length(max(d, vec3(0))) + vmax(min(d, vec3(0)));
}

// Same as above, but in two dimensions (an endless box)
float fBox2Cheap(vec2 p, vec2 b) {
	return vmax(abs(p)-b);
}

float fBox2(vec2 p, vec2 b) {
	vec2 d = abs(p) - b;
	return length(max(d, vec2(0))) + vmax(min(d, vec2(0)));
}


// Endless "corner"
float fCorner (vec2 p) {
	return length(max(p, vec2(0))) + vmax(min(p, vec2(0)));
}

// Blobby ball object. You've probably seen it somewhere. This is not a correct distance bound, beware.
float fBlob(vec3 p) {
	p = abs(p);
	if (p.x < max(p.y, p.z)) p = p.yzx;
	if (p.x < max(p.y, p.z)) p = p.yzx;
	float b = max(max(max(
		dot(p, normalize(vec3(1, 1, 1))),
		dot(p.xz, normalize(vec2(PHI+1, 1)))),
		dot(p.yx, normalize(vec2(1, PHI)))),
		dot(p.xz, normalize(vec2(1, PHI))));
	float l = length(p);
	return l - 1.5 - 0.2 * (1.5 / 2)* cos(min(sqrt(1.01 - b / l)*(PI / 0.25), PI));
}

// Cylinder standing upright on the xz plane
float fCylinder(vec3 p, float r, float height) {
	float d = length(p.xz) - r;
	d = max(d, abs(p.y) - height);
	return d;
}

// Capsule: A Cylinder with round caps on both sides
float fCapsule(vec3 p, float r, float c) {
	return mix(length(p.xz) - r, length(vec3(p.x, abs(p.y) - c, p.z)) - r, step(c, abs(p.y)));
}

// Distance to line segment between <a> and <b>, used for fCapsule() version 2below
float fLineSegment(vec3 p, vec3 a, vec3 b) {
	vec3 ab = b - a;
	float t = saturate(dot(p - a, ab) / dot(ab, ab));
	return length((ab*t + a) - p);
}

// Capsule version 2: between two end points <a> and <b> with radius r 
float fCapsule(vec3 p, vec3 a, vec3 b, float r) {
	return fLineSegment(p, a, b) - r;
}

// Torus in the XZ-plane
float fTorus(vec3 p, float smallRadius, float largeRadius) {
	return length(vec2(length(p.xz) - largeRadius, p.y)) - smallRadius;
}

// A circle line. Can also be used to make a torus by subtracting the smaller radius of the torus.
float fCircle(vec3 p, float r) {
	float l = length(p.xz) - r;
	return length(vec2(p.y, l));
}

// A circular disc with no thickness (i.e. a cylinder with no height).
// Subtract some value to make a flat disc with rounded edge.
float fDisc(vec3 p, float r) {
	float l = length(p.xz) - r;
	return l < 0 ? abs(p.y) : length(vec2(p.y, l));
}

// Hexagonal prism, circumcircle variant
float fHexagonCircumcircle(vec3 p, vec2 h) {
	vec3 q = abs(p);
	return max(q.y - h.y, max(q.x*sqrt(3)*0.5 + q.z*0.5, q.z) - h.x);
	//this is mathematically equivalent to this line, but less efficient:
	//return max(q.y - h.y, max(dot(vec2(cos(PI/3), sin(PI/3)), q.zx), q.z) - h.x);
}

// Hexagonal prism, incircle variant
float fHexagonIncircle(vec3 p, vec2 h) {
	return fHexagonCircumcircle(p, vec2(h.x*sqrt(3)*0.5, h.y));
}

// Cone with correct distances to tip and base circle. Y is up, 0 is in the middle of the base.
float fCone(vec3 p, float radius, float height) {
	vec2 q = vec2(length(p.xz), p.y);
	vec2 tip = q - vec2(0, height);
	vec2 mantleDir = normalize(vec2(height, radius));
	float mantle = dot(tip, mantleDir);
	float d = max(mantle, -q.y);
	float projected = dot(tip, vec2(mantleDir.y, -mantleDir.x));
	
	// distance to tip
	if ((q.y > height) && (projected < 0)) {
		d = max(d, length(tip));
	}
	
	// distance to base ring
	if ((q.x > radius) && (projected > length(vec2(height, radius)))) {
		d = max(d, length(q - vec2(radius, 0)));
	}
	return d;
}

//
// "Generalized Distance Functions" by Akleman and Chen.
// see the Paper at https://www.viz.tamu.edu/faculty/ergun/research/implicitmodeling/papers/sm99.pdf
//
// This set of constants is used to construct a large variety of geometric primitives.
// Indices are shifted by 1 compared to the paper because we start counting at Zero.
// Some of those are slow whenever a driver decides to not unroll the loop,
// which seems to happen for fIcosahedron und fTruncatedIcosahedron on nvidia 350.12 at least.
// Specialized implementations can well be faster in all cases.
//

const vec3 GDFVectors[19] = vec3[](
	normalize(vec3(1, 0, 0)),
	normalize(vec3(0, 1, 0)),
	normalize(vec3(0, 0, 1)),

	normalize(vec3(1, 1, 1 )),
	normalize(vec3(-1, 1, 1)),
	normalize(vec3(1, -1, 1)),
	normalize(vec3(1, 1, -1)),

	normalize(vec3(0, 1, PHI+1)),
	normalize(vec3(0, -1, PHI+1)),
	normalize(vec3(PHI+1, 0, 1)),
	normalize(vec3(-PHI-1, 0, 1)),
	normalize(vec3(1, PHI+1, 0)),
	normalize(vec3(-1, PHI+1, 0)),

	normalize(vec3(0, PHI, 1)),
	normalize(vec3(0, -PHI, 1)),
	normalize(vec3(1, 0, PHI)),
	normalize(vec3(-1, 0, PHI)),
	normalize(vec3(PHI, 1, 0)),
	normalize(vec3(-PHI, 1, 0))
);

// Version with variable exponent.
// This is slow and does not produce correct distances, but allows for bulging of objects.
float fGDF(vec3 p, float r, float e, int begin, int end) {
	float d = 0;
	for (int i = begin; i <= end; ++i)
		d += pow(abs(dot(p, GDFVectors[i])), e);
	return pow(d, 1/e) - r;
}

// Version with without exponent, creates objects with sharp edges and flat faces
float fGDF(vec3 p, float r, int begin, int end) {
	float d = 0;
	for (int i = begin; i <= end; ++i)
		d = max(d, abs(dot(p, GDFVectors[i])));
	return d - r;
}

// Primitives follow:

float fOctahedron(vec3 p, float r, float e) {
	return fGDF(p, r, e, 3, 6);
}

float fDodecahedron(vec3 p, float r, float e) {
	return fGDF(p, r, e, 13, 18);
}

float fIcosahedron(vec3 p, float r, float e) {
	return fGDF(p, r, e, 3, 12);
}

float fTruncatedOctahedron(vec3 p, float r, float e) {
	return fGDF(p, r, e, 0, 6);
}

float fTruncatedIcosahedron(vec3 p, float r, float e) {
	return fGDF(p, r, e, 3, 18);
}

float fOctahedron(vec3 p, float r) {
	return fGDF(p, r, 3, 6);
}

float fDodecahedron(vec3 p, float r) {
	return fGDF(p, r, 13, 18);
}

float fIcosahedron(vec3 p, float r) {
	return fGDF(p, r, 3, 12);
}

float fTruncatedOctahedron(vec3 p, float r) {
	return fGDF(p, r, 0, 6);
}

float fTruncatedIcosahedron(vec3 p, float r) {
	return fGDF(p, r, 3, 18);
}


////////////////////////////////////////////////////////////////
//
//                DOMAIN MANIPULATION OPERATORS
//
////////////////////////////////////////////////////////////////
//
// Conventions:
//
// Everything that modifies the domain is named pSomething.
//
// Many operate only on a subset of the three dimensions. For those,
// you must choose the dimensions that you want manipulated
// by supplying e.g. <p.x> or <p.zx>
//
// <inout p> is always the first argument and modified in place.
//
// Many of the operators partition space into cells. An identifier
// or cell index is returned, if possible. This return value is
// intended to be optionally used e.g. as a random seed to change
// parameters of the distance functions inside the cells.
//
// Unless stated otherwise, for cell index 0, <p> is unchanged and cells
// are centered on the origin so objects don't have to be moved to fit.
//
//
////////////////////////////////////////////////////////////////



// Rotate around a coordinate axis (i.e. in a plane perpendicular to that axis) by angle <a>.
// Read like this: R(p.xz, a) rotates "x towards z".
// This is fast if <a> is a compile-time constant and slower (but still practical) if not.
void pR(inout vec2 p, float a) {
	p = cos(a)*p + sin(a)*vec2(p.y, -p.x);
}

// Shortcut for 45-degrees rotation
void pR45(inout vec2 p) {
	p = (p + vec2(p.y, -p.x))*sqrt(0.5);
}

// Repeat space along one axis. Use like this to repeat along the x axis:
// <float cell = pMod1(p.x,5);> - using the return value is optional.
float pMod1(inout float p, float size) {
	float halfsize = size*0.5;
	float c = floor((p + halfsize)/size);
	p = mod(p + halfsize, size) - halfsize;
	return c;
}

// Same, but mirror every second cell so they match at the boundaries
float pModMirror1(inout float p, float size) {
	float halfsize = size*0.5;
	float c = floor((p + halfsize)/size);
	p = mod(p + halfsize,size) - halfsize;
	p *= mod(c, 2.0)*2 - 1;
	return c;
}

// Repeat the domain only in positive direction. Everything in the negative half-space is unchanged.
float pModSingle1(inout float p, float size) {
	float halfsize = size*0.5;
	float c = floor((p + halfsize)/size);
	if (p >= 0)
		p = mod(p + halfsize, size) - halfsize;
	return c;
}

// Repeat only a few times: from indices <start> to <stop> (similar to above, but more flexible)
float pModInterval1(inout float p, float size, float start, float stop) {
	float halfsize = size*0.5;
	float c = floor((p + halfsize)/size);
	p = mod(p+halfsize, size) - halfsize;
	if (c > stop) { //yes, this might not be the best thing numerically.
		p += size*(c - stop);
		c = stop;
	}
	if (c <start) {
		p += size*(c - start);
		c = start;
	}
	return c;
}


// Repeat around the origin by a fixed angle.
// For easier use, num of repetitions is use to specify the angle.
float pModPolar(inout vec2 p, float repetitions) {
	float angle = 2*PI/repetitions;
	float a = atan(p.y, p.x) + angle/2.;
	float r = length(p);
	float c = floor(a/angle);
	a = mod(a,angle) - angle/2.;
	p = vec2(cos(a), sin(a))*r;
	// For an odd number of repetitions, fix cell index of the cell in -x direction
	// (cell index would be e.g. -5 and 5 in the two halves of the cell):
	if (abs(c) >= (repetitions/2)) c = abs(c);
	return c;
}

// Repeat in two dimensions
vec2 pMod2(inout vec2 p, vec2 size) {
	vec2 c = floor((p + size*0.5)/size);
	p = mod(p + size*0.5,size) - size*0.5;
	return c;
}

// Same, but mirror every second cell so all boundaries match
vec2 pModMirror2(inout vec2 p, vec2 size) {
	vec2 halfsize = size*0.5;
	vec2 c = floor((p + halfsize)/size);
	p = mod(p + halfsize, size) - halfsize;
	p *= mod(c,vec2(2))*2 - vec2(1);
	return c;
}

// Same, but mirror every second cell at the diagonal as well
vec2 pModGrid2(inout vec2 p, vec2 size) {
	vec2 c = floor((p + size*0.5)/size);
	p = mod(p + size*0.5, size) - size*0.5;
	p *= mod(c,vec2(2))*2 - vec2(1);
	p -= size/2;
	if (p.x > p.y) p.xy = p.yx;
	return floor(c/2);
}

// Repeat in three dimensions
vec3 pMod3(inout vec3 p, vec3 size) {
	vec3 c = floor((p + size*0.5)/size);
	p = mod(p + size*0.5, size) - size*0.5;
	return c;
}

// Mirror at an axis-aligned plane which is at a specified distance <dist> from the origin.
float pMirror (inout float p, float dist) {
	float s = sgn(p);
	p = abs(p)-dist;
	return s;
}

// Mirror in both dimensions and at the diagonal, yielding one eighth of the space.
// translate by dist before mirroring.
vec2 pMirrorOctant (inout vec2 p, vec2 dist) {
	vec2 s = sgn(p);
	pMirror(p.x, dist.x);
	pMirror(p.y, dist.y);
	if (p.y > p.x)
		p.xy = p.yx;
	return s;
}

// Reflect space at a plane
float pReflect(inout vec3 p, vec3 planeNormal, float offset) {
	float t = dot(p, planeNormal)+offset;
	if (t < 0) {
		p = p - (2*t)*planeNormal;
	}
	return sgn(t);
}


////////////////////////////////////////////////////////////////
//
//             OBJECT COMBINATION OPERATORS
//
////////////////////////////////////////////////////////////////
//
// We usually need the following boolean operators to combine two objects:
// Union: OR(a,b)
// Intersection: AND(a,b)
// Difference: AND(a,!b)
// (a and b being the distances to the objects).
//
// The trivial implementations are min(a,b) for union, max(a,b) for intersection
// and max(a,-b) for difference. To combine objects in more interesting ways to
// produce rounded edges, chamfers, stairs, etc. instead of plain sharp edges we
// can use combination operators. It is common to use some kind of "smooth minimum"
// instead of min(), but we don't like that because it does not preserve Lipschitz
// continuity in many cases.
//
// Naming convention: since they return a distance, they are called fOpSomething.
// The different flavours usually implement all the boolean operators above
// and are called fOpUnionRound, fOpIntersectionRound, etc.
//
// The basic idea: Assume the object surfaces intersect at a right angle. The two
// distances <a> and <b> constitute a new local two-dimensional coordinate system
// with the actual intersection as the origin. In this coordinate system, we can
// evaluate any 2D distance function we want in order to shape the edge.
//
// The operators below are just those that we found useful or interesting and should
// be seen as examples. There are infinitely more possible operators.
//
// They are designed to actually produce correct distances or distance bounds, unlike
// popular "smooth minimum" operators, on the condition that the gradients of the two
// SDFs are at right angles. When they are off by more than 30 degrees or so, the
// Lipschitz condition will no longer hold (i.e. you might get artifacts). The worst
// case is parallel surfaces that are close to each other.
//
// Most have a float argument <r> to specify the radius of the feature they represent.
// This should be much smaller than the object size.
//
// Some of them have checks like "if ((-a < r) && (-b < r))" that restrict
// their influence (and computation cost) to a certain area. You might
// want to lift that restriction or enforce it. We have left it as comments
// in some cases.
//
// usage example:
//
// float fTwoBoxes(vec3 p) {
//   float box0 = fBox(p, vec3(1));
//   float box1 = fBox(p-vec3(1), vec3(1));
//   return fOpUnionChamfer(box0, box1, 0.2);
// }
//
////////////////////////////////////////////////////////////////


// The "Chamfer" flavour makes a 45-degree chamfered edge (the diagonal of a square of size <r>):
float fOpUnionChamfer(float a, float b, float r) {
	return min(min(a, b), (a - r + b)*sqrt(0.5));
}

// Intersection has to deal with what is normally the inside of the resulting object
// when using union, which we normally don't care about too much. Thus, intersection
// implementations sometimes differ from union implementations.
float fOpIntersectionChamfer(float a, float b, float r) {
	return max(max(a, b), (a + r + b)*sqrt(0.5));
}

// Difference can be built from Intersection or Union:
float fOpDifferenceChamfer (float a, float b, float r) {
	return fOpIntersectionChamfer(a, -b, r);
}

// The "Round" variant uses a quarter-circle to join the two objects smoothly:
float fOpUnionRound(float a, float b, float r) {
	vec2 u = max(vec2(r - a,r - b), vec2(0));
	return max(r, min (a, b)) - length(u);
}

float fOpIntersectionRound(float a, float b, float r) {
	vec2 u = max(vec2(r + a,r + b), vec2(0));
	return min(-r, max (a, b)) + length(u);
}

float fOpDifferenceRound (float a, float b, float r) {
	return fOpIntersectionRound(a, -b, r);
}


// The "Columns" flavour makes n-1 circular columns at a 45 degree angle:
float fOpUnionColumns(float a, float b, float r, float n) {
	if ((a < r) && (b < r)) {
		vec2 p = vec2(a, b);
		float columnradius = r*sqrt(2)/((n-1)*2+sqrt(2));
		pR45(p);
		p.x -= sqrt(2)/2*r;
		p.x += columnradius*sqrt(2);
		if (mod(n,2) == 1) {
			p.y += columnradius;
		}
		// At this point, we have turned 45 degrees and moved at a point on the
		// diagonal that we want to place the columns on.
		// Now, repeat the domain along this direction and place a circle.
		pMod1(p.y, columnradius*2);
		float result = length(p) - columnradius;
		result = min(result, p.x);
		result = min(result, a);
		return min(result, b);
	} else {
		return min(a, b);
	}
}

float fOpDifferenceColumns(float a, float b, float r, float n) {
	a = -a;
	float m = min(a, b);
	//avoid the expensive computation where not needed (produces discontinuity though)
	if ((a < r) && (b < r)) {
		vec2 p = vec2(a, b);
		float columnradius = r*sqrt(2)/n/2.0;
		columnradius = r*sqrt(2)/((n-1)*2+sqrt(2));

		pR45(p);
		p.y += columnradius;
		p.x -= sqrt(2)/2*r;
		p.x += -columnradius*sqrt(2)/2;

		if (mod(n,2) == 1) {
			p.y += columnradius;
		}
		pMod1(p.y,columnradius*2);

		float result = -length(p) + columnradius;
		result = max(result, p.x);
		result = min(result, a);
		return -min(result, b);
	} else {
		return -m;
	}
}

float fOpIntersectionColumns(float a, float b, float r, float n) {
	return fOpDifferenceColumns(a,-b,r, n);
}

// The "Stairs" flavour produces n-1 steps of a staircase:
// much less stupid version by paniq
float fOpUnionStairs(float a, float b, float r, float n) {
	float s = r/n;
	float u = b-r;
	return min(min(a,b), 0.5 * (u + a + abs ((mod (u - a + s, 2 * s)) - s)));
}

// We can just call Union since stairs are symmetric.
float fOpIntersectionStairs(float a, float b, float r, float n) {
	return -fOpUnionStairs(-a, -b, r, n);
}

float fOpDifferenceStairs(float a, float b, float r, float n) {
	return -fOpUnionStairs(-a, b, r, n);
}


// Similar to fOpUnionRound, but more lipschitz-y at acute angles
// (and less so at 90 degrees). Useful when fudging around too much
// by MediaMolecule, from Alex Evans' siggraph slides
float fOpUnionSoft(float a, float b, float r) {
	float e = max(r - abs(a - b), 0);
	return min(a, b) - e*e*0.25/r;
}


// produces a cylindical pipe that runs along the intersection.
// No objects remain, only the pipe. This is not a boolean operator.
float fOpPipe(float a, float b, float r) {
	return length(vec2(a, b)) - r;
}

// first object gets a v-shaped engraving where it intersect the second
float fOpEngrave(float a, float b, float r) {
	return max(a, (a + r - abs(b))*sqrt(0.5));
}

// first object gets a capenter-style groove cut out
float fOpGroove(float a, float b, float ra, float rb) {
	return max(a, min(a + ra, rb - abs(b)));
}

// first object gets a capenter-style tongue attached
float fOpTongue(float a, float b, float ra, float rb) {
	return min(a, max(a - ra, abs(b) - rb));
}



































































































/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Rendering code
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const int OBJECT_COUNT_MAX = 32;
const int IMAGE_COUNT_MAX = 16;

layout(binding = 4) uniform sampler2D texSampler[IMAGE_COUNT_MAX];

struct WorldObject {
    vec3 center;
    vec3 size;
    vec3 color;
    vec4 data1;
    vec4 data2;
    vec4 data3;
    vec4 data4;
    int textureIndex;
    int int2;
    int int3;
    int int4;
    int int5;
    int int6;
    int int7;
    int int8;
    int type;
    int is_negated;
    float shadow_blur;
	float shadow_intensity;
    float reflectivity;
    float transparency;
    float diffuse_intensity;
    float refractive_index;
};

struct WorldObjectCombineModifier {
    vec4 data1;
    vec4 data2;
    vec4 data3;
    vec4 data4;
    int int1;
    int int2;
    int int3;
    int int4;
    int int5;
    int int6;
    int int7;
    int int8;
    int index1;
    int index2;
    int type;
    int index1Type;
};

struct WorldObjectDomainModifier {
    vec4 data1;
    vec4 data2;
    vec4 data3;
    vec4 data4;
    int int1;
    int int2;
    int int3;
    int int4;
    int int5;
    int int6;
    int int7;
    int int8;
    int index1;
    int type;
    int index1Type;
};

struct WorldObjectIndex {
    vec4 data1;
    vec4 data2;
    vec4 data3;
    vec4 data4;
    int int1;
    int int2;
    int int3;
    int int4;
    int int5;
    int int6;
    int int7;
    int int8;
    int index;
    int type;
};

struct PixelInfo{
	float dist;
	int index;
	int object;
	vec3 hitPos;
	vec3 normal;
};

struct HitInfo{
	vec3 rd;
	vec3 ro;
	vec3 color;
	float totalDist;
	int index;
	int steps;
	float influence;
	float totalInfluence;
	int iterDepth;
	int parent;
};

layout(binding = 0) uniform CameraData {
    vec3 camera_pos;
    vec3 camera_rot;
    vec3 light_pos;
    vec4 data1;
    vec4 data2;
    vec4 data3;
    vec4 data4;
    vec2 resolution;
    int int1;
    int int2;
    int int3;
    int int4;
    int int5;
    int int6;
    int int7;
    int int8;
    float time;
    int num_steps;
    float min_step;
    float max_dist;
    int ray_depth;
} camData;

layout(binding = 1) uniform WorldObjectsData {
    int num_objects;
    WorldObject objects[OBJECT_COUNT_MAX];
	int num_combine_modifiers;
    WorldObjectCombineModifier combineModifiers[OBJECT_COUNT_MAX];
	int num_domain_modifiers;
    WorldObjectDomainModifier domainModifiers[OBJECT_COUNT_MAX];
	int num_indices;
	WorldObjectIndex indices[OBJECT_COUNT_MAX];
} worldObjectsData;

layout(location = 0) out vec4 outColor;

float d_wiggle_sphere(in vec3 p, float radius, float multi){
    float displacement = sin(2.0 * p.x + multi) * sin(3.0 * p.y + multi * -3.0) * sin(7.0 * p.z - multi * 0.5) * 0.25;
    return length(p) - radius + displacement;
}

float d_wiggle_plane(in vec3 camera, in vec3 origin, in vec3 normal, float multi){
    float displacement = sin(2.0 * camera.x + multi) * sin(3.0 * camera.x + 2.5 * camera.z + multi * -3.0) * sin(7.0 * camera.z - multi * 0.5) * 0.25 * cos(1.0 * camera.x + multi) * cos(0.5 * camera.x + 1.5 * camera.z + multi * -3.0) * cos(2.0 * camera.z - multi * 0.5) * 0.25;
    return dot(camera, normal) - dot(origin, normal) + displacement;
}

/*
std::vector<std::string> worldObjectTypes = {
    "Nothing",
    "Plane",
    "Sphere",
    "Box",
    "Box2",
    "Corner",
    "Wiggle Sphere",
    "Cylinder",
    "Capsule",
    "Torus",
    "Circle",
    "Disc",
    "Hexagon Circumcircle",
    "Hexagon Incircle",
    "Cone",
    "Wiggle Sphere Move",
    "Wiggle Plane Move"
};
*/

float map_the_object(in vec3 p, in int objectIndex){

	vec3 point = p - worldObjectsData.objects[objectIndex].center;
	switch(worldObjectsData.objects[objectIndex].type){
	case 0:
		return 1000000000.0;
	case 1:
		return fPlane(point, worldObjectsData.objects[objectIndex].size, 0.0);
	case 2:
		return fSphere(point, worldObjectsData.objects[objectIndex].size.x);
	case 3:
		return fBox(point, worldObjectsData.objects[objectIndex].size);
	case 4:
		return fBox2(point.xz, worldObjectsData.objects[objectIndex].size.xz);
	case 5:
		return fCorner(point.xy);
	case 6:
		return fBlob(point);
	case 7:
		return fCylinder(point, worldObjectsData.objects[objectIndex].size.x, worldObjectsData.objects[objectIndex].size.y);
	case 8:
		return fCapsule(point, worldObjectsData.objects[objectIndex].size.x, worldObjectsData.objects[objectIndex].size.y);
	case 9:
		return fTorus(point, worldObjectsData.objects[objectIndex].size.x, worldObjectsData.objects[objectIndex].size.y);
	case 10:
		return fCircle(point, worldObjectsData.objects[objectIndex].size.x);
	case 11:
		return fDisc(point, worldObjectsData.objects[objectIndex].size.x);
	case 12:
		return fHexagonCircumcircle(point, worldObjectsData.objects[objectIndex].size.xy);
	case 13:
		return fHexagonIncircle(point, worldObjectsData.objects[objectIndex].size.xy);
	case 14:
		return fCone(point, worldObjectsData.objects[objectIndex].size.x, worldObjectsData.objects[objectIndex].size.y);
	case 15:
	    return d_wiggle_sphere(point, worldObjectsData.objects[objectIndex].size.x, camData.time);
	case 16:
		return d_wiggle_plane(p, worldObjectsData.objects[objectIndex].center, worldObjectsData.objects[objectIndex].size, camData.time);
	}
    return 1000000000.0;
}

vec3 reflect_ray(in vec3 incident, in vec3 normal){
    return incident - 2.0 * dot(incident, normal) * normal;
}

vec3 calculate_normal_object(in vec3 p, int objectIndex, float totalDist){
    // const vec3 small_step = vec3(max(camData.data4.z, camData.data4.z*totalDist), 0.0, 0.0);
	vec3 small_step;
	if(camData.data4.z <= 0.0){
		 small_step= vec3(0.001, 0.0, 0.0);
	}
	else{
		small_step = vec3(max(camData.data4.z, camData.data4.z*totalDist), 0.0, 0.0);
	}

    float gradient_x = map_the_object(p + small_step.xyy, objectIndex) - map_the_object(p - small_step.xyy, objectIndex);
    float gradient_y = map_the_object(p + small_step.yxy, objectIndex) - map_the_object(p - small_step.yxy, objectIndex);
    float gradient_z = map_the_object(p + small_step.yyx, objectIndex) - map_the_object(p - small_step.yyx, objectIndex);

    vec3 normal = vec3(gradient_x, gradient_y, gradient_z);

    return normalize(normal);
}

/*std::vector<std::string> worldObjectCombineModifierTypes = {
    "Nothing",
    "Union",
    "Intersection",
    "Difference",
    "Union Chamfer",
    "Intersection Chamfer",
    "Difference Chamfer",
    "Union Round",
    "Intersection Round",
    "Difference Round",
    "Union Columns",
    "Intersection Columns",
    "Difference Columns",
    "Union Stairs",
    "Intersection Stairs",
    "Difference Stairs",
    "Union Soft",
    "Engrave",
    "Grove",
    "Tounge",
	"Bounding Box",
};

};*/

PixelInfo map_the_combine_modifier(int type, float dist1, float dist2, int index1, int index2, vec3 normal1, vec3 normal2, vec4 data){
	
	PixelInfo result;

	switch(type){
        case 1:
            result.dist = min(dist1, dist2);
			result.object = dist1 < dist2 ? index1 : index2;
			result.normal = dist1 < dist2 ? normal1 : normal2;
			break;
		case 2:
			result.dist = max(dist1, dist2);
			result.object = dist1 > dist2 ? index1 : index2;
			result.normal = dist1 > dist2 ? normal1 : normal2;
			break;
		case 3:
			result.dist = max(dist1, -dist2);
			result.object = index1; // dist1 > -dist2 ? index1 : index2;
			result.normal = normal1;
			break;
		case 4:
			result.dist = fOpUnionChamfer(dist1, dist2, data[0]);
			result.object = dist1 < dist2 ? index1 : index2;
			result.normal = dist1 < dist2 ? normal1 : normal2;
			break;
		case 5:
			result.dist = fOpIntersectionChamfer(dist1, dist2, data[0]);
			result.object = dist1 > dist2 ? index1 : index2;
			result.normal = dist1 > dist2 ? normal1 : normal2;
			break;
		case 6:
			result.dist = fOpDifferenceChamfer(dist1, dist2, data[0]);
			result.object = index1; // dist1 > -dist2 ? index1 : index2;
			result.normal = normal1;
			break;
		case 7:
			result.dist = fOpUnionRound(dist1, dist2, data[0]);
			result.object = dist1 < dist2 ? index1 : index2;
			result.normal = dist1 < dist2 ? normal1 : normal2;
			break;
		case 8:
			result.dist = fOpIntersectionRound(dist1, dist2, data[0]);
			result.object = dist1 > dist2 ? index1 : index2;
			result.normal = dist1 > dist2 ? normal1 : normal2;
			break;
		case 9:
			result.dist = fOpDifferenceRound(dist1, dist2, data[0]);
			result.object = index1; // dist1 > -dist2 ? index1 : index2;
			result.normal = normal1;
			break;
		case 10:
			result.dist = fOpUnionColumns(dist1, dist2, data[0], data[1]);
			result.object = dist1 < dist2 ? index1 : index2;
			result.normal = dist1 < dist2 ? normal1 : normal2;
			break;
		case 11:
			result.dist = fOpIntersectionColumns(dist1, dist2, data[0], data[1]);
			result.object = dist1 > dist2 ? index1 : index2;
			result.normal = dist1 > dist2 ? normal1 : normal2;
			break;
		case 12:
			result.dist = fOpDifferenceColumns(dist1, dist2, data[0], data[1]);
			result.object = index1; // dist1 > -dist2 ? index1 : index2;
			result.normal = normal1;
			break;
		case 13:
			result.dist = fOpUnionStairs(dist1, dist2, data[0], data[1]);
			result.object = dist1 < dist2 ? index1 : index2;
			result.normal = dist1 < dist2 ? normal1 : normal2;
			break;
		case 14:
			result.dist = fOpIntersectionStairs(dist1, dist2, data[0], data[1]);
			result.object = dist1 > dist2 ? index1 : index2;
			result.normal = dist1 > dist2 ? normal1 : normal2;
			break;
		case 15:
			result.dist = fOpDifferenceStairs(dist1, dist2, data[0], data[1]);
			result.object = index1; // dist1 > -dist2 ? index1 : index2;
			result.normal = normal1;
			break;
		case 16:
			result.dist = fOpUnionSoft(dist1, dist2, data[0]);
			result.object = dist1 < dist2 ? index1 : index2;
			result.normal = dist1 < dist2 ? normal1 : normal2;
			break;
		case 17:
			result.dist = fOpPipe(dist1, dist2, data[0]);
			result.object = dist1 < dist2 ? index1 : index2;
			result.normal = dist1 < dist2 ? normal1 : normal2;
			break;
		case 18:
			result.dist = fOpEngrave(dist1, dist2, data[0]);
			result.object = dist1 < dist2 ? index1 : index2;
			result.normal = dist1 < dist2 ? normal1 : normal2;
			break;
		case 19:
			result.dist = fOpGroove(dist1, dist2, data[0], data[1]);
			result.object = dist1 < dist2 ? index1 : index2;
			result.normal = dist1 < dist2 ? normal1 : normal2;
			break;
		case 20:
			result.dist = fOpTongue(dist1, dist2, data[0], data[1]);
			result.object = dist1 < dist2 ? index1 : index2;
			result.normal = dist1 < dist2 ? normal1 : normal2;
			break;
		case 22:
			result.dist = max(-dist1, dist2);
			result.object = index2; // dist1 > -dist2 ? index1 : index2;
			result.normal = normal2;
			break;
		default:
			result.dist = dist1;
			result.object = index1;
			result.normal = normal1;
	}

	return result;
}

void pTwist(inout vec3 p, int axis1, int axis2, int axis3, int axis4, float amount){
	float c = cos(amount*p[axis1]);
	float s = sin(amount*p[axis1]);
	mat2  m = mat2(c,-s,s,c);
	p = vec3(m*vec2(p[axis2], p[axis3]),p[axis4]);
}

/*
std::vector<std::string> worldObjectDomainModifierTypes = {
    "Nothing",
    "Translate 3D",
    "Scale 3D",
    "Rotate 3D",
    "Reflect Plane",
    "Repeat 3D",
    "Repeat Mirror 3D",
    "Extend 3D",
    "Multiply 3D",
    "Ring 3D",
    "Octant 3D",
	"Twist 3D",
    "Bend 3D",
	"Wiggle Space 3D",
	"Wiggle Space Move 3D"
	"Wiggle Dist 3D",
	"Wiggle Dist Move 3D",
	"Rotate Space 3D",
	"Rotate Space Move 3D",
	"Rotate Dist 3D",
	"Rotate Dist Move 3D",
	"Look At Camera",
};
*/
void map_the_domain_modifier(inout vec3 p, int modifierIndex){


	switch(worldObjectsData.domainModifiers[modifierIndex].type){
		case 1:
			p -= worldObjectsData.domainModifiers[modifierIndex].data1.xyz;
			break;
		case 2:
			if (worldObjectsData.domainModifiers[modifierIndex].data1.x != 0)	p.x /= worldObjectsData.domainModifiers[modifierIndex].data1.x;
			if (worldObjectsData.domainModifiers[modifierIndex].data1.y != 0)	p.y /= worldObjectsData.domainModifiers[modifierIndex].data1.y;
			if (worldObjectsData.domainModifiers[modifierIndex].data1.z != 0)	p.z /= worldObjectsData.domainModifiers[modifierIndex].data1.z;
			break;
		case 3:
			if (worldObjectsData.domainModifiers[modifierIndex].data1.x != 0)	pR(p.yz, worldObjectsData.domainModifiers[modifierIndex].data1.x);
			if (worldObjectsData.domainModifiers[modifierIndex].data1.y != 0)	pR(p.xz, worldObjectsData.domainModifiers[modifierIndex].data1.y);
			if (worldObjectsData.domainModifiers[modifierIndex].data1.z != 0)	pR(p.xy, worldObjectsData.domainModifiers[modifierIndex].data1.z);
			break;
		case 4:
			pReflect(p, worldObjectsData.domainModifiers[modifierIndex].data1.xyz, worldObjectsData.domainModifiers[modifierIndex].data1.w);
			break;
		case 5: 
			if (worldObjectsData.domainModifiers[modifierIndex].data1.x != 0)	pMod1(p.x, worldObjectsData.domainModifiers[modifierIndex].data1.x);
			if (worldObjectsData.domainModifiers[modifierIndex].data1.y != 0)	pMod1(p.y, worldObjectsData.domainModifiers[modifierIndex].data1.y);
			if (worldObjectsData.domainModifiers[modifierIndex].data1.z != 0)	pMod1(p.z, worldObjectsData.domainModifiers[modifierIndex].data1.z);
			break;
		case 6:
			if (worldObjectsData.domainModifiers[modifierIndex].data1.x != 0)	pModMirror1(p.x, worldObjectsData.domainModifiers[modifierIndex].data1.x);
			if (worldObjectsData.domainModifiers[modifierIndex].data1.y != 0)	pModMirror1(p.y, worldObjectsData.domainModifiers[modifierIndex].data1.y);
			if (worldObjectsData.domainModifiers[modifierIndex].data1.z != 0)	pModMirror1(p.z, worldObjectsData.domainModifiers[modifierIndex].data1.z);
			break;
		case 7:
			if (worldObjectsData.domainModifiers[modifierIndex].data1.x != 0)	pModSingle1(p.x, worldObjectsData.domainModifiers[modifierIndex].data1.x);
			if (worldObjectsData.domainModifiers[modifierIndex].data1.y != 0)	pModSingle1(p.y, worldObjectsData.domainModifiers[modifierIndex].data1.y);
			if (worldObjectsData.domainModifiers[modifierIndex].data1.z != 0)	pModSingle1(p.z, worldObjectsData.domainModifiers[modifierIndex].data1.z);
			break;
		case 8:
			if (worldObjectsData.domainModifiers[modifierIndex].data1.x != 0)	pModInterval1(p.x, worldObjectsData.domainModifiers[modifierIndex].data1.x, 0.0, worldObjectsData.domainModifiers[modifierIndex].data2.x);
			if (worldObjectsData.domainModifiers[modifierIndex].data1.y != 0)	pModInterval1(p.y, worldObjectsData.domainModifiers[modifierIndex].data1.y, 0.0, worldObjectsData.domainModifiers[modifierIndex].data2.y);
			if (worldObjectsData.domainModifiers[modifierIndex].data1.z != 0)	pModInterval1(p.z, worldObjectsData.domainModifiers[modifierIndex].data1.z, 0.0, worldObjectsData.domainModifiers[modifierIndex].data2.z);
			break;
		case 9:
			if (worldObjectsData.domainModifiers[modifierIndex].data1.x != 0)	pModPolar(p.yz, worldObjectsData.domainModifiers[modifierIndex].data1.x);
			if (worldObjectsData.domainModifiers[modifierIndex].data1.y != 0)	pModPolar(p.xz, worldObjectsData.domainModifiers[modifierIndex].data1.y);
			if (worldObjectsData.domainModifiers[modifierIndex].data1.z != 0)	pModPolar(p.yx, worldObjectsData.domainModifiers[modifierIndex].data1.z);
			break;
		case 10:
			if (worldObjectsData.domainModifiers[modifierIndex].data1.x != 0)	pMirrorOctant(p.yz, vec2(worldObjectsData.domainModifiers[modifierIndex].data1.x, worldObjectsData.domainModifiers[modifierIndex].data2.x));
			if (worldObjectsData.domainModifiers[modifierIndex].data1.y != 0)	pMirrorOctant(p.xz, vec2(worldObjectsData.domainModifiers[modifierIndex].data1.y, worldObjectsData.domainModifiers[modifierIndex].data2.y));
			if (worldObjectsData.domainModifiers[modifierIndex].data1.z != 0)	pMirrorOctant(p.xy, vec2(worldObjectsData.domainModifiers[modifierIndex].data1.z, worldObjectsData.domainModifiers[modifierIndex].data2.z));
			break;
		case 11:
			if (worldObjectsData.domainModifiers[modifierIndex].data1.x != 0)	pTwist(p, 1, 1, 2, 0, worldObjectsData.domainModifiers[modifierIndex].data1.x);
			if (worldObjectsData.domainModifiers[modifierIndex].data1.y != 0)	pTwist(p, 1, 0, 2, 1, worldObjectsData.domainModifiers[modifierIndex].data1.y);
			if (worldObjectsData.domainModifiers[modifierIndex].data1.z != 0)	pTwist(p, 2, 0, 1, 2, worldObjectsData.domainModifiers[modifierIndex].data1.z);
			if (worldObjectsData.domainModifiers[modifierIndex].data2.x != 0)	pTwist(p, 0, 1, 2, 0, worldObjectsData.domainModifiers[modifierIndex].data2.x); // working
			if (worldObjectsData.domainModifiers[modifierIndex].data2.y != 0)	pTwist(p, 1, 2, 0, 1, worldObjectsData.domainModifiers[modifierIndex].data2.y);
			if (worldObjectsData.domainModifiers[modifierIndex].data2.z != 0)	pTwist(p, 2, 1, 0, 2, worldObjectsData.domainModifiers[modifierIndex].data2.z);
			break;
		case 12:
			if (worldObjectsData.domainModifiers[modifierIndex].data1.x != 0)	pTwist(p, 1, 1, 2, 0, worldObjectsData.domainModifiers[modifierIndex].data1.x);
			if (worldObjectsData.domainModifiers[modifierIndex].data1.y != 0)	pTwist(p, 2, 2, 0, 1, worldObjectsData.domainModifiers[modifierIndex].data1.y);
			if (worldObjectsData.domainModifiers[modifierIndex].data1.z != 0)	pTwist(p, 0, 0, 1, 2, worldObjectsData.domainModifiers[modifierIndex].data1.z);
			if (worldObjectsData.domainModifiers[modifierIndex].data2.x != 0)	pTwist(p, 2, 2, 1, 0, worldObjectsData.domainModifiers[modifierIndex].data2.x); // working
			if (worldObjectsData.domainModifiers[modifierIndex].data2.y != 0)	pTwist(p, 0, 0, 2, 1, worldObjectsData.domainModifiers[modifierIndex].data2.y);
			if (worldObjectsData.domainModifiers[modifierIndex].data2.z != 0)	pTwist(p, 1, 1, 0, 2, worldObjectsData.domainModifiers[modifierIndex].data2.z);			
			break;
		case 13: // Wiggle Space move
			if (worldObjectsData.domainModifiers[modifierIndex].data1.x != 0)	p.x += sin(worldObjectsData.domainModifiers[modifierIndex].data1.x * p.y + camData.time * worldObjectsData.domainModifiers[modifierIndex].data1.w) * worldObjectsData.domainModifiers[modifierIndex].data2.x;
			if (worldObjectsData.domainModifiers[modifierIndex].data1.y != 0)	p.y += sin(worldObjectsData.domainModifiers[modifierIndex].data1.y * p.z + camData.time * worldObjectsData.domainModifiers[modifierIndex].data2.w) * worldObjectsData.domainModifiers[modifierIndex].data2.y;
			if (worldObjectsData.domainModifiers[modifierIndex].data1.z != 0)	p.z += sin(worldObjectsData.domainModifiers[modifierIndex].data1.z * p.x + camData.time * worldObjectsData.domainModifiers[modifierIndex].data3.w) * worldObjectsData.domainModifiers[modifierIndex].data2.z;
			if (worldObjectsData.domainModifiers[modifierIndex].data3.x != 0)	p.x += sin(worldObjectsData.domainModifiers[modifierIndex].data3.x * p.z + camData.time * worldObjectsData.domainModifiers[modifierIndex].data1.w) * worldObjectsData.domainModifiers[modifierIndex].data4.x;
			if (worldObjectsData.domainModifiers[modifierIndex].data3.y != 0)	p.y += sin(worldObjectsData.domainModifiers[modifierIndex].data3.y * p.x + camData.time * worldObjectsData.domainModifiers[modifierIndex].data2.w) * worldObjectsData.domainModifiers[modifierIndex].data4.y;
			if (worldObjectsData.domainModifiers[modifierIndex].data3.z != 0)	p.z += sin(worldObjectsData.domainModifiers[modifierIndex].data3.z * p.y + camData.time * worldObjectsData.domainModifiers[modifierIndex].data3.w) * worldObjectsData.domainModifiers[modifierIndex].data4.z;
			break;
		case 14: // Wiggle Camera move
			if (worldObjectsData.domainModifiers[modifierIndex].data1.x != 0)	p.x += sin(worldObjectsData.domainModifiers[modifierIndex].data1.x * (p.y - camData.camera_pos.y) + camData.time * worldObjectsData.domainModifiers[modifierIndex].data1.w) * worldObjectsData.domainModifiers[modifierIndex].data2.x;
			if (worldObjectsData.domainModifiers[modifierIndex].data1.y != 0)	p.y += sin(worldObjectsData.domainModifiers[modifierIndex].data1.y * (p.z - camData.camera_pos.z) + camData.time * worldObjectsData.domainModifiers[modifierIndex].data2.w) * worldObjectsData.domainModifiers[modifierIndex].data2.y;
			if (worldObjectsData.domainModifiers[modifierIndex].data1.z != 0)	p.z += sin(worldObjectsData.domainModifiers[modifierIndex].data1.z * (p.x - camData.camera_pos.x) + camData.time * worldObjectsData.domainModifiers[modifierIndex].data3.w) * worldObjectsData.domainModifiers[modifierIndex].data2.z;
			if (worldObjectsData.domainModifiers[modifierIndex].data3.x != 0)	p.x += sin(worldObjectsData.domainModifiers[modifierIndex].data3.x * (p.z - camData.camera_pos.z) + camData.time * worldObjectsData.domainModifiers[modifierIndex].data1.w) * worldObjectsData.domainModifiers[modifierIndex].data4.x;
			if (worldObjectsData.domainModifiers[modifierIndex].data3.y != 0)	p.y += sin(worldObjectsData.domainModifiers[modifierIndex].data3.y * (p.x - camData.camera_pos.x) + camData.time * worldObjectsData.domainModifiers[modifierIndex].data2.w) * worldObjectsData.domainModifiers[modifierIndex].data4.y;
			if (worldObjectsData.domainModifiers[modifierIndex].data3.z != 0)	p.z += sin(worldObjectsData.domainModifiers[modifierIndex].data3.z * (p.y - camData.camera_pos.y) + camData.time * worldObjectsData.domainModifiers[modifierIndex].data3.w) * worldObjectsData.domainModifiers[modifierIndex].data4.z;
			break;
		case 15: // Rotate Space move
			if (worldObjectsData.domainModifiers[modifierIndex].data1.x != 0)	pR(p.yz, worldObjectsData.domainModifiers[modifierIndex].data1.x * .001 * p.x + camData.time * worldObjectsData.domainModifiers[modifierIndex].data1.w);	
			if (worldObjectsData.domainModifiers[modifierIndex].data1.y != 0)	pR(p.xz, worldObjectsData.domainModifiers[modifierIndex].data1.y * .001 * p.y + camData.time * worldObjectsData.domainModifiers[modifierIndex].data2.w);
			if (worldObjectsData.domainModifiers[modifierIndex].data1.z != 0)	pR(p.xy, worldObjectsData.domainModifiers[modifierIndex].data1.z * .001 * p.z + camData.time * worldObjectsData.domainModifiers[modifierIndex].data3.w);
			break;
		case 16: // Rotate Camera move
			if (worldObjectsData.domainModifiers[modifierIndex].data1.x != 0){
				vec2 tempPoint = p.yz - camData.camera_pos.yz;
				pR(tempPoint, worldObjectsData.domainModifiers[modifierIndex].data1.x * .001 * (p.x - camData.camera_pos.x) + camData.time * worldObjectsData.domainModifiers[modifierIndex].data1.w);	
				p.yz = tempPoint + camData.camera_pos.yz;
			}
			if (worldObjectsData.domainModifiers[modifierIndex].data1.y != 0){
				vec2 tempPoint = p.xz - camData.camera_pos.xz;
				pR(tempPoint, worldObjectsData.domainModifiers[modifierIndex].data1.y * .001 * (p.y - camData.camera_pos.y) + camData.time * worldObjectsData.domainModifiers[modifierIndex].data2.w);
				p.xz = tempPoint + camData.camera_pos.xz;
			}
			if (worldObjectsData.domainModifiers[modifierIndex].data1.z != 0){
				vec2 tempPoint = p.xy - camData.camera_pos.xy;
				pR(tempPoint, worldObjectsData.domainModifiers[modifierIndex].data1.z * .001 * (p.z - camData.camera_pos.z) + camData.time * worldObjectsData.domainModifiers[modifierIndex].data3.w);
				p.xy = tempPoint + camData.camera_pos.xy;
			}
			break;
		case 17: // Look At camera
			vec3 lookDir = normalize(camData.camera_pos - p);
			vec3 right = normalize(cross(vec3(0.0, 1.0, 0.0), lookDir));
			vec3 up = cross(lookDir, right);

			mat3 rotationMatrix = mat3(right, up, lookDir);

			// Apply this rotation matrix to your object coordinates
			p = rotationMatrix * p;

		default:
			break;
	}
}

vec4 triPlanar(int textureIndex, vec3 p, vec3 blend, vec4 offset, int objectType){
	p -= offset.xyz;
	p.y = -p.y;
	return (texture(texSampler[textureIndex], p.xy * 0.5 + 0.5) * blend.z +
			texture(texSampler[textureIndex], p.xz * 0.5 + 0.5) * blend.y +
			texture(texSampler[textureIndex], p.zy * 0.5 + 0.5) * blend.x);
}

vec4 getTextureValForCoordType(int textureIndex, vec3 worldPos, vec3 normal, vec3 size, vec3 scale, vec4 offset, int objectType){
	vec3 p, blend;
	blend = abs(normal);
	switch(objectType){
	case 1: // "Plane"
		p = worldPos / scale;
		break;
	case 2: // "Sphere"
		p = worldPos / (scale*size.x);
		blend = pow(blend, vec3(5.0));
		blend /= blend.x + blend.y + blend.z;
		break;
	case 3: // "Box"
		p = worldPos / (scale*size);
		break;
	case 4: // "Box2"
		p = worldPos / (scale*size.xyy);
		break;
	case 5: // "Corner"
		p = worldPos / (scale);
		break;
	case 6: // "Wiggle Sphere"
		p = worldPos / scale;
		blend = pow(blend, vec3(5.0));
		blend /= blend.x + blend.y + blend.z;
		break;
	case 7: // "Cylinder"
	case 8: // "Capsule"
		p = worldPos / (scale*size.xyx);
		blend = pow(blend, vec3(5.0));
		blend /= blend.x + blend.y + blend.z;
		break;
	case 9: // "Torus"
		p = worldPos / (scale*size.yxy);
		blend = pow(blend, vec3(5.0));
		blend /= blend.x + blend.y + blend.z;
		break;
	case 10: // "Circle"
	case 11: // "Disc"
		p = worldPos / (scale*size);
		break;
	case 12: // "Hexagon Circumcircle"
	case 13: // "Hexagon Incircle"
	case 14: // "Cone"
	case 15: // "Wiggle Sphere Move"
		p = worldPos / (scale*size.xyx);
		blend = pow(blend, vec3(5.0));
		blend /= blend.x + blend.y + blend.z;
		break;
	case 16: // "Wiggle Plane Move"
		p = worldPos / scale;
		break;
	}
	return triPlanar(textureIndex, p, blend, offset, objectType);
}

vec4 sampleSkybox(vec3 rayDirection, int textureIndex) {
    // Normalize the ray direction
    vec3 dir = normalize(rayDirection);

    // Spherical coordinates
    float theta = atan(dir.z, dir.x);
    float phi = acos(dir.y);

    // Handle the derivative discontinuity using the absolute x component (as in the tunnel example)
    float thetaR = atan(dir.z, abs(dir.x));

    // Texture coordinates for both sides
    float uL = theta / (2.0 * PI) + 0.5;
    float uR = thetaR / (2.0 * PI) + 0.5;
    float v = phi / PI;

    // Calculate the texture gradients using the right side's u coordinate
    vec2 uvL = vec2(uL, v);
    vec2 uvR = vec2(uR, v);

    // Fetch from texture using textureGrad to avoid mipmapping artifacts
    return textureGrad(texSampler[textureIndex], uvL, dFdx(uvR), dFdy(uvR));
}

vec4 getTextureValForType(int textureIndex, vec3 worldPos, vec3 normal, vec3 size, vec3 scale, vec4 offset, int objectType, vec2 uv, vec3 rayDir, int orientationType){
	switch(orientationType){
	case 0: // World
	case 1: // Object
		return getTextureValForCoordType(textureIndex, worldPos, normal, size, scale, offset, objectType);
	case 2: // Screen
		return texture(texSampler[textureIndex], vec2(uv.x-offset.x, offset.y-uv.y)*scale.xy);
	case 3: // Skybox
		return sampleSkybox(rayDir, textureIndex);
	}
	return vec4(0.0);
}

float bumpMapping(int textureIndex, vec3 p, vec3 n, float dist, vec3 size, vec3 scale, vec4 offset, int objectType){
	float bump = 0.0;
	if (dist < camData.data3.z){
		vec3 normal = normalize(n);
		bump += camData.data3.y * (1 - getTextureValForCoordType(textureIndex, p, normal, size, scale, offset, objectType).r);
	}
	return bump;
}

PixelInfo map_the_index(vec3 p, int i, int skipIndex){
    PixelInfo result;
    result.dist = camData.max_dist;
    result.index = i;
    result.object = -1;
    result.hitPos = p;
	result.normal = vec3(0.0);
    
    if(i == skipIndex) return result;

    float cur_dist = 0.0;
    int cur_object = -1;
	vec3 cur_normal = vec3(0.0);

    int cur_type_to_check = worldObjectsData.indices[i].type;
    int cur_index_to_check = worldObjectsData.indices[i].index;
    int distBufferSize = 0;

    int stop_crash = 0;

    float distBuffer[OBJECT_COUNT_MAX];
    int modifierBuffer[OBJECT_COUNT_MAX];
	vec3 normalBuffer[OBJECT_COUNT_MAX];

    while(stop_crash < OBJECT_COUNT_MAX){
        if(cur_type_to_check == 1){
            stop_crash = OBJECT_COUNT_MAX;
            cur_dist = map_the_object(p, cur_index_to_check);
			cur_normal = calculate_normal_object(mix(p, result.hitPos, worldObjectsData.objects[cur_index_to_check].int4), cur_index_to_check, cur_dist);
            if(worldObjectsData.objects[cur_index_to_check].int3 != 0){
				cur_dist += bumpMapping(worldObjectsData.objects[cur_index_to_check].int3, mix(p, result.hitPos, worldObjectsData.objects[cur_index_to_check].int4), cur_normal, cur_dist, worldObjectsData.objects[cur_index_to_check].size, worldObjectsData.objects[cur_index_to_check].data1.rgb, worldObjectsData.objects[cur_index_to_check].data2, worldObjectsData.objects[cur_index_to_check].type);
            }
            cur_object = cur_index_to_check;
            p -= worldObjectsData.objects[cur_index_to_check].center;
        }
        else if(cur_type_to_check == 2){
            stop_crash++;
            int index2 = worldObjectsData.combineModifiers[cur_index_to_check].index2;
            distBuffer[distBufferSize] = map_the_object(p, index2);
            normalBuffer[distBufferSize] = calculate_normal_object(mix(p, result.hitPos, worldObjectsData.objects[index2].int4), index2, distBuffer[distBufferSize]);
            if(worldObjectsData.objects[index2].int3 != 0){
                distBuffer[distBufferSize] += bumpMapping(worldObjectsData.objects[index2].int3, mix(p, result.hitPos, worldObjectsData.objects[cur_index_to_check].int4), normalBuffer[distBufferSize], distBuffer[distBufferSize], worldObjectsData.objects[index2].size, worldObjectsData.objects[index2].data1.rgb, worldObjectsData.objects[index2].data2, worldObjectsData.objects[index2].type);
            }
            if(worldObjectsData.combineModifiers[cur_index_to_check].type == 21){
                if(distBuffer[distBufferSize] > worldObjectsData.combineModifiers[cur_index_to_check].data1.x){
                    cur_dist = distBuffer[distBufferSize];
                    cur_object = worldObjectsData.combineModifiers[cur_index_to_check].index1;
                    stop_crash = OBJECT_COUNT_MAX;
                    break;
                }
                else{
                    cur_type_to_check = worldObjectsData.combineModifiers[cur_index_to_check].index1Type;
                    cur_index_to_check = worldObjectsData.combineModifiers[cur_index_to_check].index1;
                    stop_crash++;
                }
                continue;
            }
            modifierBuffer[distBufferSize] = cur_index_to_check;
            distBufferSize++;

            cur_type_to_check = worldObjectsData.combineModifiers[cur_index_to_check].index1Type;
            cur_index_to_check = worldObjectsData.combineModifiers[cur_index_to_check].index1;

        }
        else if(cur_type_to_check == 3){
            stop_crash++;
            map_the_domain_modifier(p, cur_index_to_check);
            cur_dist = 1.0;
            cur_type_to_check = worldObjectsData.domainModifiers[cur_index_to_check].index1Type;
            cur_index_to_check = worldObjectsData.domainModifiers[cur_index_to_check].index1;
        }
        else{
            cur_dist = camData.max_dist;
            stop_crash = OBJECT_COUNT_MAX;
        }
    }

    while(distBufferSize > 0){
        distBufferSize--;
        PixelInfo temp = map_the_combine_modifier(worldObjectsData.combineModifiers[modifierBuffer[distBufferSize]].type, cur_dist, distBuffer[distBufferSize], cur_object, worldObjectsData.combineModifiers[modifierBuffer[distBufferSize]].index2, cur_normal, normalBuffer[distBufferSize],worldObjectsData.combineModifiers[modifierBuffer[distBufferSize]].data1);
        cur_dist = temp.dist;
        cur_object = temp.object;
		cur_normal = temp.normal;
    }
	result.dist = cur_dist;
	result.object = cur_object;
	result.hitPos = p;
	result.normal = cur_normal;
    return result;
}

vec3 calculate_normal_index(in vec3 p, int i, int skipIndex, float totalDist){
    // const vec3 small_step = vec3(max(camData.data4.z, camData.data4.z*totalDist), 0.0, 0.0);
	vec3 small_step;
	if(camData.data4.z <= 0.0){
		 small_step= vec3(0.001, 0.0, 0.0);
	}
	else{
		small_step = vec3(max(camData.data4.z, camData.data4.z*totalDist), 0.0, 0.0);
	}

    float gradient_x = map_the_index(p + small_step.xyy, i, skipIndex).dist - map_the_index(p - small_step.xyy, i, skipIndex).dist;
    float gradient_y = map_the_index(p + small_step.yxy, i, skipIndex).dist - map_the_index(p - small_step.yxy, i, skipIndex).dist;
    float gradient_z = map_the_index(p + small_step.yyx, i, skipIndex).dist - map_the_index(p - small_step.yyx, i, skipIndex).dist;

    vec3 normal = vec3(gradient_x, gradient_y, gradient_z);

    return normalize(normal);
}

PixelInfo map_the_world_new(in vec3 point, int skipIndex){
    PixelInfo pOutput;
    pOutput.dist = 100000.0;
    pOutput.index = -1;
    pOutput.object = -1;
    pOutput.hitPos = vec3(100000.0);
	pOutput.normal = vec3(0.0);

    for (int i = 0; i < worldObjectsData.num_indices; ++i)
    {
        vec3 p = point;
        PixelInfo cur = map_the_index(p, i, skipIndex);

        if (cur.dist < pOutput.dist){
            pOutput.dist = cur.dist;
            pOutput.index = i;
            pOutput.object = cur.object;
            pOutput.hitPos = cur.hitPos;
			pOutput.normal = cur.normal;
        }
    }

    return pOutput;
}
/**/

// Quaternion multiplication: combines two quaternions
vec4 quatMult(vec4 q1, vec4 q2) {
    return vec4(
        q1.w * q2.xyz + q2.w * q1.xyz + cross(q1.xyz, q2.xyz),
        q1.w * q2.w - dot(q1.xyz, q2.xyz)
    );
}

// Rotate a vector by a quaternion
vec3 rotateByQuaternion(vec3 v, vec4 q) {
    vec4 q_conjugate = vec4(-q.xyz, q.w);
    vec4 q_v = vec4(v, 0.0);
    vec4 rotated_v = quatMult(quatMult(q, q_v), q_conjugate);
    return rotated_v.xyz;
}

// Create a quaternion from an axis-angle rotation
vec4 quatFromAxisAngle(vec3 axis, float angle) {
    float halfAngle = angle * 0.5;
    float s = sin(halfAngle);
    return vec4(axis * s, cos(halfAngle));
}

vec3 rotateVec3ByYawPitchRoll(vec3 v, float yaw, float pitch, float roll) {
    // Create quaternion for yaw (rotation around the Y axis)
    vec4 q_yaw = quatFromAxisAngle(vec3(0.0, 1.0, 0.0), yaw);
    
    // Create quaternion for pitch (rotation around the X axis)
    vec4 q_pitch = quatFromAxisAngle(vec3(1.0, 0.0, 0.0), pitch);
    
	vec4 q_roll = quatFromAxisAngle(vec3(0.0, 0.0, 1.0), roll);

    // Combine the yaw and pitch quaternions
    vec4 q_combined = quatMult(q_yaw, q_pitch);

	q_combined = quatMult(q_combined, q_roll);
    
    // Rotate the vector by the combined quaternion
    return rotateByQuaternion(v, q_combined);
}

vec3 calculate_normal_world(in vec3 p, int skipIndex, float totalDist){
    // const vec3 small_step = vec3(max(camData.data4.z, camData.data4.z*totalDist), 0.0, 0.0);
	vec3 small_step;
	if(camData.data4.z <= 0.0){
		 small_step= vec3(0.001, 0.0, 0.0);
	}
	else{
		small_step = vec3(max(camData.data4.z, camData.data4.z*totalDist), 0.0, 0.0);
	}

    float gradient_x = map_the_world_new(p + small_step.xyy, skipIndex).dist - map_the_world_new(p - small_step.xyy, skipIndex).dist;
    float gradient_y = map_the_world_new(p + small_step.yxy, skipIndex).dist - map_the_world_new(p - small_step.yxy, skipIndex).dist;
    float gradient_z = map_the_world_new(p + small_step.yyx, skipIndex).dist - map_the_world_new(p - small_step.yyx, skipIndex).dist;

    vec3 normal = vec3(gradient_x, gradient_y, gradient_z);

    return normalize(normal);
}

vec3 ray_march_shadow(in vec3 ro, in vec3 rd, int skipIndex, int remainingSteps){
	float total_distance_traveled = 0.0;
	float minStep = camData.min_step * 10;

    for (int i = remainingSteps; i < camData.num_steps; ++i)
    {
        vec3 current_position = ro + total_distance_traveled * rd;
		float dist = length(current_position - camData.light_pos);
		float cur_dist = length(current_position - ro);

        PixelInfo closestInfo = map_the_world_new(current_position, skipIndex);

		if (closestInfo.dist < camData.min_step && cur_dist > camData.min_step * 10) {
			return vec3(0.0);
		}
        if (dist < camData.min_step) {
			return vec3(1.0);
        }
		total_distance_traveled += min(closestInfo.dist, dist); // max(min(closestInfo.dist, dist), minStep);
    }
    return vec3(0.0);
}
	
/*
vec3 ray_march_new2(in vec3 ro, in vec3 rd, int skipIndex, int remainingSteps)
{
    float total_distance_traveled = 0.0;
	float minStep = camData.min_step * 10;

    for (int i = remainingSteps; i < camData.num_steps; ++i)
    {
        vec3 current_position = ro + total_distance_traveled * rd;

        PixelInfo closestInfo = map_the_world_new(current_position, skipIndex);

        if (closestInfo.dist < camData.min_step) 
        {   
			WorldObject current_object = worldObjectsData.objects[closestInfo.object];
            vec3 normal = calculate_normal_world(current_position, skipIndex);
            vec3 direction_to_light = normalize(current_position - camData.light_pos);

            
            vec3 current_color;

            if(current_object.color.x == -2.0){
                current_color = normal * 0.5 + 0.5;
            }
            else{
                current_color = current_object.color;
                float diffuse_intensity = max(0.0, dot(normal, -direction_to_light));
                current_color = current_color * (1.0 - current_object.diffuse_intensity) + current_color * diffuse_intensity * current_object.diffuse_intensity;
            }

			if(current_object.has_shadow == 1){
				vec3 newRayDir = normalize(camData.light_pos - current_position);
				vec3 newPos = current_position + newRayDir * 2.5 * camData.min_step;
				vec3 shadow = ray_march_shadow(newPos, newRayDir, -1, i);
                current_color = current_color * (1.0 - current_object.diffuse_intensity) + current_color * shadow * current_object.diffuse_intensity;
			}
			
            return current_color; // * min(1.0, 20.0/total_distance_traveled);
        }

        if (total_distance_traveled > camData.max_dist)
        {
            return sampleSkybox(rd, texSampler);
        }
        total_distance_traveled += closestInfo.dist; // max(closestInfo.dist, minStep);
    }
    return vec3(0.0);
} 

vec3 ray_march_new2(in vec3 ro, in vec3 rd, int skipIndex, int remainingSteps)
{
    float total_distance_traveled = 0.0;
    

    for (int i = remainingSteps; i < camData.num_steps; ++i)
    {
        vec3 current_position = ro + total_distance_traveled * rd;

        PixelInfo closestInfo = map_the_world_new(current_position, -1);

        if (closestInfo.dist < camData.min_step) 
        {   
			WorldObject current_object = worldObjectsData.objects[closestInfo.object];
            vec3 normal = calculate_normal_world(current_position, -1);
            vec3 direction_to_light = normalize(current_position - camData.light_pos);

            
            vec3 current_color;

            if(current_object.color.x == -2.0){
                current_color = normal * 0.5 + 0.5;
            }
            else{
                current_color = current_object.color;
                float diffuse_intensity = max(0.0, dot(normal, -direction_to_light));
                current_color = current_color * (1.0 - current_object.diffuse_intensity) + current_color * diffuse_intensity * current_object.diffuse_intensity;
            }
			
			if (current_object.reflectivity > 0.0){
                float reflectivity = current_object.reflectivity;
                current_object.reflectivity = 0.0;
				vec3 newRayDir = reflect_ray(rd, normal);
				vec3 newPos = current_position + newRayDir * 1.5 * camData.min_step;
                vec3 reflection = ray_march_new3(newPos, newRayDir, -1, i);
                current_color = (reflection * reflectivity + (1.0 - reflectivity) * current_color);
            }
            if (current_object.transparency > 0.0){
                float transparency = current_object.transparency;
                current_object.transparency = 0.0;
                vec3 transparency_color = ray_march_new3(current_position, rd, closestInfo.index, i);
                current_color = transparency_color * transparency + (1.0 - transparency) * current_color;
            }
			
            return current_color; // * min(1.0, 20.0/total_distance_traveled);
        }

        if (total_distance_traveled > camData.max_dist)
        {
            return sampleSkybox(rd, texSampler);
        }
        total_distance_traveled += closestInfo.dist;
    }
    return vec3(0.0);
} 

vec3 ray_march_new(in vec3 ro, in vec3 rd)
{
    float total_distance_traveled = 0.0;
    float minStep = camData.min_step*10;
    for (int i = 0; i < camData.num_steps; ++i)
    {
        vec3 current_position = ro + total_distance_traveled * rd;

        PixelInfo closestInfo = map_the_world_new(current_position, -1);

        if (closestInfo.dist < camData.min_step) 
        {   
			WorldObject current_object = worldObjectsData.objects[closestInfo.object];
            vec3 normal = calculate_normal_world(current_position, -1);
            vec3 direction_to_light = normalize(current_position - camData.light_pos);

            
            vec3 current_color;

            if(current_object.color.x == -2.0){
                current_color = normal * 0.5 + 0.5;
            }
            else{
                current_color = current_object.color;
                float diffuse_intensity = max(0.0, dot(normal, -direction_to_light));
                current_color = current_color * (1.0 - current_object.diffuse_intensity) + current_color * diffuse_intensity * current_object.diffuse_intensity;
            }
			
			if (current_object.reflectivity > 0.0){
                float reflectivity = current_object.reflectivity;
                current_object.reflectivity = 0.0;
				vec3 newRayDir = reflect_ray(rd, normal);
				vec3 newPos = current_position + newRayDir * 1.5 * camData.min_step;
                vec3 reflection = ray_march_new2(newPos, newRayDir, -1, i);
                current_color = (reflection * reflectivity + (1.0 - reflectivity) * current_color);
            }
            if (current_object.transparency > 0.0){
                float transparency = current_object.transparency;
                current_object.transparency = 0.0;
                vec3 transparency_color = ray_march_new2(current_position, rd, closestInfo.index, i);
                current_color = transparency_color * transparency + (1.0 - transparency) * current_color;
            }

			if(current_object.has_shadow == 1){
				vec3 newRayDir = normalize(camData.light_pos - current_position);
				vec3 newPos = current_position + newRayDir * 2.5 * camData.min_step;
				vec3 shadow = ray_march_shadow(newPos, newRayDir, -1, i);
                current_color = current_color * (1.0 - current_object.diffuse_intensity) + current_color * shadow * current_object.diffuse_intensity;
			}
			
            return current_color; // * min(1.0, 20.0/total_distance_traveled);
        }

        if (total_distance_traveled > camData.max_dist)
        {
            return sampleSkybox(rd, texSampler);
        }
        total_distance_traveled += closestInfo.dist; // max(closestInfo.dist, minStep);
    }
    return vec3(0.0);
}
*/

/*
struct HitInfo{
	vec3 rd;
	vec3 ro;
	float totalDist;
	int index;
	int steps;
	float influence;
	int iterDepth;
	int parent;
};
*/

const int MAX_ITER_COUNT = 5;
const int MAX_RAY_COUNT = 31; //pow(2, MAX_ITER_COUNT)-1;

vec3 ray_march_iter(in vec3 roIn, in vec3 rdIn, in vec2 uv){
    float minStep = camData.min_step*10;
	int min_ray_depth = min(camData.ray_depth, MAX_ITER_COUNT);

	HitInfo[MAX_RAY_COUNT] rayInfo;
	rayInfo[0].ro = roIn;
	rayInfo[0].rd = rdIn;
	rayInfo[0].totalDist = 0.0;
	rayInfo[0].index = -1;
	rayInfo[0].steps = 0;
	rayInfo[0].influence = 1.0;
	rayInfo[0].totalInfluence = 1.0;
	rayInfo[0].iterDepth = 1;
	rayInfo[0].color = vec3(1.0);
	rayInfo[0].parent = -1;
	int rayCount = 1;
	for(int rayIndex = 0; rayIndex < rayCount; rayIndex++){
		float cur_dist = 0.0;
		for (int i = 0; i < camData.num_steps; ++i){	
			vec3 current_position = rayInfo[rayIndex].ro + cur_dist * rayInfo[rayIndex].rd;

			PixelInfo closestInfo = map_the_world_new(current_position, rayInfo[rayIndex].index);

			if (closestInfo.dist < camData.min_step && cur_dist > camData.min_step * 10) {   
				WorldObject current_object = worldObjectsData.objects[closestInfo.object];
				vec3 normal = calculate_normal_world(current_position, rayInfo[rayIndex].index, rayInfo[rayIndex].totalDist);
				vec3 direction_to_light = normalize(current_position - camData.light_pos);

				if(current_object.textureIndex != 0){
					vec4 color = getTextureValForType(current_object.textureIndex, mix(closestInfo.hitPos, current_position, current_object.int2), mix(closestInfo.normal, normal, current_object.int2), current_object.size, current_object.data1.rgb, current_object.data2, current_object.type, uv, rayInfo[rayIndex].rd, current_object.int2); // Change `closestInfo.hitPos` to `current_position` to swap from object space to world space for the texture
					rayInfo[rayIndex].color = color.rgb;
					current_object.reflectivity *= 1 - color.a;
					current_object.transparency *= 1 - color.a;
				}
				if(current_object.color.x == -2.0){
					rayInfo[rayIndex].color *= rotateVec3ByYawPitchRoll(normal, camData.data1.x, camData.data1.y, camData.data1.z) * 0.5 + 0.5;
				}
				else{
					rayInfo[rayIndex].color *= current_object.color;
				}
				float diffuse_intensity = max(0.0, dot(normal, -direction_to_light));
				rayInfo[rayIndex].color *= (1.0 - current_object.diffuse_intensity + diffuse_intensity * current_object.diffuse_intensity);
				if(rayInfo[rayIndex].iterDepth < min_ray_depth){
					if (current_object.reflectivity > 0.0){ // * rayInfo[rayIndex].totalInfluence > 0.05){
						vec3 newRayDir = reflect_ray(rayInfo[rayIndex].rd, normal);
						vec3 newPos = current_position + newRayDir * 1.5 * camData.min_step;
						rayInfo[rayCount].ro = newPos;
						rayInfo[rayCount].rd = newRayDir;
						rayInfo[rayCount].totalDist = rayInfo[rayIndex].totalDist;
						rayInfo[rayCount].index = -1;
						rayInfo[rayCount].steps = i;
						rayInfo[rayCount].influence = current_object.reflectivity;
						rayInfo[rayCount].totalInfluence *= current_object.reflectivity;
						rayInfo[rayCount].iterDepth = rayInfo[rayIndex].iterDepth + 1;
						rayInfo[rayCount].color = vec3(1.0);
						rayInfo[rayCount].parent = rayIndex;
						rayCount++;
					}
					if (current_object.transparency > 0.0){ // * rayInfo[rayIndex].totalInfluence > 0.025){
						rayInfo[rayCount].ro = current_position;
						rayInfo[rayCount].rd = refract(rayInfo[rayIndex].rd, normal, current_object.refractive_index);
						rayInfo[rayCount].totalDist = rayInfo[rayIndex].totalDist;
						rayInfo[rayCount].index = closestInfo.index;
						rayInfo[rayCount].steps = i;
						rayInfo[rayCount].influence = current_object.transparency;
						rayInfo[rayCount].totalInfluence *= current_object.transparency;
						rayInfo[rayCount].iterDepth = rayInfo[rayIndex].iterDepth + 1;
						rayInfo[rayCount].color = vec3(1.0);
						rayInfo[rayCount].parent = rayIndex;
						rayCount++;
					}
				}

				if(current_object.shadow_blur > 0){
					vec3 newRayDir = normalize(camData.light_pos - current_position);
					vec3 newPos = current_position + newRayDir * 2.5 * camData.min_step;
					vec3 shadow = ray_march_shadow(newPos, newRayDir, -1, i);
					rayInfo[rayIndex].color = rayInfo[rayIndex].color * (1.0 - current_object.shadow_intensity) + rayInfo[rayIndex].color * shadow * current_object.shadow_intensity;
				}
				break;
			}
			rayInfo[rayIndex].totalDist += closestInfo.dist; // max(closestInfo.dist, minStep);
			cur_dist += closestInfo.dist;
			
			if (rayInfo[rayIndex].totalDist > camData.max_dist)
			{
				rayInfo[rayIndex].color =  sampleSkybox(rayInfo[rayIndex].rd, 0).rgb;
				break;
			}
		}
	}
	
	for(int i = rayCount-1; i >0 ; i--){
		rayInfo[rayInfo[i].parent].color = mix(rayInfo[rayInfo[i].parent].color, rayInfo[i].color, rayInfo[i].influence);
	}
	//finalColor = vec3(rayCount/5);
	return rayInfo[0].color;
}

void main() {

    float aspect = camData.resolution.x / camData.resolution.y;
    
    // Calculate UV coordinates in the range of [-1, 1] and correct for aspect ratio
    vec2 uv = (gl_FragCoord.xy / camData.resolution.xy) * 2.0 - 1.0;
    uv.x *= aspect;
	uv.y *= -1.0;
    
    // Define field of view (in radians)
    float fov = radians(camData.data4.x); // Example 90-degree FOV
    float z = 1.0 / tan(fov * 0.5); // Adjust based on FOV

    // Ray direction for the current pixel
    vec3 rd = normalize(vec3(uv.xy, z));
    
    // Rotate the ray direction based on camera orientation
    rd = rotateVec3ByYawPitchRoll(rd, camData.camera_rot.x, camData.camera_rot.y, camData.camera_rot.z);
    
    // Set the ray origin as the camera position
    vec3 ro = camData.camera_pos;

    // Perform ray marching or tracing with the computed ray direction
    vec3 shaded_color = ray_march_iter(ro, rd, uv);
    
    // Output the final color
    outColor = vec4(shaded_color, 1.0);
}