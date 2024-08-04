#ifndef TreeBush_Bunch
#define TreeBush_Bunch

#include "Bush.h"

namespace treebush 
{
	class Land;
	class Zone;

	class Bunch
	{
	public:
		friend class Zone;

	public:
		Bunch(Land* land);

	public:
		unsigned int	id() const { return m_id; }
		const Area&		overall() { return m_overall; }

		void			addBush(Bush* bush);
		bool			bushCheck(float x, float y) const;

	private:
		Land*			m_land;
		Zone*			m_zone;
		unsigned int	m_id;

	private:
		BushPList		m_bushList;
		Area			m_overall;
	};

	typedef std::map<unsigned int, Bunch*> BunchMap;

}

#endif
