#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <chrono>
#include <random>
#include <iostream>
#include <map>
#include <algorithm>
#include "Settings.hpp"
#include "Global.hpp"
void Reset();
void ManageOpponent();


std::vector<CollisionEdge>* ScreenEdges = new std::vector<CollisionEdge>{
	{{ScreenBox.left,ScreenBox.top},{ScreenBox.left + ScreenBox.width,ScreenBox.top}},
	{{ScreenBox.left,ScreenBox.top + ScreenBox.height},{ScreenBox.left + ScreenBox.width,ScreenBox.top + ScreenBox.height}}
};
std::vector<CollisionEdge>* MockScreenEdges = new std::vector<CollisionEdge>{
	{{ScreenBox.left,ScreenBox.top},{ScreenBox.left + ScreenBox.width,ScreenBox.top}},
	{{ScreenBox.left,ScreenBox.top + ScreenBox.height},{ScreenBox.left + ScreenBox.width,ScreenBox.top + ScreenBox.height}}
};
void GetEdges(sf::FloatRect box, std::vector<CollisionEdge>& container) {

	auto getpoint = [&](int i) -> sf::Vector2f {
		i %= 4;
		return sf::Vector2f((i == 1 || i == 2) * box.width + box.left, (i >= 2) * box.height + box.top);
		};
	for (int i = 0; i < 4; i++)
	{
		if (container.size() <= i)
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

bool running = false;
int framesPassed = 0;

double magnitude(sf::Vector2f vec) {
	return sqrt(vec.x * vec.x + vec.y * vec.y);
}
using EdgeSet = std::vector<const std::vector<CollisionEdge>*>;
class Entity;
struct UpdateData {
	std::vector<Entity*>* entities;
	const EdgeSet* collisionEdges;
	int Segments;
};
class  Entity
{
public:
	std::vector<CollisionEdge>* edges = nullptr;
	sf::Vector2f velocity{};
	sf::Vector2f acceleration{};
	sf::Vector2f position{};
	virtual void PreUpdate() {

	}
	virtual void Update(UpdateData data) {
	}
	virtual void UpdateEdges() {
	}
	virtual void Draw(sf::RenderTarget& target) = 0;
	Entity() {
	}
	virtual ~Entity() {
		if (edges) delete edges;
	};
	virtual Entity* Mock() = 0;
private:

};


class Ball : public Entity {
	sf::CircleShape shape;
public:
	double radius;
	void PreUpdate() override {
	}
	void Update(UpdateData data) override {
		auto centre = position + sf::Vector2f(radius, radius);
		if (position.x > ScreenBox.left + ScreenBox.width) {

			if (!leaderboard.contains(Difficulty::Current)) {
				leaderboard.insert({ Difficulty::Current, framesPassed });

			}
			else {
				leaderboard[Difficulty::Current] = std::min<long long>(framesPassed, leaderboard[Difficulty::Current]);
			}
			Reset();
			framesPassed = 0;
			Difficulty::Current++;
			return;
		}
		if (position.x < ScreenBox.left) {
			Reset();
			return;
		}
		for (auto& set : *data.collisionEdges)
		{
			if (set == nullptr) continue;
			for (auto& i : *set)
			{
				if (Distance(i.start, centre) <= radius || Distance(i.end, centre) <= radius) {

					position -= velocity * (1.f / data.Segments);
					velocity = -velocity;
					return;

				}
				double d = i.distance(centre);
				if (d > radius) {
					continue;
				}
				auto H = i.CollisionPoints(centre, radius);
				if ((isfinite(H.first.x) && isfinite(H.first.y)) || (isfinite(H.second.x) && isfinite(H.second.y))) {


					double thisAngle = atan2(velocity.y, velocity.x);
					double edgeAngle = i.GetAngle();
					double dTheta = thisAngle - edgeAngle;
					
					
					double v = magnitude(velocity);
					double finalAngle = 2 * edgeAngle - thisAngle;
					position -= velocity;
					velocity = sf::Vector2f(v * cos(finalAngle), v * sin(finalAngle));
					return;
				}
				
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
	void Update(UpdateData data) override {

		position.y = std::clamp(position.y, ScreenBox.top, ScreenBox.top + ScreenBox.height - shape.getSize().y);

	}
	void Draw(sf::RenderTarget& target) {
		shape.setPosition(position);
		shape.setSize(size);
		target.draw(shape);
	}
	void UpdateEdges() override {
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
	void Update(UpdateData data) override {
		position.y = std::clamp(position.y, ScreenBox.top, ScreenBox.top + ScreenBox.height - shape.getSize().y);
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
class Wall : public Entity {
	sf::VertexArray vertices;
public:
	void PreUpdate() override {
		ManageOpponent();
		if (OpponentUp) velocity = { 0,(float)-OpponentSpeed() };
		else if (OpponentDown) velocity = { 0,(float)OpponentSpeed() };
		else velocity = { 0 , 0 };
	}
	void Update(UpdateData data) override {
		//position.y = std::clamp(position.y, screenBox.top, screenBox.top + screenBox.height - shape.getSize().y);
	}
	void Draw(sf::RenderTarget& target) override {
		target.draw(vertices);
	}
	void UpdateEdges() override {
		//GetEdges({ position,size }, *edges);
	}
	Wall(std::initializer_list<sf::Vector2f> vertices, sf::Color color) {

		this->vertices.setPrimitiveType(sf::LineStrip);
		edges = new std::vector<CollisionEdge>;
		for (auto iter = vertices.begin(); iter != vertices.end(); iter++)
		{
			this->vertices.append(sf::Vertex(*iter, color));
			if (iter + 1 != vertices.end()) {
				edges->push_back({ *iter,*(iter + 1) });
			}
		}
	}
};
class Vortex : public Entity {
protected:
	sf::VertexArray vertices;
	double radius;
	sf::Vector2f centre;
	std::vector<sf::Vector2f> segments;
	double theta = 0;
	friend class MockVortex;
public:
	
	void PreUpdate() override {

	}
	void Update(UpdateData data) override {
		theta += VortexAngularVelocity / data.Segments;
		//position.y = std::clamp(position.y, screenBox.top, screenBox.top + screenBox.height - shape.getSize().y);
	}
	void Draw(sf::RenderTarget& target) override {
		for (auto& i : segments)
		{
			vertices[0].position = centre + sf::Vector2f(cos(theta + i.x) * radius, sin(theta + i.x) * radius);
			vertices[1].position = centre + sf::Vector2f(cos(theta + i.y) * radius, sin(theta + i.y) * radius);
			target.draw(vertices);
		}
	}
	void UpdateEdges() override {
		for (size_t i = 0; i < segments.size(); i++)
		{
			auto& edge = edges->at(i);
			edge.start = centre + sf::Vector2f(cos(theta + segments[i].x) * radius, sin(theta + segments[i].x) * radius);
			edge.end = centre + sf::Vector2f(cos(theta + segments[i].y) * radius, sin(theta + segments[i].y) * radius);
		}
	}
	Vortex(sf::Vector2f centre, double radius, std::initializer_list<sf::Vector2f> segments, sf::Color color)
		: centre(centre), radius(radius), segments(segments) {

		this->vertices.setPrimitiveType(sf::LineStrip);
		edges = new std::vector<CollisionEdge>;
		edges->resize(segments.size());
		vertices.resize(2);
		vertices[0].color = color;
		vertices[1].color = color;
	}

};
class MockVortex : public Entity {
	double radius = 0;
	sf::Vector2f centre;
	std::vector<sf::Vector2f> segments;
	double theta = 0;
public:
	void PreUpdate() override {

	}
	void Update(UpdateData data) override {
		theta += VortexAngularVelocity / data.Segments;
		//position.y = std::clamp(position.y, screenBox.top, screenBox.top + screenBox.height - shape.getSize().y);
	}
	void Draw(sf::RenderTarget& target) override {
	}
	void UpdateEdges() override {
		for (size_t i = 0; i < segments.size(); i++)
		{
			auto& edge = edges->at(i);
			edge.start = centre + sf::Vector2f(cos(theta + segments[i].x) * radius, sin(theta + segments[i].x) * radius);
			edge.end = centre + sf::Vector2f(cos(theta + segments[i].y) * radius, sin(theta + segments[i].y) * radius);
		}
	}
	void Reset(Vortex* vortex) {

		 radius = vortex->radius;
		 centre = vortex->centre;
		 segments = vortex->segments;
		 theta = vortex->theta;
		 *edges = *vortex->edges;
	}
	MockVortex() {	
		edges = new std::vector<CollisionEdge>;
	}
};
class Mockball : public Entity {
public:
	double radius;
	Mockball() {
	}
	void Reset(Ball* original) {
		radius = original->radius;
		position = original->position;
		acceleration = original->acceleration;
		velocity = original->velocity;
	}
	void Update(UpdateData data) override {
		bool coll = false;
		for (auto& set : *data.collisionEdges)
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
	}
};
class MockPlayer : public Entity {
	bool up = false;
	bool down = false;
public:
	sf::Vector2f size;
	void PreUpdate() override {
		if (Difficulty::Current < Difficulty::PlayerTracking) {
			velocity = { 0,0 };
			return;
		}
		if (up) velocity = { 0,-PlayerSpeed };
		if (down) velocity = { 0,PlayerSpeed };
	}
	void Update(UpdateData data) override {
		position.y = std::clamp<float>(position.y, ScreenBox.top, ScreenBox.top + ScreenBox.height - size.y);
	}
	void Draw(sf::RenderTarget& target) {
	}
	void UpdateEdges() override {

		GetEdges({ position,size }, *edges);
	}
	MockPlayer() {
	}
	void Reset(Player* original, bool up, bool down) {
		this->up = up;
		this->down = down;
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

std::vector<Entity*> entities = { ball,opponent,player,
	new Vortex(ScreenCenter(),100,
		{
			{0,3.14 / 3}, {2. / 3 * 3.14,3. / 3 * 3.14},{4. / 3 * 3.14,5. / 3 * 3.14}
},sf::Color::White)
};

EdgeSet collisionEdges = {
	entities[0]->edges,entities[1]->edges,entities[2]->edges,entities[3]->edges, ScreenEdges,
};
std::vector<CollisionEdge>* playerWall = new std::vector<CollisionEdge>{
	{{ScreenBox.left + 75,ScreenBox.top},{ScreenBox.left + 75,ScreenBox.top + ScreenBox.height}}
};
void Reset() {
	opponent->position = {
		ScreenBox.left + ScreenBox.width - 50 - opponent->size.x,
		ScreenBox.top + ScreenBox.height / 2 - opponent->size.y / 2
	};
	ball->position = sf::Vector2f(
		ScreenBox.left + ScreenBox.width / 2 - ball->radius,
		ScreenBox.top + ScreenBox.height / 2 - ball->radius
	);
	player->size = sf::Vector2f(25, PlayerHeight());
	player->position = { ScreenBox.left + 50, 250 - player->size.y / 2 };
	ball->velocity = { 0,0 };
	ball->acceleration = { 0,0 };
	running = false;
}

void Tick(std::vector<Entity*>& entities, EdgeSet& collisionEdges) {

	std::vector<double > magnitudes;
	for (auto i : entities)
	{
		i->PreUpdate();
		magnitudes.push_back(magnitude(i->velocity));
		magnitudes.push_back(magnitude(i->acceleration));
	}
	double maxlength = *std::max_element(magnitudes.begin(), magnitudes.end());
	int times = std::max(1., ceil(maxlength / Engine::SegmentLength));
	int n = 0;
	while (n++ < times) {

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
			i->Update({ &entities, &collisionEdges,times });
		}
	}
}
sf::Vector2f PredictTrajectory() {
	int ticks = 0;
	double GravityAngle = Gravity::Angle();
	auto Gravity = ::Gravity::Vector();
	bool gravityLeft = sf::Keyboard::isKeyPressed(Controls::Gravity_CounterClockwise);
	bool gravityRight = sf::Keyboard::isKeyPressed(Controls::Gravity_Clockwise);
	static Mockball* ball = new Mockball;
	static MockPlayer* player = new MockPlayer;
	static MockVortex* vortex = new MockVortex;
	player->Reset(::player, PlayerUp, PlayerDown);
	ball->Reset(::ball);

	static std::vector<Entity*> entities = { ball,player,vortex};
	EdgeSet collisionEdges = {
	(Difficulty::Current >= Difficulty::PlayerCollisions) ? player->edges : nullptr,
	(Difficulty::Current >= Difficulty::BorderCollisions) ? MockScreenEdges : nullptr,
	(Difficulty::Current >= Difficulty::PlayerWall) ? playerWall : nullptr,
	(Difficulty::Current >= Difficulty::VortexTracking)? vortex->edges : nullptr
	};
	vortex->Reset((Vortex*)::entities[3]);
	for (auto i : collisionEdges)
	{
		if (i == nullptr) continue;
		for (auto& j : *i) {
			j.colliding = false;
		}
	}
	while (ball->position.x + 2 * ball->radius < opponent->position.x) {

		if (ticks > PredictionTicks) break;
		if (Difficulty::Current >= Difficulty::GravityTracking && Controls::Gravity_Mode == Controls::ControlMode::Keyboard) {
			if (gravityLeft) {
				GravityAngle += Gravity::RateOfChange;
				Gravity = sf::Vector2f(cos(GravityAngle) * Gravity::Magnitude, -sin(GravityAngle) * Gravity::Magnitude);
			}
			if (gravityRight) {
				GravityAngle -= Gravity::RateOfChange;
				Gravity = sf::Vector2f(cos(GravityAngle) * Gravity::Magnitude, -sin(GravityAngle) * Gravity::Magnitude);

			}
		}
		ball->acceleration = Gravity;
		Tick(entities, collisionEdges);
		ticks++;
	}
	auto pos = ball->position;
	if (ticks > PredictionTicks && Difficulty::Current >= Difficulty::Fallback) return { 0, ScreenBox.top + ScreenBox.height / 2 };
	return pos;
}

void ManageOpponent() {
	OpponentUp = false;
	OpponentDown = false;
	auto ballPos = (Difficulty::Current >= Difficulty::Prediction) ? PredictTrajectory() : ball->position;
	auto opponentPos = opponent->position;
	opponentPos.y += opponent->size.y / 2;
	if (ballPos.y < opponentPos.y - OpponentSpeed()) OpponentUp = true;
	if (ballPos.y > opponentPos.y + OpponentSpeed()) OpponentDown = true;
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
	compass.setOrigin({ 0,compass.getSize().y / 2 });
	for (auto& i : entities)
	{
		i->Draw(target);
	}
	target.draw(compass);
	sf::RectangleShape line({ 5.f,float(target.getSize().y) });
	line.setFillColor(sf::Color::Magenta);
	line.setPosition(ScreenBox.left - line.getSize().x, 0);
	target.draw(line);
	
	sf::CircleShape compassCore;
	compassCore.setFillColor(sf::Color::Red);
	compassCore.setRadius(5);
	compassCore.setPosition(compass.getPosition() - sf::Vector2f(5, 5));
	target.draw(compassCore);
	if constexpr (DrawEdges) {
		sf::CircleShape vertexpointer;
		vertexpointer.setFillColor(sf::Color::Blue);
		vertexpointer.setRadius(3);

		sf::Vertex z[2]{};
		z[0].color = z[1].color = sf::Color::Blue;
		for (auto set : collisionEdges)
		{
			if (set == nullptr) continue;
			for (auto& i : *set) {

				vertexpointer.setPosition(i.start + sf::Vector2f(-3, -3));
				target.draw(vertexpointer);
				vertexpointer.setPosition(i.end + sf::Vector2f(-3, -3));
				target.draw(vertexpointer);
				z[0].position = i.start;
				z[1].position = i.end;
				target.draw(z, 2, sf::LineStrip);
			}
		}
	}
	auto text = "Difficulty: \n"
		+ std::to_string(Difficulty::Current) + "\n"
		+ "Time: \n"
		+ TimeFormat(framesPassed / FPS) + "\n";

	draw_text(false, false, 10, 10, text, target, font_texture, 22);
	for (auto& i : leaderboard)
	{
		/*draw_text(false, false, 10, y,
			"Level " + std::to_string(i.first) + " : " + TimeFormat(i.second / 60),
			target,
			font_texture,
			16);*/

			//y += 16;
	}
}
void ShootBall() {
	/*double velocity = 10;
	
	ball->velocity = sf::Vector2f( velocity * cos(Gravity::Angle()), velocity * sin(Gravity::Angle()) );*/
}
int main() {

	sf::RenderWindow window(sf::VideoMode(ScreenBox.left + ScreenBox.width, ScreenBox.top + ScreenBox.height), "Ping Pong");
	std::chrono::steady_clock clock;
	auto lasttime = clock.now();
	compass.setSize(Compass::Size);
	compass.setFillColor(Compass::Color);
	compass.setPosition(ScreenBox.getPosition() + (0.5f * ScreenBox.getSize()));
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
			case sf::Event::KeyReleased:
				if (event.key.code == sf::Keyboard::Space) {
					if (running) {
						Reset();
					}
					else {

						running = true;
						ShootBall();
					}
				}
			case sf::Event::MouseWheelScrolled:
				if (Controls::Player_Mode == Controls::ControlMode::Scroll) {
					player->position.y += event.mouseWheelScroll.delta * Controls::Player_Scroll_Sensitivity;
				}
				if (Controls::Gravity_Mode == Controls::ControlMode::Scroll) {
					Gravity::SetAngle(Gravity::Angle() + event.mouseWheelScroll.delta * Controls::Gravity_Scroll_Sensitivity);
					ball->acceleration = Gravity::Vector();
				}
				break;
			default:
				break;
			}
			

		}	
		if (Controls::Gravity_Mode == Controls::ControlMode::Keyboard) {
			if (sf::Keyboard::isKeyPressed(Controls::Gravity_CounterClockwise)) Gravity::SetAngle(Gravity::Angle() + Gravity::RateOfChange);
			if (sf::Keyboard::isKeyPressed(Controls::Gravity_Clockwise)) Gravity::SetAngle(Gravity::Angle() - Gravity::RateOfChange);
			ball->acceleration = Gravity::Vector();
		}
		else {
			auto coords = sf::Mouse::getPosition(window);

			auto position = window.mapPixelToCoords(coords);
			if (Controls::Gravity_Mode == Controls::ControlMode::Mouse) {
				
				//std::cout << positionF.x << "   " << positionF.y << std::endl;
				sf::Vector2f center = compass.getPosition();// { screenBox.left + screenBox.width, screenBox.top + screenBox.height / 2 };
				auto delta = position - center;
				delta.y *= -1;
				Gravity::SetAngle(atan2(delta.y, delta.x));
				ball->acceleration = Gravity::Vector();
			}
			if (Controls::Player_Mode == Controls::ControlMode::Mouse) {
				player->position.y = position.y - player->size.y/2;
			}
		}

		auto thisTime = clock.now();
		auto duration = thisTime - lasttime;
		if (duration.count() >= FrameDuration * 1000000) {
			lasttime = thisTime;
			compass.setRotation(-Gravity::Angle() * 180 / 3.14);
			PlayerUp = sf::Keyboard::isKeyPressed(sf::Keyboard::LBracket);
			PlayerDown = sf::Keyboard::isKeyPressed(sf::Keyboard::Quote);
			if (running) Tick(entities, collisionEdges);
			framesPassed++;
		}
		DrawGame(window);
		window.display();

	}


}