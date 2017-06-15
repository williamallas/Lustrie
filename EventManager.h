#pragma once

#include <EASTL/map.h>
#include <Windows.h>
#include <math/Vector.h>
#include <EASTL/algorithm.h>

class EventManager
{
public:
	struct KeyState
	{
		bool pressed = false;
		bool firstPress = false;
	};

	void setMousePos(tim::ivec2 v, bool updateDelta = true) 
	{
		if (updateDelta)
		{
			_deltaMouse = v - _curMousePos;
			//_deltaMouse.x() = eastl::min(eastl::max(_deltaMouse.x(), -3), 3);
			//_deltaMouse.y() = eastl::min(eastl::max(_deltaMouse.y(), -3), 3);
		}
		
		_curMousePos = v;  
	}

	tim::ivec2 mouseDelta() const { return _deltaMouse;  }

	KeyState key(WPARAM key)
	{
		auto it = _key.find(key);
		if (it != _key.end())
			return it->second;
		else 
			return KeyState();
	}

	KeyState mouseButton(int b) const { return _mouseButton[b]; }

	void setKeyPressed(WPARAM key)
	{
		_key[key] = { true, true };
	}

	void setKeyReleased(WPARAM key)
	{
		_key[key] = { false, false };
	}

	void setMousePressed(int b)
	{
		_mouseButton[b] = { true, true };
	}

	void setMouseReleased(int b)
	{
		_mouseButton[b] = { false, false };
	}

	void update()
	{
		_deltaMouse = { 0,0 };

		for (auto& p : _key)
			p.second.firstPress = false;

		_mouseButton[0].firstPress = false; 
		_mouseButton[1].firstPress = false;
	}

private:
	tim::ivec2 _curMousePos=0;
	tim::ivec2 _deltaMouse=0;
	eastl::map<WPARAM, KeyState> _key;
	KeyState _mouseButton[2];
};