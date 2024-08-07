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
	LARGE_INTEGER	m_frequency;	//��ʱ��Ƶ��  
	LARGE_INTEGER	m_timeStart;	//��ʼʱ��  
	LARGE_INTEGER	m_timeEnd;		//����ʱ��  
	double			m_quadpart;		//��ʱ��Ƶ��  
};

#endif
