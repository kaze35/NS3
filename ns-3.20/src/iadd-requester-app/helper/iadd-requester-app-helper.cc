/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "iadd-requester-app-helper.h"

namespace ns3 {


IADDApplicationHelper::IADDApplicationHelper()
 {
	m_simpleApplicationFactory.SetTypeId
	(IADDApplication::GetTypeId ());
 }

 void IADDApplicationHelper::SetAttribute(std::string name, const AttributeValue &value)
 {
	 m_simpleApplicationFactory.Set(name, value);
 }

 ApplicationContainer IADDApplicationHelper::Install (NodeContainer c)
 {
	 ApplicationContainer apps;
	 for (NodeContainer::Iterator nodei =
			 c.Begin(); nodei != c.End(); ++nodei )
	 {
		 Ptr<Node> node = *nodei;
		 m_simpleApplication =
				 m_simpleApplicationFactory.Create<IADDApplication> ();
		 node->AddApplication(m_simpleApplication);
		 apps.Add (m_simpleApplication);
	 }
	 return apps;
 }

 ApplicationContainer IADDApplicationHelper::Install (Ptr<Node> node)
   {
  	 ApplicationContainer apps;
  	m_simpleApplication = m_simpleApplicationFactory.Create<IADDApplication> ();
  	 node->AddApplication(m_simpleApplication);
  	 apps.Add (m_simpleApplication);
  	 return apps;
   }


 Ptr<IADDApplication> IADDApplicationHelper::GetApplication(void)
 {
	 return m_simpleApplication;
 }

}



