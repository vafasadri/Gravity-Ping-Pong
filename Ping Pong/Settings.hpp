#pragma once
int difficulty = 1;
constexpr double FPS = 60;
constexpr double FrameDuration = 1000 / FPS;
constexpr double GravityMagnitude = 0.1;
constexpr double GravityRateOfChange = 0.002;
double OpponentSpeed() {
	return 5 + 0.3 * (difficulty - 3);
}
constexpr double PlayerSpeed = 5;
constexpr double BallSpeed = 0;
sf::Vector2f groundCollisionElasticity{ 1,-1 };
sf::Vector2f wallCollisionElasticity{ -1,1 };
sf::FloatRect screenBox(200, 0, 700, 500);
sf::Vector2f Gravity{ 0,0.3 };
double GravityAngle = -0.5 * 3.14;
enum class ControlMode {
	Keyboard,Mouse
};
auto controlMode = ControlMode::Mouse;
