#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <chrono>
#include <random>
#include <iostream>
#include <map>
#include <algorithm>
#include "Settings.hpp"
void Reset();
void ManageOpponent();
std::pair<double, double> Quadratic(double a,double b,double c){
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
	return sqrt(dy * dy - dx * dx);
}

struct CollisionEdge {
	mutable bool colliding = false;
	sf::Vector2f start{};
	sf::Vector2f end{};
	double distance(sf::Vector2f point) const {
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
	sf::Vector2f ClosestPoint(sf::Vector2f point) const {
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
	std::pair<sf::Vector2f,sf::Vector2f> CollisionPoints(sf::Vector2f point,double radius) const {
		
		double dy = end.y - start.y;
		double dx = end.x - start.x;
		
		double a = dy / dx;
		double b = start.y - a * start.x;
		double x0 = point.x;
		double y0 = point.y;
		double r = radius;
		double z = b - y0;
		// (x - x0)2 + (ax+ (b - y0))2 = radius2
			// y = slope * x + b
		// x^2 - 2xx0 + x0^2 +  a^2x^2 + 2ax(b-y0) + (b-y0)^2 = r^2
		// a = a^2 + 1
		// b = 2(b-y0 - x0)
		// c = (b-y0)^2 + x0^2 - r^2
		auto candidates = Quadratic(a * a + 1, 2 * (z - x0), z * z + x0 * x0 - r * r);


		std::pair<sf::Vector2f, sf::Vector2f> xy = {
			sf::Vector2f(candidates.first,candidates.first * a + b),
			sf::Vector2f(candidates.second,candidates.second * a + b)
		};

		if (!IsBetween(xy.first.x, start.x, end.x) || !IsBetween(xy.first.y, start.y, end.y)) xy.first = { NAN,NAN };
		if (!IsBetween(xy.second.x, start.x, end.x) || !IsBetween(xy.second.y, start.y, end.y)) xy.second = { NAN,NAN };

		return xy;
			
		
	}
	double GetAngle() const {
		double dy = end.y- start.y;
		double dx = end.x - start.x;
		return atan2(dy, dx);
	}
	double GetLength() const {
		return Distance(start, end);
	}
	CollisionEdge Rotate(double angle, sf::Vector2f centre) {
		double s = sin(angle);
		double c = cos(angle);
		auto dStart = start - centre;
		auto dEnd = end - centre;
		// sin a + b = y cos b + x sin b
		// cos a + b = x cos b - sin a y
		sf::Vector2f startprime(dStart.x * c - dStart.y * s, dStart.y * c + dStart.x*s);
		sf::Vector2f endprime(dEnd.x * c - dEnd.y * s, dEnd.y * c + dEnd.x*s);
		startprime += centre;
		endprime += centre;
		return CollisionEdge(startprime, endprime);
	}
	CollisionEdge(sf::Vector2f start, sf::Vector2f end) : start(start), end(end) {

	}
	CollisionEdge(sf::Vector2f start, double length, double angle) : start(start) {
		end.x = length * cos(angle);
		end.y = length * sin(angle);
		end += start;
	}

};
std::vector<CollisionEdge>* ScreenEdges = new std::vector<CollisionEdge>{
	{{screenBox.left,screenBox.top},{screenBox.left + screenBox.width,screenBox.top}},
	{{screenBox.left,screenBox.top+screenBox.height},{screenBox.left + screenBox.width,screenBox.top + screenBox.height}}
};

void GetEdges(sf::FloatRect box, std::vector<CollisionEdge>& container) {

	auto getpoint = [&](int i) -> sf::Vector2f {
		i %= 4;
		return sf::Vector2f((i == 1 || i == 2) * box.width + box.left, (i >= 2) * box.height + box.top);
		};
	for (int i = 0; i < 4; i++)
	{
		if(container.size() <= i) 
		container.push_back(CollisionEdge(getpoint(i), getpoint(i + 1)));
		else {
			container[i].start = getpoint(i);
			container[i].end = getpoint(i + 1);
		}
	}
}
std::map<int, long long> leaderboard;
static sf::Texture get_texture(int resource);
static sf::Texture get_texture(std::string file);
void draw_text(const bool i_center_x, const bool i_center_y, const short i_x, const short i_y, const std::string& i_text, sf::RenderTarget& i_window, const sf::Texture& i_font_texture, const short i_font_size);
extern sf::Texture font_texture;
bool OpponentUp = false;
bool OpponentDown = false;
bool PlayerUp = false;
bool PlayerDown = false;
sf::RectangleShape compass;

bool started = false;
int framesPassed = 0;

double magnitude(sf::Vector2f vec) {
	return sqrt(vec.x * vec.x + vec.y * vec.y);
}
using EdgeSet = std::vector<const std::vector<CollisionEdge>*>;
class  Entity
{
public:
	std::vector<CollisionEdge>* edges = nullptr;
	sf::Vector2f velocity{};
	sf::Vector2f acceleration{};
	sf::Vector2f position{};
	virtual void PreUpdate() {

	}
	virtual void Update(std::vector<Entity*>& entities,const EdgeSet& collisionEdges) {

	}
	virtual void UpdateEdges() {
	}
	virtual void Draw(sf::RenderTarget& target) = 0;
	Entity() {
	}
	virtual ~Entity() {

	};

private:

};


class Ball : public Entity{
	sf::CircleShape shape;
public:
	double radius;
	void PreUpdate() override {
	}
	void Update(std::vector<Entity*>& entities, const EdgeSet& collisionEdges) override {
		if (position.x > screenBox.left + screenBox.width) {

			if (!leaderboard.contains(difficulty)) {
				leaderboard.insert({ difficulty, framesPassed });

			}
			else {
				leaderboard[difficulty] = std::min<long long>(framesPassed, leaderboard[difficulty]);
			}
			Reset();
			framesPassed = 0;
			difficulty++;
	
			return;
		}
		if (position.x < screenBox.left) {
			Reset();
			return;
		}
		for (auto& set : collisionEdges)
		{
			if (set == nullptr) continue;
			for (auto& i : *set)
			{

				double d = i.distance(this->position + sf::Vector2f(radius, radius));
				if (d > radius) {
					i.colliding = false;
					continue;
				}
				auto H = i.ClosestPoint(this->position + sf::Vector2f(radius, radius));
				if (isfinite(H.x) && isfinite(H.y)) {

					if (i.colliding == false) {
						double thisAngle = atan2(velocity.y, velocity.x);
						double edgeAngle = i.GetAngle();
						
						double v = magnitude(velocity);
						double finalAngle = 2 * edgeAngle - thisAngle;
						velocity = sf::Vector2f(v * cos(finalAngle), v * sin(finalAngle));
					}
					i.colliding = true;
				}
				else i.colliding = false;
			}
		}
	}
	void Draw(sf::RenderTarget& target) override {
		shape.setPosition(position);
		target.draw(shape);
	}
	Ball() {
		radius = 10;
		shape.setRadius(radius);
		shape.setFillColor(sf::Color::White);

	}
};
class Player : public Entity {
	sf::RectangleShape shape;
public:
	sf::Vector2f size;
	void PreUpdate() override {
		if (PlayerUp) velocity = { 0,-PlayerSpeed };
		else if (PlayerDown) velocity = { 0,PlayerSpeed };
		else velocity = { 0 , 0 };
	}
	void Update(std::vector<Entity*>& entities, const EdgeSet& collisionEdges) override {
		
		position.y = std::clamp(position.y, screenBox.top, screenBox.top + screenBox.height - shape.getSize().y);

	}
	void Draw(sf::RenderTarget& target) {
		shape.setPosition(position);
		shape.setSize(size);
		target.draw(shape);
	}
	void UpdateEdges() override{
		GetEdges({ position,size }, *edges);
	}
	Player() {
		edges = new std::vector<CollisionEdge>;
		size = { 25, 100 };
		shape.setFillColor(sf::Color::White);

	}
};
class Opponent : public Entity {
	sf::RectangleShape shape;
public:
	sf::Vector2f size;
	void PreUpdate() override {
		ManageOpponent();
		if (OpponentUp) velocity = { 0,(float)-OpponentSpeed() };
		else if (OpponentDown) velocity = { 0,(float)OpponentSpeed() };
		else velocity = { 0 , 0 };
	}
	void Update(std::vector<Entity*>& entities, const EdgeSet& collisionEdges) override {
		position.y = std::clamp(position.y, screenBox.top, screenBox.top + screenBox.height - shape.getSize().y);
	}
	void Draw(sf::RenderTarget& target) override {
		shape.setPosition(position);
		shape.setSize(size);

		target.draw(shape);
	}
	void UpdateEdges() override {
		GetEdges({ position,size }, *edges);
	}
	Opponent() {
		size = { 25,100 };

		edges = new std::vector<CollisionEdge>;
	}
};
class Mockball : public Entity {
public:
	double radius;
	Mockball(Ball* original) {
		radius = original->radius;
		position = original->position;
		acceleration = original->acceleration;
		velocity = original->velocity;
	}
	void Draw(sf::RenderTarget& target) {
	}
};
class MockPlayer : public Entity {
	bool up;
	bool down;
public:
	sf::Vector2f size;
	void PreUpdate() override {
		if (difficulty <= 0) {
			velocity = { 0,0 };
			return;
		}
		if (up) velocity = { 0,-PlayerSpeed };
		if (down) velocity = { 0,PlayerSpeed };
	}
	void Update(std::vector<Entity*>& entities, const EdgeSet& collisionEdges) override {
		position.y = std::clamp<float>(position.y, screenBox.top, screenBox.top + screenBox.height - size.y);
	}
	void Draw(sf::RenderTarget& target) {
	}
	void UpdateEdges() override {
		
		GetEdges({ position,size }, *edges);
	}
	MockPlayer(Player* original,bool up,bool down) : up(up),down(down) {
		size = original->size;
		position = original->position;
		acceleration = original->acceleration;
		velocity = original->velocity;
		edges = new std::vector<CollisionEdge>;
	}
};
Ball* ball = new Ball;
Opponent* opponent = new Opponent;
Player* player = new Player;

std::vector<Entity*> entities = { ball,opponent,player };

EdgeSet collisionEdges = {
	entities[0]->edges,entities[1]->edges,entities[2]->edges,ScreenEdges
};

void Reset() {
	opponent->position = { 
		screenBox.left + screenBox.width - 50 - opponent->size.x,
		screenBox.top + screenBox.height / 2 - opponent->size.y / 2 
	};
	ball->position = sf::Vector2f(
		screenBox.left + screenBox.width / 2 - ball->radius,
		screenBox.top + screenBox.height / 2 - ball->radius
	);
	player->size = sf::Vector2f( 25,150 + std::max(0,5 * (5 - difficulty)) );
	player->position = { screenBox.left + 50, 250 - player->size.y / 2 };
	ball->velocity = { 0,0 };
	ball->acceleration = { 0,0 };
	started = false;
}

void Tick(std::vector<Entity*>& entities,EdgeSet& collisionEdges) {
	
	std::vector<double > magnitudes;
	for (auto i : entities)
	{
		i->PreUpdate();
		magnitudes.push_back(magnitude(i->velocity));
		magnitudes.push_back(magnitude(i->acceleration));
	}
	double maxlength = *std::max_element(magnitudes.begin(),magnitudes.end());	
	int times = std::max(1., ceil(maxlength / 10));
	int n = 0;
	while(n++ < times) {

		for (auto i : entities)
		{
			i->velocity += i->acceleration * (1.f / times);
			i->position += i->velocity * (1.f / times);
		}
		for (auto i : entities)
		{
			i->UpdateEdges();
		}
		for (auto i : entities)
		{
			i->Update(entities,collisionEdges);
		}	
	}
	
}
std::vector<CollisionEdge>* playerWall = new std::vector<CollisionEdge>{
	{{screenBox.left + 75,screenBox.top},{screenBox.left + 75,screenBox.top + screenBox.height}}
};
sf::Vector2f PredictTrajectory() {
	int ticks = 0;
	bool playerUp = sf::Keyboard::isKeyPressed(sf::Keyboard::W);
	bool playerDown = sf::Keyboard::isKeyPressed(sf::Keyboard::S);
	double GravityAngle = ::GravityAngle;
	auto Gravity = ::Gravity;
	bool gravityLeft = sf::Keyboard::isKeyPressed(sf::Keyboard::Left);
	bool gravityRight = sf::Keyboard::isKeyPressed(sf::Keyboard::Right);
	std::vector<Entity*> entities = { new Mockball(ball),new MockPlayer(player,playerUp,playerDown) };
	EdgeSet collisionEdges = {
	(difficulty > 4) ? entities[1]->edges : nullptr,
	(difficulty > 3) ? ScreenEdges : nullptr,
	(difficulty > 7) ? playerWall : nullptr
	};
	auto ball = (Mockball*)entities[0];
	while (ball->position.x + 2 * ball->radius < opponent->position.x) {

		if (ticks > 1000) break;
		if (difficulty > 6 && controlMode == ControlMode::Keyboard) {
			if (gravityLeft) {
				GravityAngle += GravityRateOfChange;
				Gravity = { float(cos(GravityAngle) * GravityMagnitude),float(-sin(GravityAngle) * GravityMagnitude) };
			}
			if (gravityRight) {
				GravityAngle -= GravityRateOfChange;
				Gravity = { float(cos(GravityAngle) * GravityMagnitude),float(-sin(GravityAngle) * GravityMagnitude) };

			}
		}
		ball->acceleration = Gravity;
		Tick(entities, collisionEdges);
		ticks++;
	}
	auto pos = ball->position;
	for (auto& i : entities)
	{
		delete i;
	}
	return pos;
}

void ManageOpponent() {
	OpponentUp = false;
	OpponentDown = false;
	auto ballPos = (difficulty > 2) ? PredictTrajectory() : ball->position;
	auto opponentPos = opponent->position;
	opponentPos.y += opponent->size.y / 2;
	if (ballPos.y < opponentPos.y + OpponentSpeed()) OpponentUp = true;
	if (ballPos.y > opponentPos.y - OpponentSpeed()) OpponentDown = true;
}



std::string TimeFormat(long long seconds) {
	long long s = seconds % 60;
	long long m = (seconds / 60) % 60;
	long long h = seconds / 60 / 60;
	std::string sstr = std::to_string(s);
	std::string mstr = std::to_string(m);
	std::string hstr = std::to_string(h);
	if (sstr.length() < 2) sstr = "0" + sstr;
	if (mstr.length() < 2) mstr = "0" + mstr;
	if (hstr.length() < 2) hstr = "0" + hstr;
	
	
	std::string result;
	if (h) {
		result += hstr + ":";
	}
	result += mstr + ":" + sstr;
	return result;
}

void DrawGame(sf::RenderTarget& target) {
	target.clear();
	for (auto& i : entities)
	{
		i->Draw(target);
	}
	target.draw(compass);
	sf::RectangleShape line({ 5.f,float(target.getSize().y) });
	line.setFillColor(sf::Color::Magenta);
	line.setPosition(screenBox.left - line.getSize().x, 0);
	target.draw(line);
	draw_text(false, false, 10, 10, "Difficulty:", target, font_texture, 22);
	draw_text(false, false, 10, 32, std::to_string(difficulty), target, font_texture, 22);
	draw_text(false, false, 10, 54, "Time: ", target, font_texture, 22);
	long long ff = (long long)framesPassed / 60;
	draw_text(false, false, 10, 76, TimeFormat(ff), target, font_texture, 22);
	int y = 98;
	for (auto& i : leaderboard)
	{
		draw_text(false, false, 10, y, 
			"Level " + std::to_string(i.first) + " : " + TimeFormat(i.second / 60),
			target,
			font_texture,
			16);

		y += 16;
	}
}

int main() {
	
	sf::RenderWindow window(sf::VideoMode(screenBox.left + screenBox.width, screenBox.top + screenBox.height), "Ping Pong");
	std::chrono::steady_clock clock;
	auto lasttime = clock.now();
	
	compass.setSize({ 50,5 });
	compass.setFillColor(sf::Color::Red);
	compass.setPosition(screenBox.getPosition() + (0.5f * screenBox.getSize()));
	Reset();


	while (window.isOpen())
	{
		sf::Event event;
		if (window.pollEvent(event)) {
			switch (event.type)
			{
			case sf::Event::Closed:
				window.close();
				return 0;
			default:
				break;
			}

		}
		if (!started && sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
			started = true;
		}
		if (controlMode == ControlMode::Keyboard) {
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
				GravityAngle += GravityRateOfChange;
				Gravity = { float(cos(GravityAngle) * GravityMagnitude),float(-sin(GravityAngle) * GravityMagnitude) };
				ball->acceleration = Gravity;
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
				GravityAngle -= GravityRateOfChange;
				Gravity = { float(cos(GravityAngle) * GravityMagnitude),float(-sin(GravityAngle) * GravityMagnitude) };
				ball->acceleration = Gravity;
			}
		}
		else {
			auto position = sf::Mouse::getPosition(window);
			sf::Vector2f positionF(position.x, position.y);
			//std::cout << positionF.x << "   " << positionF.y << std::endl;
			sf::Vector2f center = compass.getPosition();// { screenBox.left + screenBox.width, screenBox.top + screenBox.height / 2 };
			auto delta = positionF - center;
			delta.y *= -1;
			GravityAngle = atan2(delta.y, delta.x);
			Gravity = { float(cos(GravityAngle) * GravityMagnitude),float(-sin(GravityAngle) * GravityMagnitude) };
			ball->acceleration = Gravity;
		}

		auto thisTime = clock.now();
		auto duration = thisTime - lasttime;
		if (duration.count() >= FrameDuration * 1000000) {
			lasttime = thisTime;
			compass.setRotation(-GravityAngle * 180 / 3.14);
			PlayerUp = sf::Keyboard::isKeyPressed(sf::Keyboard::W);
			PlayerDown = sf::Keyboard::isKeyPressed(sf::Keyboard::S);
			if(started) Tick(entities,collisionEdges);
			framesPassed++;
		}
		DrawGame(window);
		window.display();

	}


}