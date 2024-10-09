#ifndef Editor_Robot
#define Editor_Robot

#include<map>
#include "../base/Land.h"
#include "Util.h"

const float ROBOT_SPEED_MIN = 2.0f;
const float ROBOT_SPEED_MAX = 10.0f;

const float MOVE_TIME_MIN = 2.0f;
const float MOVE_TIME_MAX = 5.0f;

const float BUSH_TIME_MIN = 2.0f;
const float BUSH_TIME_MAX = 5.0f;
const float BUSH_COUNT_PER_SEC = 1 / ((BUSH_TIME_MIN + BUSH_TIME_MAX) * 0.5f);

const float BUSH_LIVE_AVG = (float)PER_ROBOT_AREA / PER_BUSH_AREA / BUSH_COUNT_PER_SEC;
const float BUSH_LIVE_MIN = BUSH_LIVE_AVG * 0.5f;
const float BUSH_LIVE_MAX = BUSH_LIVE_AVG * 1.5f;


class Robot
{
public:
	struct BushInfo
	{
		BushInfo() : id(-1), life(0) {}
		BushInfo(unsigned int _id, float _life) : id(_id), life(_life) {}
		unsigned int	id;
		float			life;
	};

	typedef std::map<unsigned int, BushInfo> BushInfoMap;

public:
	Robot()
		: m_x(0.0f)
		, m_y(0.0f)
		, m_dir(0.0f)
		, m_speed(0.0f)
		, m_updateTime(0.0f)
		, m_randMove(false)
		, m_moveTime(0.0f)
		, m_moveTimeTotal(0.0f)
		, m_autoBush(false)
		, m_bushGroupID(-1)
		, m_bushTime(0.0f)
		, m_bushTimeTotal(0.0f)
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
		m_speed = RangeRand(ROBOT_SPEED_MIN, ROBOT_SPEED_MAX);
		m_dir = RangeRand(0.0f, PI2);
		m_moveTimeTotal = RangeRand(MOVE_TIME_MIN, MOVE_TIME_MAX);
		m_moveTime = 0;
	}

	void CreateBush(qtb::Land* land)
	{
		float w = RangeRand(AREA_SIZE_MIN, AREA_SIZE_MAX) * 0.5f;
		float h = RangeRand(AREA_SIZE_MIN, AREA_SIZE_MAX) * 0.5f;
		unsigned int bushID = ::CreateBush(land, qtb::Area(m_x - w, m_x + w, m_y - h, m_y + h));
		m_bushInfos[bushID] = BushInfo(bushID, RangeRand(BUSH_LIVE_MIN, BUSH_LIVE_MAX));

		m_bushTimeTotal = RangeRand(BUSH_TIME_MIN, BUSH_TIME_MAX);
		m_bushTime = 0;
	}

	void Tick(qtb::Land* land, float time)
	{
		if (m_updateTime <= 0)
		{
			m_updateTime = time;
			return;
		}

		float deltaTime = time - m_updateTime;
		m_updateTime = time;

		if (m_randMove)
		{
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

		if (m_autoBush)
		{
			m_bushTime += deltaTime;
			if (m_bushTime >= m_bushTimeTotal)
				CreateBush(land);

			BushInfoMap::iterator it = m_bushInfos.begin();
			while (it != m_bushInfos.end())
			{
				it->second.life -= deltaTime;
				if (it->second.life <= 0)
				{
					RemoveBush(land, it->first);
					it = m_bushInfos.erase(it);
				}
				else
				{
					++it;
				}
			}
		}
	}

	void EnableMove(bool value)
	{
		m_randMove = value;
	}

	void EnableBush(qtb::Land* land, bool value)
	{
		m_autoBush = value;

		if (!value)
		{
			BushInfoMap::const_iterator itEnd = m_bushInfos.end();
			for(BushInfoMap::const_iterator it = m_bushInfos.begin(); it != itEnd; ++it)
				RemoveBush(land, it->first);
			m_bushInfos.clear();
		}
	}

private:
	qtb::Area		m_area;

	float			m_x;
	float			m_y;
	float			m_dir;
	float			m_speed;
	float			m_updateTime;

	bool			m_randMove;
	float			m_moveTime;
	float			m_moveTimeTotal;

	bool			m_autoBush;
	unsigned int	m_bushGroupID;
	float			m_bushTime;
	float			m_bushTimeTotal;
	BushInfoMap		m_bushInfos;
};

#endif
