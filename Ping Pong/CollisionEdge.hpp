#pragma once
#include <SFML/Graphics.hpp>
struct CollisionEdge {
	mutable bool colliding = false;
	sf::Vector2f start{};
	sf::Vector2f end{};
	double distance(sf::Vector2f point) const;
	sf::Vector2f ClosestPoint(sf::Vector2f point) const;
	std::pair<sf::Vector2f, sf::Vector2f> CollisionPoints(sf::Vector2f point, double radius) const;
	double GetAngle() const;
	double GetLength() const;
	CollisionEdge Rotate(double angle, sf::Vector2f centre) const;
	CollisionEdge(sf::Vector2f start, sf::Vector2f end);
	CollisionEdge(sf::Vector2f start, double length, double angle);
	CollisionEdge() = default;
};

std::pair<double, double> Quadratic(double a, double b, double c); 
bool IsBetween(double val, double left, double right);
double Distance(sf::Vector2f v1, sf::Vector2f v2);