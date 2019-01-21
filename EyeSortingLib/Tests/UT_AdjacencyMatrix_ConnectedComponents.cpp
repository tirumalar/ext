#include <tut/tut.hpp>
#include <stdio.h>
#include <stdlib.h>

#include "IrisSelector.h"
using namespace std;

namespace tut
{
	struct AdjacencyMatrixData
	{		
		
		AdjacencyMatrixData() 
		{ 	

		}
		~AdjacencyMatrixData() 
		{ 
		}

	};

	typedef	test_group<AdjacencyMatrixData> tg;
	typedef tg::object testobject;
}

namespace {
	tut::tg test_group("AdjacencyMatrix and ConnectedComponents TestCases");
}

namespace tut 
{
	template<>
	template<>
	void testobject::test<1>() 
	{
		set_test_name("Adjacency Matrix Test");
		AdjacencyMatrix m_graph(5);
		ensure("\nSize of Graph = ",m_graph.GetSize()==5);
		m_graph.Connect(2,3);
		m_graph.Connect(3,4);
		m_graph.Connect(1,4);
		ensure("IsConnected(2,3) = ",m_graph.Connected(2,3));
		ensure("IsConnected(3,4) = ",m_graph.Connected(3,4));
		ensure("IsConnected(1,4) = ",m_graph.Connected(1,4));
		ensure("IsConnected(2,1) = ",!m_graph.Connected(2,1));
	} 

	template<>
	template<>
	void testobject::test<2>() 
	{
		set_test_name("Connected Components Test1");
		AdjacencyMatrix m_graph(8);
		m_graph.Connect(1,7);
		m_graph.Connect(2,3);
		m_graph.Connect(2,5);
		m_graph.Connect(3,4);
		m_graph.Connect(5,6);
		m_graph.Connect(5,7);
		ensure("IsConnected(1,7) = ",m_graph.Connected(1,7));
		ensure("IsConnected(2,3) = ",m_graph.Connected(2,3));
		ensure("IsConnected(2,5) = ",m_graph.Connected(2,5));
		ensure("IsConnected(3,4) = ",m_graph.Connected(3,4));
		ensure("IsConnected(5,6) = ",m_graph.Connected(5,6));
		ensure("IsConnected(5,7) = ",m_graph.Connected(5,7));
		ConnectedComponents cc(m_graph);
		const std::vector<int> &labels = cc.GetLabels();

		for(int i = 0;i<labels.size();i++)
		{
			//cout<<"\nNode = "<< i <<"  Label = " << labels[i];
		}
		
		ensure("Node0 should have label 0",labels[0]==0);
		ensure("Node1 should have label 1",labels[1]==1);
		ensure("Node2 should have label 1",labels[2]==1);
		ensure("Node3 should have label 1",labels[3]==1);
		ensure("Node4 should have label 1",labels[4]==1);
		ensure("Node5 should have label 1",labels[5]==1);
		ensure("Node6 should have label 1",labels[6]==1);
		ensure("Node7 should have label 1",labels[7]==1);
	} 

	template<>
	template<>
	void testobject::test<3>() 
	{
		set_test_name("Connected Components Test2");
		AdjacencyMatrix m_graph(6);
		m_graph.Connect(0,1);
		m_graph.Connect(2,3);
		m_graph.Connect(0,4);
		m_graph.Connect(1,4);
		m_graph.Connect(3,5);
		
		ConnectedComponents cc(m_graph);
		const std::vector<int> &labels = cc.GetLabels();

		for(int i = 0;i<labels.size();i++)
		{
			//cout<<"\nNode = "<< i <<"  Label = " << labels[i];
		}
		
		ensure("Node0 should have label 0",labels[0]==0);
		ensure("Node1 should have label 0",labels[1]==0);
		ensure("Node2 should have label 1",labels[2]==1);
		ensure("Node3 should have label 1",labels[3]==1);
		ensure("Node4 should have label 0",labels[4]==0);
		ensure("Node5 should have label 1",labels[5]==1);
	} 

} 
