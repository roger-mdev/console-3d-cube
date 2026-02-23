#include <Windows.h>
#include <iostream>
#include <vector>
#include <array>
#include <cmath>
#include <string>
#include <thread>
#include <chrono>

namespace Global
{
	float A{ 0 }, B{ 0 }, C{ 0 };
	std::size_t height{};
	std::size_t width{};

	// might find a way to use getting window size to adjust cube and dist from
	int cubeSize{ 20 };
	float distanceFromCam{ 100 }; //higher is further from monitor
	float scale{ 30 };
	constexpr std::array<char, 6> symbol{ '@', '#', '%', '.', '=', '^' };
}

class Buffer
{
public:
	Buffer(std::size_t h, std::size_t w)
		: m_buffer(h * w, ' ')
		, m_zbuffer(h * w, 0)
	{}
	
	void resetBuffers()
	{
		std::fill(m_buffer.begin(), m_buffer.end(), ' ');
		std::fill(m_zbuffer.begin(), m_zbuffer.end(), 0);

	}

	std::vector<char>& buffer() { return m_buffer; }
	std::vector<float>& zbuffer() { return m_zbuffer; }

private:
	std::vector<char> m_buffer;
	std::vector<float> m_zbuffer;
};

void setTerminalSize()
{
	// Source - https://stackoverflow.com/a/23370070
	// Posted by herohuyongtao, modified by community. See post 'Timeline' for change history
	// Retrieved 2026-02-19, License - CC BY-SA 4.0

	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
	Global::width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
	Global::height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
}



void clearAndHideTerminal()
{
	system("cls"); 
	std::cout << "\x1b[?25l";
}

float calculateX(float i, float j, float k) {
	using namespace Global;
	return i * cos(C) * cos(B) + j * cos(C) * sin(B) * sin(A) - j * sin(C) * cos(A)
	+ k * cos(C) * sin(B) * cos(A) + k * sin(C) * sin(A);
}
	
float calculateY(float i, float j, float k) {
	using namespace Global;
	return i * sin(C) * cos(B) + j * sin(C) * sin(B) * sin(A) + j * cos(C) * cos(A)
	+ k * sin(C) * sin(B) * cos(A) - k * cos(C) * sin(A);
}

float calculateZ(float i, float j, float k) {
	using namespace Global;
	return i * -(sin(B)) + j * cos(B) * sin(A) + k * cos(B) * cos(A);
}

void renderPoint(float x, float y, float z, char symbol, Buffer& b)
{
	float xp = calculateX(x, y, z);
	float yp = calculateY(x, y, z);
	float zp = calculateZ(x, y, z) + Global::distanceFromCam;
	
	if (zp <= 0.1f) return;
	float depth{ 1 / zp };
	
	float aspect = 2.0f;
	int screenX{ static_cast<int>(Global::width / 2 + Global::scale * xp / zp * aspect) };
	int screenY{ static_cast<int>(Global::height / 2 - Global::scale * yp / zp) };

	if (screenX < 0 || screenX >= Global::width || screenY < 0 || screenY >= Global::height)
		return;

	std::size_t index{ screenY * Global::width + screenX };

	

	if (depth > b.zbuffer()[index])
	{
		b.zbuffer()[index] = depth;

		b.buffer()[index] = symbol;
	}
}

void rotate()
{
	Global::A += 0.05;
	Global::B += 0.05;
	Global::C += 0.01;
}

void render(Buffer& b)
{
	float asciiDencity{ 0.8 };
	for (float x = -Global::cubeSize; x < Global::cubeSize; x += asciiDencity)
		for (float y = -Global::cubeSize; y < Global::cubeSize; y += asciiDencity)
		{
			//front side
			renderPoint(x, y, -Global::cubeSize, Global::symbol[0], b);
			//right side
			renderPoint(Global::cubeSize, y, x, Global::symbol[1], b);
			//left side
			renderPoint(-Global::cubeSize, y, -x, Global::symbol[2], b);
			//back side
			renderPoint(-x, y, Global::cubeSize, Global::symbol[3], b);
			//bottom side
			renderPoint(x, -Global::cubeSize, -y, Global::symbol[4], b);
			//top side
			renderPoint(x, Global::cubeSize, y, Global::symbol[5], b);
		}
	rotate();
}

void drawCube(Buffer& b)
{
	std::string frame;
	frame.reserve(Global::width * Global::height + Global::height);
	frame = "\x1b[H";

	for (std::size_t i{}; i < Global::height; ++i)
	{
		frame.append(&b.buffer()[i * Global::width], Global::width);
		frame.push_back('\n');
	}

	std::string output;
	for (char c : frame)
	{
		switch (c)
		{
		case Global::symbol[0]: 
		{
			output += "\033[34m";  
			output += c;  
			output += "\033[0m"; // Reset color
			break;
		}; //blue
		case Global::symbol[1]:
		{
			output += "\033[31m";  
			output += c;  
			output += "\033[0m"; // Reset color
			break;
		}; //red
		case Global::symbol[2]: 
		{
			output += "\033[33m";  
			output += c;  
			output += "\033[0m"; // Reset color
			break;
		}; //yellow
		case Global::symbol[3]:
		{
			output += "\033[36m";  
			output += c;  
			output += "\033[0m"; // Reset color
			break;
		}; //cyan
		case Global::symbol[4]: 
		{
			output += "\033[35m";  
			output += c; 
			output += "\033[0m"; // Reset color
			break;
		}; //magenta
		case Global::symbol[5]: 
		{
			output += "\033[32m";  
			output += c; 
			output += "\033[0m"; // Reset color
			break;
		}; //green
		default: 
		{
			output += c;
			break;
		}
		}
	}

	std::cout << output << std::flush;
}

int main(int argc, char* argv[])
{
	setTerminalSize();
	Buffer b{ Global::height, Global::width };
	clearAndHideTerminal();
	while (true)
	{
		
		render(b);
		drawCube(b);

		b.resetBuffers();
		std::this_thread::sleep_for(std::chrono::milliseconds(8));
	}

	return 0;
}