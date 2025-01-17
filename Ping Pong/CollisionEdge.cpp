#include "CollisionEdge.hpp"
double CollisionEdge::distance(sf::Vector2f point) const {
	double dy = end.y - start.y;
	double dx = end.x - start.x;
	// horizontal
	if (dy == 0) return abs(point.y - start.y);
	// vertical
	if (dx == 0) return abs(point.x - start.x);
	double slope = dy / dx;
	double offset = start.y - slope * start.x;



	/*if (this.start.x == this.d.x) {
		return Math.abs(point.x - this.d.x)
	}
	if (this.start.y == this.d.y) {
		return Math.abs(point.y - this.d.y)
	}

	const slope = (this.d.y - this.start.y) / (this.d.x - this.start.x)
		const b = -(slope * this.start.x) + this.start.y*/

	return abs(dx * point.y - dy * point.x - (dx)*offset) / sqrt(dx * dx + dy * dy);
}
sf::Vector2f CollisionEdge::ClosestPoint(sf::Vector2f point) const {
	sf::Vector2f H = { 0,0 };
	double dy = end.y - start.y;
	double dx = end.x - start.x;
	if (dy == 0) {
		H.x = point.x;
		H.y = start.y;
	}
	else if (dx == 0) {
		H.y = point.y;
		H.x = start.x;
	}
	else {
		double slope = dy / dx;
		double verticalSlope = -1 / slope;
		double b = -(slope * start.x) + start.y;
		double verticalB = -(verticalSlope * point.x) + point.y;

		H.x = (b - verticalB) / (verticalSlope - slope);
		H.y = verticalB + verticalSlope * H.x;
	}
	if (!IsBetween(H.x, start.x, end.x) || !IsBetween(H.y, start.y, end.y)) H = { NAN,NAN };
	return H;
}
std::pair<sf::Vector2f, sf::Vector2f> CollisionEdge::CollisionPoints(sf::Vector2f point, double radius) const {
	std::pair<sf::Vector2f, sf::Vector2f> xy;
	double dy = end.y - start.y;
	double dx = end.x - start.x;
	if (dx == 0) {
		// x = start.x
		// (y - y0) = +-sqrt(r^2 -start.x^2)
		auto d = start.x - point.x;
		auto s = sqrt(radius * radius - d * d);
		sf::Vector2f canditates = sf::Vector2f(point.y + s, point.y - s);
		xy = {
			{start.x,canditates.x},
			{start.x,canditates.y}
		};
	}
	else {
		double a = dy / dx;
		double b = start.y - a * start.x;
		double x0 = point.x;
		double y0 = point.y;
		double r = radius;
		double z = b - y0;
		// (x-x0)^2 + (y - y0)^2 = r^2		
		// y = ax + b
		// (x-x0)^2 + (ax + b - y0)^2 = r^2	
		// 	x^2 + x0^2 + a^2x^2 + 2ax(b-y0) - 2x0x + (b-y0)^2 - r^2 = 0
		// x^2 - 2xx0 + x0^2 +  a^2x^2 + 2ax(b-y0) + (b-y0)^2 = r^2
		// a = a^2 + 1
		// b = 2(b-y0 - x0)
		// c = (b-y0)^2 + x0^2 - r^2
		auto candidates = Quadratic(a * a + 1, 2 * (a * z - x0), z * z + x0 * x0 - r * r);
		xy = {
			sf::Vector2f(candidates.first,candidates.first * a + b),
			sf::Vector2f(candidates.second,candidates.second * a + b)
		};
	}
	if (!IsBetween(xy.first.x, start.x, end.x) || !IsBetween(xy.first.y, start.y, end.y)) xy.first = { NAN,NAN };
	if (!IsBetween(xy.second.x, start.x, end.x) || !IsBetween(xy.second.y, start.y, end.y)) xy.second = { NAN,NAN };

	return xy;


}
double CollisionEdge::GetAngle() const {
	double dy = end.y - start.y;
	double dx = end.x - start.x;
	return atan2(dy, dx);
}
double CollisionEdge::GetLength() const {
	return Distance(start, end);
}
CollisionEdge CollisionEdge::Rotate(double angle, sf::Vector2f centre) const {
	double s = sin(angle);
	double c = cos(angle);
	auto dStart = start - centre;
	auto dEnd = end - centre;
	// sin a + b = y cos b + x sin b
	// cos a + b = x cos b - sin a y
	sf::Vector2f startprime(dStart.x * c - dStart.y * s, dStart.y * c + dStart.x * s);
	sf::Vector2f endprime(dEnd.x * c - dEnd.y * s, dEnd.y * c + dEnd.x * s);
	startprime += centre;
	endprime += centre;
	return CollisionEdge(startprime, endprime);
}
CollisionEdge::CollisionEdge(sf::Vector2f start, sf::Vector2f end) : start(start), end(end) {

}
CollisionEdge::CollisionEdge(sf::Vector2f start, double length, double angle) : start(start) {
	end.x = length * cos(angle);
	end.y = length * sin(angle);
	end += start;
}
std::pair<double, double> Quadratic(double a, double b, double c) {
	double delta = sqrt(b * b - 4 * a * c);
	double x1 = (-b + delta) / 2 / a;
	double x2 = (-b - delta) / 2 / a;
	return { x1,x2 };
}
bool IsBetween(double val, double left, double right) {
	if (left > right) std::swap(left, right);
	return left <= val && val <= right;
}
double Distance(sf::Vector2f v1, sf::Vector2f v2) {
	double dy = v2.y - v1.y;
	double dx = v2.x - v1.x;
	return sqrt(dy * dy + dx * dx);
}