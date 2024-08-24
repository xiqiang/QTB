#ifndef Editor_PerfTool
#define Editor_PerfTool

#include <windows.h>

class PerfTool
{
public:
	PerfTool() {
		QueryPerformanceFrequency(&m_frequency);
		m_quadpart = (double)m_frequency.QuadPart;
		m_timeStart.QuadPart = 0;
		m_timeEnd.QuadPart = 0;
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
	double			m_quadpart;		//��ʱ��Ƶ��
	LARGE_INTEGER	m_timeStart;	//��ʼʱ�� 
	LARGE_INTEGER	m_timeEnd;		//����ʱ�� 
};

#endif
