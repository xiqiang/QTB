#ifndef QTB_BushGroup
#define QTB_BushGroup

#include "Bush.h"

namespace qtb 
{
	class Land;
	class Zone;

	class BushGroup
	{
	public:
		friend class Zone;

	public:
		BushGroup(Land* land);

	public:
		unsigned int		id() const { return m_id; }
		const Area&			overall() const { return m_overall; }
		Zone*				zone() const { return m_zone; }

		void				addBush(Bush* bush);
		void				removeBush(unsigned int bushID);

		bool				overlap(const Bush& r) const;
		void				splice(BushGroup& r);
		bool				bushCheck(float x, float y, unsigned int* bushID = NULL) const;

		const BushPMap&		bushMap() { return m_bushMap; }

	private:
		Land*			m_land;
		Zone*			m_zone;
		unsigned int	m_id;

	private:
		BushPMap		m_bushMap;
		Area			m_overall;
	};

	typedef std::map<unsigned int, BushGroup*> BushGroupPMap;

}

#endif
