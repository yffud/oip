#include "messages.h"
#include <iostream>
using std::cerr;

packet::packet(Uint8* buffer, int s)
:data(buffer),size(s) 
{
	if (!s) 
	{
		setreserved(); 
		setversion();
	} 
	else if (getversion() != VERSION)
		cerr << "Version mismatch. Recieved packet of version " << getversion() << " but expected version " << VERSION << "\n";
	
}

setuppacket::setuppacket(Uint8*buffer, int s)
:packet(buffer, s),setupsize(0)
{
	settype(SETUP);
}

void setuppacket::setopts(const map<string, string> &m)
{
	int pos = packet::getsize() + 2;
	//put16inc(pos, m.size());
	int optcount = 0;
	map<string, string>::const_iterator i = m.begin();
	for (;i != m.end(); i++)
	{
		//check to see if the data they want to add is too long
		if (pos + (*i).first.length() + (*i).second.length() + 4 >= MAXPACKET)
			continue; //skip it. 
		putstringinc(pos, (*i).first);
		putstringinc(pos, (*i).second);
		optcount++;
	}
	put16(packet::getsize(), optcount);
		
	setupsize = pos - packet::getsize();
}

void setuppacket::getopts(map <string, string> &m)
{
	int pos = packet::getsize();
	int count = get16inc(pos);
	int i;
	for (i = 0; i < count; i++)
	{
		string s = getstringinc(pos);
		m[s] = getstringinc(pos);
	}
	setupsize = pos - packet::getsize();
}

datapacket::datapacket(Uint8*buffer, int s)
:packet(buffer, s)
{
	if (!s) 
	{
		settype(DUMPPACKETS);
		size = packet::getsize();
		put16(size, 0);		
	}
}

bool datapacket::adddata(Uint32 src, Uint32 dst, Uint32 color, Uint32 count)
{
	int pos = getsize();
	if (MAXPACKET < pos + 16)
		return false; //unable to add more data
	put32inc(pos, src);
	put32inc(pos, dst);
	put32inc(pos, color);
	put32inc(pos, count);
	size = pos;
	inccount();
	return true;
}

void datapacket::dumpdata(packetmanager&pm)
{
	int pos = packet::getsize();
	int i;
	int c = count();
	for (i = 0; i < c; i++)
	{
		Uint32 s,d,c,count;
		s = get32inc(pos);
		d = get32inc(pos);
		c = get32inc(pos);
		count = get32inc(pos);
		pm.addpacket(s,d,c,count);
	}
}
