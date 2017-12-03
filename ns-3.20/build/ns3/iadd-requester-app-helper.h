/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef IADD_REQUESTER_APP_HELPER_H
#define IADD_REQUESTER_APP_HELPER_H

#include "ns3/iadd-requester-app.h"


namespace ns3 {

class IADDApplicationHelper
{
public:

	IADDApplicationHelper();

	 void SetAttribute (std::string name, const AttributeValue &value);

	// void SetDataInfo ( Ptr<Application> app, float_t dataInfo);

	  ApplicationContainer Install (NodeContainer c);
	  ApplicationContainer Install (Ptr<Node> node);
	  Ptr<IADDApplication> GetApplication (void);
private:
	  ObjectFactory m_simpleApplicationFactory;
	  Ptr<IADDApplication> m_simpleApplication;

}; //SimpleModuleHelper end

}

#endif /* IADD_REQUESTER_APP_HELPER_H */

