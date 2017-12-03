/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#define NS_LOG_APPEND_CONTEXT                                   \
  { std::clog << Simulator::Now().GetSeconds()<< "s [node " << GetNode()->GetId () <<",Dest] "; }

#include "iadd-requester-app.h"
#include <string>
#include <sstream>
namespace ns3 {

/* ... */
NS_LOG_COMPONENT_DEFINE("IADDApplication");

void InterestTable::addInterest(Interest i) {
	InterestTableEntry in(i);
	table.push_back(in);
}

// DI : find data in the cache?
uint64_t InterestTable::dataReceived(Data d, Ptr<TimeMinMaxAvgTotalCalculator> e) {
	for (list<InterestTableEntry>::iterator i = table.begin(); i != table.end();
			++i) {
		Vector v1(i->interest.centerX, i->interest.centerY, 0);
		Vector v2(d.generatorX, d.generatorY, 0);
		if (distance(v1, v2) < i->interest.range && i->dataReceived == 0
				&& d.type == i->interest.type && d.uid==i->interest.uid) {
			i->dataReceived = Simulator::Now();
			i->dataGenerated = d.time;
			Time diff = (i->dataReceived - i->interestAdded);
			if(diff < Seconds(1))
				cout<<"wwwwwwwwwwwwwhhh";
			e->Update(diff);
			return i->interest.uid;
		}
	}
	return 0;
}
double InterestTable::distance(Vector v1, Vector v2) {
	return sqrt(pow(v1.x - v2.x, 2) + pow(v1.y - v2.y, 2));
}

TypeId IADDApplication::GetTypeId(void) {
	static TypeId tid =

	TypeId("ns3::IADDApplication").SetParent<Application>().AddConstructor<
			IADDApplication>().AddAttribute("Interval", "Interest frequency",
			UintegerValue(50), MakeTimeAccessor(&IADDApplication::m_interval),
			MakeTimeChecker()).AddAttribute("interestGenerationStart",
			"interestGenerationStart", TimeValue(Seconds(0)),
			MakeTimeAccessor(&IADDApplication::interestGenerationStart),
			MakeTimeChecker()).AddAttribute("interestGenerationEnd",
			"interestGenerationEnd", TimeValue(Seconds(100)),
			MakeTimeAccessor(&IADDApplication::interestGenerationEnd),
			MakeTimeChecker()).AddAttribute("gridMin", "Grid Min Size",
			UintegerValue(0), MakeDoubleAccessor(&IADDApplication::gridMin),
			MakeDoubleChecker<double>()).AddAttribute("gridMax",
			"Grid Max size", UintegerValue(100),
			MakeDoubleAccessor(&IADDApplication::gridMax),
			MakeDoubleChecker<double>());

	//uint32_t interestRate;
	Time interestGenerationStart;
	Time interestGenerationEnd;
	return tid;
}

IADDApplication::IADDApplication() :
		m_transmitTimer(Timer::CANCEL_ON_DESTROY) {
	m_interval = Seconds(0);
	//m_packetNum = 0;
	gridMax = 0;
	gridMin = 0;
	interestRate = 0;
}

IADDApplication::~IADDApplication() {
	m_transmitTimer.Cancel();
}

uint32_t IADDApplication::getNumberofInterestsSent() {
	return table.table.size();
}

uint32_t IADDApplication::getNumberofDataReceived() {
	uint32_t ctr = 0;
	for (list<InterestTableEntry>::iterator i = table.table.begin();
			i != table.table.end(); ++i) {
		if (i->dataReceived > 0) {
			ctr++;
		}
	}
	return ctr;
}

double IADDApplication::getAvgTime() {
	uint32_t ctr = 0;
	Time avg(0);
	for (list<InterestTableEntry>::iterator i = table.table.begin();
			i != table.table.end(); ++i) {
		if (i->dataReceived > 0) {
			ctr++;
			avg = avg + (i->dataReceived - i->interestAdded);
		}
	}
	if (ctr != 0)
		return avg.GetMilliSeconds() / ctr;
	else
		return 0;
}

Time IADDApplication::getMaxTime() {
	Time max(0);
	for (list<InterestTableEntry>::iterator i = table.table.begin();
			i != table.table.end(); ++i) {
		if (i->dataReceived > 0) {
			Time diff = (i->dataReceived - i->interestAdded);
			if (diff > max)
				max = diff;
		}
	}

	return max;
}

Time IADDApplication::getMinTime() {
	Time min(Simulator::Now() + Time(1));
	for (list<InterestTableEntry>::iterator i = table.table.begin();
			i != table.table.end(); ++i) {
		if (i->dataReceived > 0) {
			Time diff = (i->dataReceived - i->interestAdded);
			if (diff < min)
				min = diff;
		}
	}

	return min;
}

void IADDApplication::StartApplication(void) {
	NS_LOG_FUNCTION (this);
	//NS_LOG_INFO(""<<m_packetNum);
	//Create a socket to send only on this interface
	TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
	m_socket = Socket::CreateSocket(GetNode(), tid);
	NS_ASSERT(m_socket != 0);

	m_socket->Bind(InetSocketAddress(Ipv4Address::GetAny(), IADD_APP_PORT));
	//Ptr<Ipv4> ipv4 = GetObject<Ipv4>();

	//Ptr<Ipv4L3Protocol> l3 = ipv4->GetObject<Ipv4L3Protocol>();
	m_socket->BindToNetDevice(GetNode()->GetDevice(0));
	m_socket->SetAllowBroadcast(true);
	m_socket->SetRecvCallback(MakeCallback(&IADDApplication::RecvPacket, this));
	m_socket->SetAttribute("IpTtl", UintegerValue(255));

	m_transmitTimer.SetFunction(&IADDApplication::SendInterest, this);
	//double randValue = LogNormalVariable (0,1).GetValue();
	//m_transmitTimer.SetDelay(m_interval);
	Time jitter = MilliSeconds(UniformVariable().GetInteger(0, MAX_JITTER));
	m_transmitTimer.Schedule(jitter + interestGenerationStart);

}

void IADDApplication::StopApplication(void) {
	NS_LOG_FUNCTION_NOARGS ();

	CancelTimers();
	if (m_socket != 0) {
		m_socket->Close();
	} else {
		NS_LOG_WARN ("Application found null socket to close in StopApplication");
	}
	// summary and prints.
	cout << "Application Summary" << endl;
	cout << "Time From Interest Sent to Data Received:" << endl;
	cout << "Average time:" << getAvgTime() << endl;
	cout << "Max time:" << getMaxTime().GetMilliSeconds() << "ms" << endl;
	cout << "Min time:" << getMinTime().GetMilliSeconds() << "ms" << endl;
	cout << "Number of interests :" << getNumberofInterestsSent() << endl;
	cout << "Number of data:" << getNumberofDataReceived() << endl;

}

void IADDApplication::SendInterest() {
	NS_LOG_FUNCTION ( this );
	if (Simulator::Now() < interestGenerationStart) {
		m_transmitTimer.Cancel();
		Time jitter = MilliSeconds(UniformVariable().GetInteger(0, MAX_JITTER));
		m_transmitTimer.Schedule(jitter + m_interval);
		return;
	}
	if (Simulator::Now() > interestGenerationEnd) {
		m_transmitTimer.Cancel();
		return;
	}

	// Get Address ID of the current Node
	Ptr<Node> node = GetNode();
	Ptr<Ipv4> nodeIPv4 = node->GetObject<Ipv4>(); // Get Ipv4 instance of the node
	Ipv4Address nodeAddr = nodeIPv4->GetAddress(1, 0).GetLocal();
	//Ipv4InterfaceAddress nodeAddrInterface = nodeIPv4->GetAddress(1, 0);
	Ptr<MobilityModel> m = nodeIPv4->GetObject<MobilityModel>();
	Vector myPos = m->GetPosition();
	Vector center = getRandomPointOnGrid();
	//Vector center(300,300,0);
	//double range = getRandomRange();
	double range = 200;
	IADDInterestTypes intType = getRandomType();
	//IADDInterestTypes intType = TYPE1;

	IADDInterestHeader intHeader(myPos.x, myPos.y, center.x, center.y, range,
			false, false, Simulator::Now(), intType);
	intHeader.setTtl(Seconds(IADD_INTEREST_TTL));
	// Create Position Message
	Ptr<Packet> packet = Create<Packet>();
	packet->AddHeader(intHeader);

	cout<<"packet size: "<<packet->GetSize()<<endl;
	IADDTypeHeader tHeader(IADD_INTEREST);
	packet->AddHeader(tHeader);

	int byteSent = m_socket->SendTo(packet, 0,
			InetSocketAddress("192.168.254.254", IADD_APP_PORT));
	if (byteSent > 0) {
		Interest i(center.x, center.y, range, intType,packet->GetUid());
		table.addInterest(i);
		NS_LOG_INFO("New interest ("<<packet->GetUid()<<","<<center.x<<","<<center.y<<","<<range<<","<<intType<<") has been sent");
		numberofInterstGenerated->Update();
		tracer->addInterest( center.x, center.y, range, intType,packet->GetUid(),nodeAddr);
		//NS_LOG_DEBUG ("Send a packet t to socket from " <<
		//	nodeAddr << " with number of bytes " << byteSent);
	}
	//if (table.table.size() < m_packetNum) {
	if (byteSent <= 0) {
		//m_transmitTimer.Cancel();
		Time jitter = MilliSeconds(UniformVariable().GetInteger(0, MAX_JITTER));
		m_transmitTimer.Schedule(jitter + Seconds(1));
	} else {
		//m_transmitTimer.Cancel();
		Time jitter = MilliSeconds(UniformVariable().GetInteger(0, MAX_JITTER));
		m_transmitTimer.Schedule(jitter + m_interval);
	}

}

void IADDApplication::RecvPacket(Ptr<Socket> socket) {
	NS_LOG_FUNCTION (this << socket);
	Address sourceAddress;
	Ptr<Packet> packet = socket->RecvFrom(sourceAddress);

	// Get source IPv4 Address
	InetSocketAddress inetSourceAddr = InetSocketAddress::ConvertFrom(
			sourceAddress);

	Ipv4Address senderAddress = inetSourceAddr.GetIpv4();

	// Get receiver (this Node) IPv4 Address
	Ptr<Node> node = GetNode();
	Ptr<Ipv4> receiverIPv4 = node->GetObject<Ipv4>(); // Get Ipv4 instance of the node
	//Ipv4Address receiverAddress = receiverIPv4->GetAddress(1, 0).GetLocal();

	//NS_LOG_INFO ("Received a  packet from "
		//	<< senderAddress << " at " << receiverAddress);

	IADDTypeHeader tHeader(IADD_INTEREST);
	packet->RemoveHeader(tHeader);
	if (!tHeader.isValid()) {
		NS_LOG_WARN ("Message received with unknown message type\n Message Dropped\n");
		return; // drop
	}
	switch (tHeader.getType()) {
	case IADD_DATA: {
		IADDInterestHeader intHeader;
		packet->RemoveHeader(intHeader);
		//Interest interest(intHeader.getCenterX(), intHeader.getCenterY(),
		//	intHeader.getRange());
		Data d(intHeader.getCenterX(), intHeader.getCenterY(),
				intHeader.getTimeGenerated(), intHeader.getInterestType(),intHeader.getInterestUid());

		// DI : data generated ???
		uint64_t i = table.dataReceived(d, elapsedTime);
		NS_LOG_INFO ("Received a  packet from "
					<< senderAddress << " ID: " << packet->GetUid());
		if(packet->GetUid ()==106365)
			cout<<"fff";
		if (i!=0) {
			packetsArrived->Update();
			tracer->packetStatus[i] = "Interest_matched";
			tracer->packetStatus[packet->GetUid ()]	 = "Data_Recieved";
			if (isVehicle(senderAddress))
				packetsFromVehicles->Update();
			// TODO check SCF
			NS_LOG_INFO ("Interest is matched, Data packet " << packet->GetUid () << " was issued by me.\n");

		}
		break;
	}
	default:
		NS_LOG_WARN ("Not data packet");
	}
}

void IADDApplication::CancelTimers() {
	NS_LOG_FUNCTION_NOARGS ();

	if (m_transmitTimer.IsRunning()) {

		m_transmitTimer.Remove();
	}
}
void IADDApplication::setCalculators(Ptr<TimeMinMaxAvgTotalCalculator> a,
		Ptr<CounterCalculator<> > b, Ptr<CounterCalculator<> > c,
		Ptr<CounterCalculator<> > d) {
	elapsedTime = a;
	packetsArrived = b;
	packetsFromVehicles = c;
	numberofInterstGenerated = d;
}
bool IADDApplication::isVehicle(Ipv4Address a) {
	stringstream ss;
	ss << a;
	uint32_t i = ss.str().find_last_of('.');
	string s = ss.str().substr(i + 1, ss.str().length());
	uint32_t ip = atoi(s.c_str());
	if (ip >= 1 && ip <= 20)
		return false;
	return true;
}
Vector IADDApplication::getRandomPointOnGrid() {

	Vector center(300,450,0);
	/*
	uint8_t c = UniformVariable().GetInteger(0, 2);
	if (c == 0) {
		center.x = UniformVariable().GetInteger(0, 901);
		center.y = UniformVariable().GetInteger(0, 4) * 300;

	} else {
		center.y = UniformVariable().GetInteger(0, 901);
		center.x = UniformVariable().GetInteger(0, 4) * 300;
	}
*/
	Vector p1(300, 450, 0);
	Vector p2(600, 450, 0);
	Vector p3(450, 0, 0);
	Vector p4(450, 900, 0);
	uint32_t r = UniformVariable().GetInteger(0, 4);
	switch (r) {
	case 0:
		return p1;
	case 1:
		return p2;
	case 2:
		return p3;
	case 3:
		return p4;
	}

	return center;
}
double IADDApplication::getRandomRange() {
	return UniformVariable().GetInteger(150, 151);
}
IADDInterestTypes IADDApplication::getRandomType() {
	uint32_t r = UniformVariable().GetInteger(0, 4);
	switch (r) {
	case 0:
		return TYPE1;
	case 1:
		return TYPE2;
	case 2:
		return TYPE3;
	case 3:
		return TYPE4;
	case 4:
		return TYPE5;
	case 5:
		return TYPE6;
	}
	return TYPE1;
}

void IADDApplication::setTracer(Ptr<IADDPacketTracer> t){
	tracer = t;
}

}
