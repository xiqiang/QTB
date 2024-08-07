#ifndef Editor_PerfTool
#define Editor_PerfTool

#include <windows.h>

class PerfTool
{
public:
	PerfTool() {
		QueryPerformanceFrequency(&m_frequency);
		m_quadpart = (double)m_frequency.QuadPart;
	}

	void Start() {
		QueryPerformanceCounter(&m_timeStart);
	}

	double End() {
		QueryPerformanceCounter(&m_timeEnd);
		return (m_timeEnd.QuadPart - m_timeStart.QuadPart) / m_quadpart;
	}

private:
	LARGE_INTEGER	m_frequency;	//计时器频率  
	LARGE_INTEGER	m_timeStart;	//开始时间  
	LARGE_INTEGER	m_timeEnd;		//结束时间  
	double			m_quadpart;		//计时器频率  
};

#endif
