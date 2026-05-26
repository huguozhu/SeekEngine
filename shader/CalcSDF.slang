module CalcSDF;


/* ***************************************************************
 * ******************** 2D Basic Shape SDFs **********************
 * ***************************************************************/
// 1. Circle SDF
public float sdCircle(float2 p, float r) {
    return length(p) - r;
}

// 2. Rectangle SDF
// b: rectangle half-dimensions (half-width, half-height)
public float sdRectangle(float2 p, float2 b) {
    public float2 d = abs(p) - b;
    // max(d.x, d.y) handles exterior distance, min(..., 0.0) handles interior distance
    return length(max(d, 0.0)) + min(max(d.x, d.y), 0.0);
}

// 3. Rounded Rectangle SDF
// b: half-dimensions, r: corner radius
public float sdRoundedRectangle(float2 p, float2 b, float r) {
    public float2 d = abs(p) - b + r;
    return length(max(d, 0.0)) - r + min(max(d.x, d.y), 0.0);
}

// 4. Line Segment SDF
// a: start point, b: end point, r: line width (radius)
public float sdSegment(float2 p, float2 a, float2 b, float r) {
    public float2 pa = p - a;
    public float2 ba = b - a;
    public float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0); // Calculate projection ratio onto segment
    public float2 closestPoint = a + ba * h; // Find closest point on segment
    return length(p - closestPoint) - r; // Calculate distance to closest point minus line width
}

// 5. Ellipse SDF (approximation)
// r: ellipse semi-axes (x-radius, y-radius)
public float sdEllipse(float2 p, float2 r) {
    // This implementation is an approximation but good enough for most cases
    public float k0 = length(p / r);
    public float k1 = length(p / (r * r));
    return k0 * (k0 - 1.0) / k1;
}

// 6. Triangle SDF
// a,b,c: Triangle's three points
public float sdTriangle(float2 p, float2 a, float2 b, float2 c) {
    public float2 e0 = b - a;
    public float2 e1 = c - b;
    public float2 e2 = a - c;
    
    public float2 v0 = p - a;
    public float2 v1 = p - b;
    public float2 v2 = p - c;
    
    public float2 pq0 = v0 - e0 * clamp(dot(v0, e0) / dot(e0, e0), 0.0, 1.0);
    public float2 pq1 = v1 - e1 * clamp(dot(v1, e1) / dot(e1, e1), 0.0, 1.0);
    public float2 pq2 = v2 - e2 * clamp(dot(v2, e2) / dot(e2, e2), 0.0, 1.0);
    
    public float s = sign(e0.x * e2.y - e0.y * e2.x);
    public float2 d = min(min(
        float2(dot(pq0, pq0), s * (v0.x * e0.y - v0.y * e0.x)),
        float2(dot(pq1, pq1), s * (v1.x * e1.y - v1.y * e1.x))),
        float2(dot(pq2, pq2), s * (v2.x * e2.y - v2.y * e2.x)));
        
    return -sqrt(d.x) * sign(d.y);
}
// 
// 7. Superellipse SDF
// n: shape power [0, 10]
//		<1: Cross Shaped --> Diamond shaped
//		=1: Diamond shape
//		=2: Circle
//		>2: RoundBox(RoundRectangle) --> Box(Rectangle)
public float sdSuperellipse(float2 p, float n, float r) {
    // Take the absolute value of the coordinates, as the formula uses |x| and |y|.
    // This makes the shape symmetrical in all quadrants.
    public float2 p_abs = abs(p);

    // Numerator of the distance formula: |x|^n + |y|^n - r^n
    // This is the implicit equation of the superellipse. Its value is 0 on the
    // boundary, negative inside, and positive outside.
    public float numerator = pow(p_abs.x, n) + pow(p_abs.y, n) - pow(r, n);

    // Denominator: n * sqrt(|x|^(2n-2) + |y|^(2n-2))
    // This is the magnitude of the gradient of the implicit function. Dividing by it
    // normalizes the result, making it a better approximation of true Euclidean distance.
    // Note: The pow() function can be computationally expensive. For a fixed, integer 'n'
    // (like n=4), you would get better performance by using direct multiplication.
    public float den_x = pow(p_abs.x, 2.0 * n - 2.0);
    public float den_y = pow(p_abs.y, 2.0 * n - 2.0);

    // Add a small epsilon to prevent division by zero at the origin (0,0).
    public float denominator = n * sqrt(den_x + den_y) + 0.00001;

    // The final signed distance
    return numerator / denominator;
}
/* ***************************************************************
 * ******************** 3D Basic Shape SDFs **********************
 * ***************************************************************/
