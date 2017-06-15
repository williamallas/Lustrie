#pragma once

#include <math/Vector.h>
#include <math/Matrix.h>
#include <math/Quaternion.h>
#include "EventManager.h"

using namespace tim;

struct ControlCamera
{
	vec3 position;
	vec3 direction = { -1,-1,-1 };
	vec3 up = { 0,0,1 };

	void update(EventManager& event, float time)
	{
		direction.normalize();

		float boost = 5;
		if (event.key(VK_SHIFT).pressed)
			boost = 30;

		if (event.key('W').pressed)
			position += direction * time * boost;
		else if (event.key('S').pressed)
			position -= direction * time * boost;

		vec3 ortho = up.cross(direction);

		if (event.key('A').pressed)
			position += ortho * time * boost;
		else if (event.key('D').pressed)
			position -= ortho * time * boost;

		if (!event.mouseButton(1).pressed)
			return;

		if (event.mouseDelta().x() != 0)
		{
			Quat rotUp = Quat::from_axis_angle(up, -event.mouseDelta().x() * time * 2);
			direction = rotUp(direction);
		}
		if (event.mouseDelta().y() != 0)
		{
			Quat rotOrtho = Quat::from_axis_angle(ortho, event.mouseDelta().y() * time * 2);
			direction = rotOrtho(direction);
		}
	}

	mat4 computeView() const { return mat4::View(position, position + direction, up); }
};