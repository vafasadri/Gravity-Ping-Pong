#include <SFML/Graphics.hpp>
#include <iostream>
#include <Windows.h>
#include <sstream>
static sf::Texture get_texture(int resource) {
	auto module = GetModuleHandleW(0);
	HRSRC myResource = ::FindResource(module, MAKEINTRESOURCE(resource), L"PNG");
	unsigned int myResourceSize = ::SizeofResource(NULL, myResource);
	HGLOBAL myResourceData = ::LoadResource(NULL, myResource);
	void* pMyBinaryData = ::LockResource(myResourceData);
	sf::Texture texture;
	texture.loadFromMemory(pMyBinaryData, myResourceSize);
	UnlockResource(myResourceData);
	return texture;
}
static sf::Texture get_texture(std::string file) {
	sf::Texture texture;
	texture.loadFromFile(file);
	return texture;
}
void draw_text(const bool i_center_x, const bool i_center_y, const short i_x, const short i_y, const std::string& i_text, sf::RenderTarget& i_window, const sf::Texture& i_font_texture, const short i_font_size)
{


	//Our font texture consists of 96 characters.

	unsigned char character_width = i_font_texture.getSize().x / 96;
	unsigned char character_height = i_font_texture.getSize().y;

	float scale = float(i_font_size) / character_height;
	sf::Sprite character_sprite(i_font_texture);

	float character_y = i_y;
	if (i_center_y)
	{
		character_y -= 0.5f * scale * character_height * (1 + std::count(i_text.begin(), i_text.end(), '\n'));
	}
	std::stringstream ss{ i_text };
	std::string line;
	while (std::getline(ss, line, '\n'))
	{
		float character_x = i_x;
		if (i_center_x)
		{
			character_x -= 0.5f * scale * character_width * line.length();
		}
		for (char c : line)
		{
			character_sprite.setPosition(character_x, character_y);
			character_sprite.setScale(scale, scale);
			character_sprite.setTextureRect(sf::IntRect(character_width * (c - 32), 0, character_width, character_height));
			character_x += character_width * scale;

			i_window.draw(character_sprite);
		}
		character_y += i_font_size;
	}
}
extern auto font_texture = get_texture("Font.png");