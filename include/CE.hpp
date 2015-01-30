#ifndef _CE_HPP_
#define _CE_HPP_

///////////////////////////////////////////
// Cognitive Engine base class
class Cognitive_Engine{
public:
	Cognitive_Engine();
	~Cognitive_Engine();
	virtual void execute(void * _args);
};

///////////////////////////////////////////
// Custom Cognitive Engine sub classes
//EDIT START FLAG
class CE_Example_1 : public Cognitive_Engine {
public:
	CE_Example_1();
	~CE_Example_1();
	virtual void execute(void * _args);
	void * custom_members;
};
class CE_Example_2 : public Cognitive_Engine {
public:
	CE_Example_2();
	~CE_Example_2();
	virtual void execute(void * _args);
	void * custom_members;
};
//EDIT END FLAG
#endif
