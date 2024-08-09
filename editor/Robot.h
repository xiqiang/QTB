#ifndef Editor_Robot
#define Editor_Robot

#include "../base/Area.h"
#include "Util.h"

class Robot
{
public:
	Robot()
		: m_x(0.0f)
		, m_y(0.0f)
		, m_dir(0.0f)
		, m_speed(0.0f)
		, m_moveTime(0.0f)
		, m_moveTimeTotal(0.0f)
		, m_updateTime(0.0f)
		, m_bushGroupID(-1)
	{
	}

	void Init(const qtb::Area& area)
	{
		m_area = area;

		m_x = RangeRand(m_area.left, m_area.right);
		m_y = RangeRand(m_area.bottom, m_area.top);
		StartRoam();
	}

	float x() const { return m_x; }
	float y() const { return m_y; }

	void setBushGroupID(unsigned int value) { m_bushGroupID = value; }
	unsigned int getBrushGroupID()const { return m_bushGroupID; }

	void StartRoam()
	{
		m_speed = RangeRand(5.0f, 15.0f);
		m_dir = RangeRand(0.0f, PI2);
		m_moveTimeTotal = RangeRand(5.0f, 10.0f);
		m_moveTime = 0;
	}

	void Tick(float time)
	{
		if (m_updateTime <= 0)
		{
			m_updateTime = time;
			return;
		}

		float deltaTime = time - m_updateTime;
		m_updateTime = time;
		m_moveTime += deltaTime;

		float x = (float)(m_speed * cos(m_dir));
		float y = (float)(m_speed * sin(m_dir));
		float nx = m_x + x * deltaTime;
		float ny = m_y + y * deltaTime;
		if (!m_area.contains(nx, ny) || m_moveTime >= m_moveTimeTotal)
		{
			StartRoam();
		}
		else
		{
			m_x = nx;
			m_y = ny;
		}
	}

private:
	qtb::Area m_area;

	float m_x;
	float m_y;
	float m_dir;
	float m_speed;

	float m_moveTime;
	float m_moveTimeTotal;
	float m_updateTime;

	unsigned int m_bushGroupID;
};

#endif
