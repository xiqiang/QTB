#ifndef QTB_Zone
#define QTB_Zone

#include "QTree.h"
#include "BushGroup.h"

namespace qtb 
{
	class Zone
		: public QTree
	{
	public:
		friend class Land;

	private:
		Zone(const Area& area, Zone* parent = NULL);

	public:
		unsigned int			generation() const { return m_generation; }

		bool					bushContains(float x, float y, unsigned int* bushGroupID = NULL, unsigned int* bushID = NULL);

	protected:
		virtual QTree*			newChild(const Area& area);

	private:
		void					addBushGroup(BushGroup* group);
		void					removeBushGroup(unsigned int groupID);
		const BushGroupPMap&	bushGroups() const { return m_bushGroups; }

		bool					_bushContains(float x, float y, unsigned int* bushGroupID = NULL, unsigned int* bushID = NULL) const;

	private:
		unsigned int	m_generation;
		BushGroupPMap	m_bushGroups;
	};
}

#endif
