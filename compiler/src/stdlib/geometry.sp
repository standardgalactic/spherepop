/* geometry.sp — Geometric utilities. */
fn circle_area(r) { return 3.14159265358979 * r * r; }
fn sphere_volume(r) { return (4.0 / 3.0) * 3.14159265358979 * r * r * r; }
fn distance(x1, y1, x2, y2) {
    let dx = x2 - x1;
    let dy = y2 - y1;
    return sqrt(dx * dx + dy * dy);
}
