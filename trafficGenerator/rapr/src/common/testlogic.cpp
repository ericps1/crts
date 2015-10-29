#include <iostream.h>
#include "raprNumberGenerator.h"
#include "map"
#include <stdio.h>
#include "raprDictionary.h"
#include "raprLogicTable.h"

void display(unsigned long value) {
	int c;
	unsigned long mask = 1 << 31;
	cout << value << " = ";
	for (c = 0;c < 32;c++) {
		cout << (value & mask ? '1' : '0');
		value <<= 1;
		if (((c+1) % 8) == 0) cout << ' ';
	}
	cout << endl;
}

int main() {
	raprDictionary *dict;
	raprStringVector *vec2;
	dict=new raprDictionary();
	cout << "Translate : This %IP% talk to %DST%" << endl;
	vec2=(*dict).translate("This %IP% talk to %DST%");
	for (int i=0;i<vec2->size();i++) {
		cout << "Value " << i << ":" << (*vec2)[i] << endl;
	}
	delete dict;
	cout << endl;
	
	raprLogicState state;
	state.fromString("1.23.3.4");
	state.setName("1");
	cout << "Initial State" << endl;
	cout << "State : " << state.toString() << endl;
	cout << "Hash : " << state.toHash() << endl;
	state.setState(5,8);
	cout << "Change state relative to initial" << endl;
	cout << "State 2:" << state.toString() << endl;
	state.setName("2","2.3.4.5.6.7.8");
	state.setState("2");
	cout << "Change state by name" << endl;
	cout << "State 3:" << state.toString() << endl;
	cout << endl;
	
	raprLogicTable table;
	raprLogicTableEntry *tmpEntry;
	tmpEntry=new raprLogicTableEntry();
	tmpEntry->addEntry("DECLARATIVE UDP SRC 5000 DST %DST%/5000 PERIODIC [1 1024]");
	table.setEntry(1,tmpEntry);
	tmpEntry=new raprLogicTableEntry();
	tmpEntry->addEntry("DECLARATIVE UDP SRC 5000 DST %LOCALHOST%/5000 PERIODIC [1 1024]");
	tmpEntry->addEntry("INTERROGATIVE UDP SRC 5000 DST %LOCALHOST%/5000 PERIODIC [1 1024]");
	tmpEntry->setSuccess(1);
	table.setEntry(2,tmpEntry);
	cout << "Logic ID : 1" << endl;
	table.DoLogicID(1);
	cout << "Logic ID : 2" << endl;
	table.DoLogicID(2);
	cout << "Success with Logic ID 2 :" << endl;
	table.DoSuccess(2);
}