// 1. Sphere SDF
public float sdSphere(float3 p, float r) {
    return length(p) - r;
}

// 2. Box SDF
// b: box half-dimensions (half-width, half-height, half-depth)
public float sdBox(float3 p, float3 b) {
    public float3 d = abs(p) - b;
    // Exterior distance with length(max(d, 0.0))
    // Interior distance with min(max(d.x, max(d.y, d.z)), 0.0)
    return length(max(d, 0.0)) + min(max(d.x, max(d.y, d.z)), 0.0);
}

// 3. Rounded Box SDF
public float sdRoundBox(float3 p, float3 b, float r) {
    public float3 d = abs(p) - b + r;
    return length(max(d, 0.0)) - r + min(max(d.x, max(d.y, d.z)), 0.0);
}

// 4. Cylinder SDF
// h: half height of cylinder, r: base radius
public float sdCylinder(float3 p, float h, float r) {
    public float2 d = abs(float2(length(p.xz), p.y)) - float2(r, h);
    return min(max(d.x, d.y), 0.0) + length(max(d, 0.0));
}

// 5. Infinite Plane SDF
// n: normal vector (should be normalized), o: signed distance from origin to plane
public float sdPlane(float3 p, float3 n, float o) {
    return dot(p, normalize(n)) - o;
}

// 6. Torus SDF
// t: major and minor radii (t.x: major radius, t.y: tube radius)
public float sdTorus(float3 p, float2 t) {
    public float2 q = float2(length(p.xz) - t.x, p.y);
    return length(q) - t.y;
}

// 7. Cone SDF
// c: cone size parameters (c.x = height, c.y = base radius)
public float sdCone(float3 p, float2 c) {
    // c.x is height, c.y is base radius
    public float2 q = float2(length(p.xz), p.y);
    
    public float d1 = -p.y - c.x;
    public float d2 = max(dot(q, float2(c.y, c.x)) / length(float2(c.y, c.x)), -p.y);
    
    return length(max(float2(d1, d2), 0.0)) + min(max(d1, d2), 0.0);
}

/* ***************************************************************
 * ****************** SDF Boolean Operations *********************
 * ***************************************************************/
// 1. Union
public float sdopUnion(float d1, float d2) {
    return min(d1, d2);
}

// 2. Intersection
public float sdopIntersection(float d1, float d2) {
    return max(d1, d2);
}

// 3. Subtraction - subtract d2 from d1
public float sdopSubtraction(float d1, float d2) {
    return max(d1, -d2);
}

// 4. Smooth Union
// k:[0,100] smoothness, k=0:no smooth  k=100:very smooth
public float sdopSmoothUnion(float d1, float d2, float k) {
    public float h = clamp(0.5 + 0.5 * (d2 - d1) / k, 0.0, 1.0);
    return lerp(d2, d1, h) - k * h * (1.0 - h);
}

// 5. Smooth Intersection
// k:[0,100] smoothness, k=0:no smooth  k=100:very smooth
public float sdopSmoothIntersection(float d1, float d2, float k) {
    public float h = clamp(0.5 - 0.5 * (d2 - d1) / k, 0.0, 1.0);
    return lerp(d2, d1, h) + k * h * (1.0 - h);
}

// 6. Smooth Subtraction
// k:[0,100] smoothness, k=0:no smooth  k=100:very smooth
public float sdopSmoothSubtraction(float d1, float d2, float k) {
    public float h = clamp(0.5 - 0.5 * (d2 + d1) / k, 0.0, 1.0);
    return lerp(d2, -d1, h) + k * h * (1.0 - h);
}
