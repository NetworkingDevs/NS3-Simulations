/**
 * This code is made for demonstration of Object Aggregation
 * concept in NS3, You can review this code to set-up object
 * aggregation in your simulations.
 *
 * Major LOC: 63-to-88
 **/

#include "ns3/core-module.h"
#include "ns3/network-module.h"

using namespace ns3;

class DummyClass : public ns3::Object
{
	public:
		static TypeId GetTypeId();
		DummyClass();
		virtual ~DummyClass();
		
		// Getter and Setter
		void SetValue (double val);
		double GetValue();
	
	private:
		double m_value;
};

TypeId DummyClass::GetTypeId()
{
	static TypeId t = TypeId("ns3::DummyClass")
				.SetParent<Object>()
				.AddConstructor<DummyClass>();
	return t;
}

DummyClass::DummyClass()
{

}

DummyClass::~DummyClass()
{

}

double DummyClass::GetValue()
{
	return m_value;
}

void DummyClass::SetValue(double val)
{
	m_value = val;
}

int
main(int argc, char* argv[])
{
	NodeContainer n;
	n.Create(2);
	
	Ptr<DummyClass> user = CreateObject<DummyClass>();
	user->SetValue(12);
	
	Ptr<Node> n0 = n.Get(0);
	n0->AggregateObject(user);
	
	Ptr<Node> new_copy = ns3::CopyObject(n0);
	std::cout << "Origional Node: \t" << n0 << "\nCopied Node: \t" <<new_copy << std::endl;
	
	// copied object doesn't have aggregate objects...
	Ptr<DummyClass> obj = new_copy->GetObject<DummyClass>();
	std::cout << "Dummy Class From Copied Obj: " << obj << std::endl;
	if(obj==NULL)
	{
		std::cout << "IT'S NULL" << std::endl;
	} else {
		std::cout << "NOT NULL" << std::endl;
	}
	
	// We can only get the aggregated object from the origional one
	obj = n0->GetObject<DummyClass>();
	std::cout << "Obj: " << obj << "\t" << obj->GetValue() << std::endl;
	
	// We can also go the other way
	Ptr<Node> ptr_to_node = user->GetObject<Node>();
	std::cout << "Oriogional Node:\t" << n0 << "\nPtr to origional:\t" << ptr_to_node << std::endl;
	
	Simulator::Stop(Seconds(60));
	Simulator::Run();
	Simulator::Destroy();	
}
