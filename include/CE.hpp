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
class CE_DSA : public Cognitive_Engine {
public:
	CE_DSA();
	~CE_DSA();
	virtual void execute(void * _args);
	void * custom_members;
};
class CE_Example : public Cognitive_Engine {
public:
	CE_Example();
	~CE_Example();
	virtual void execute(void * _args);
	void * custom_members;
};
class CE_FEC : public Cognitive_Engine {
public:
	CE_FEC();
	~CE_FEC();
	virtual void execute(void * _args);
	void * custom_members;
};
class CE_Hopper : public Cognitive_Engine {
public:
	CE_Hopper();
	~CE_Hopper();
	virtual void execute(void * _args);
	void * custom_members;
};
class CE_DSA_PU : public Cognitive_Engine {
public:
	CE_DSA_PU();
	~CE_DSA_PU();
	virtual void execute(void * _args);
	void * custom_members;
};
class CE_AMC : public Cognitive_Engine {
public:
	CE_AMC();
	~CE_AMC();
	virtual void execute(void * _args);
	void * custom_members;
};
//EDIT END FLAG
#endif

