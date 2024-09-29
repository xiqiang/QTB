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
		~BushGroup();

	public:
		unsigned int	id() const { return m_id; }
		const Area&		overall() const { return m_overall; }
		const BushPMap&	bushes() const { return m_bushes; }
		const Zone*		zone() const { return m_zone; }

		bool			overlap(const Bush& bush) const;
		bool			contains(float x, float y, unsigned int* bushID = NULL) const;

	private:
		bool			add(Bush* bush);
		void			remove(unsigned int bushID);
		bool			splice(BushGroup& other);
		void			releaseBushes();

	private:
		unsigned int	m_id;
		Zone*			m_zone;

		BushPMap		m_bushes;
		Area			m_overall;

		QTB_OVERLOAD_BLOCK
	};

	typedef std::map<unsigned int, BushGroup*>	BushGroupPMap;

}

#endif
