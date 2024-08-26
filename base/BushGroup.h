#ifndef QTB_BushGroup
#define QTB_BushGroup

#include "Bush.h"

namespace qtb 
{
	class Zone;

	class BushGroup
	{
	public:
		friend class Land;
		friend class Zone;

	private:
		BushGroup(unsigned int id);

	public:
		unsigned int	id() const { return m_id; }
		const Area&		overall() const { return m_overall; }
		const BushPMap&	bushes() const { return m_bushes; }
		const Zone*		zone() const { return m_zone; }

		bool			overlap(const Bush& bush) const;
		bool			contains(float x, float y, unsigned int* bushID = NULL) const;

	private:
		void			add(Bush* bush);
		void			remove(unsigned int bushID);
		void			splice(BushGroup& other);

	private:
		unsigned int	m_id;
		Zone*			m_zone;

		BushPMap		m_bushes;
		Area			m_overall;
	};

	typedef std::map<unsigned int, BushGroup*>	BushGroupPMap;

}

#endif
