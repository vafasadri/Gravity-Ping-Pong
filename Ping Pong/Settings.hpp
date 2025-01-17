#pragma once
#include "Global.hpp"
constexpr double FPS = 60;
constexpr double FrameDuration = 1000 / FPS;
constexpr double PlayerSpeed = 5;
constexpr double BallSpeed = 0;
sf::FloatRect ScreenBox(200, 0, 900, 500);
sf::Vector2f ScreenCenter() {
	return ScreenBox.getPosition() + (0.5f* ScreenBox.getSize());
}
class Gravity {
	static double angle;
	static sf::Vector2f vector;
public:
	static constexpr double Magnitude = 0.1;
	static constexpr double RateOfChange = 0.002;
	static sf::Vector2f Vector() {
		return vector;
	};
	static void SetAngle(double _angle) {
		angle = _angle;
		vector = sf::Vector2f( cosf(angle),-sinf(angle) ) * float(Magnitude);
	}
	static double Angle() {
		return angle;
	}
};
double Gravity::angle = -0.5 * 3.14 ; // 270 degrees
sf::Vector2f Gravity::vector = { 0,Gravity::Magnitude };
constexpr long long PredictionTicks = 1000;
namespace Controls
{
	enum class ControlMode {
		Keyboard, Mouse,Scroll
	};
	sf::Keyboard::Key Player_Up = sf::Keyboard::LBracket;
	sf::Keyboard::Key Player_Down = sf::Keyboard::Quote;
	sf::Keyboard::Key Gravity_Clockwise = sf::Keyboard::Right;
	sf::Keyboard::Key Gravity_CounterClockwise = sf::Keyboard::Left;
	constexpr double Gravity_Scroll_Sensitivity = 0.5;
	constexpr double Player_Scroll_Sensitivity = 1;
	auto Gravity_Mode = ControlMode::Mouse;
	auto Player_Mode = ControlMode::Keyboard;
}
namespace Difficulty {
	int Current = 10;
	constexpr int Prediction = 3;
	constexpr int BorderCollisions = 4;
	constexpr int PlayerCollisions = 5;
	constexpr int PlayerTracking = 6;
	constexpr int GravityTracking = 7;
	constexpr int PlayerWall = 8;
	constexpr int VortexTracking = 9;
	constexpr int Fallback = 10;
}
double OpponentSpeed() {
	//return 5 + 0.3 * (Difficulty::Current - 3);
	return 4;
}
double PlayerHeight() {
	return 150 + std::max(0, 5 * (5 - Difficulty::Current));
}
constexpr double VortexAngularVelocity = 0.01;
constexpr bool DrawEdges = false;
namespace Compass {
	sf::Color Color = sf::Color::Red;
	sf::Vector2f Size = { 50,5 };
}
namespace Engine {
	double SegmentLength = 10;
}

