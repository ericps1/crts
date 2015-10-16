#ifndef _CE_HPP_
#define _CE_HPP_

///////////////////////////////////////////
// Cognitive Engine base class
/// \brief The base class for the custom 
/// cognitive engines built using the ECR
/// (Extensible Cognitive Radio).
///
/// This class is used as the base for the custom
/// (user-defined) cognitive engines (CEs) placed
/// in the cognitive_engines/ directory of the 
/// source tree.  The CEs following
/// this model are event-driven: While the radio is running,
/// if certain events occur as defined in 
/// ExtensibleCognitiveRadio::Event, then the custom-defined
/// execute function (Cognitive_Engine::execute()) will be called.
class Cognitive_Engine{
public:
    Cognitive_Engine();
    ~Cognitive_Engine();
    /// \brief Executes the custom cognitive engine
    /// as defined by the user.
    ///
    /// When writing a custom cognitive engine (CE) using 
    /// the Extensible Cognitive Radio (ECR), this 
    /// function should be defined to contain the 
    /// main processing of the CE. 
    /// An ECR CE is event-driven: 
    /// When the radio is running,
    /// this Cognitive_Engine::execute() function is 
    /// called if certain events, as defined in
    /// ExtensibleCognitiveRadio::Event, occur.
    ///
    /// For more information on how to write a custom CE using 
    /// using the ECR, see TODO:Insert refence here.
    /// Or, for direct examples, refer to the source code of
    /// the reimplementations listed below
    /// (in the cognitive_engines/ directory of the source tree).
    virtual void execute(void * _args);
};

///////////////////////////////////////////
// Custom Cognitive Engine sub classes
///@cond INTERNAL
//EDIT START FLAG
class CE_2_Channel_DSA_Spectrum_Sensing : public Cognitive_Engine {
public:
    CE_2_Channel_DSA_Spectrum_Sensing();
    ~CE_2_Channel_DSA_Spectrum_Sensing();
    virtual void execute(void * _args);
};
class CE_2_Channel_DSA_Link_Reliability : public Cognitive_Engine {
public:
    CE_2_Channel_DSA_Link_Reliability();
    ~CE_2_Channel_DSA_Link_Reliability();
    virtual void execute(void * _args);
};
class CE_Mod_Adaptation : public Cognitive_Engine {
public:
    CE_Mod_Adaptation();
    ~CE_Mod_Adaptation();
    virtual void execute(void * _args);
};
class CE_FEC_Adaptation : public Cognitive_Engine {
public:
    CE_FEC_Adaptation();
    ~CE_FEC_Adaptation();
    virtual void execute(void * _args);
};
class CE_2_Channel_DSA_PU : public Cognitive_Engine {
public:
    CE_2_Channel_DSA_PU();
    ~CE_2_Channel_DSA_PU();
    virtual void execute(void * _args);
};
class CE_Transparent : public Cognitive_Engine {
public:
    CE_Transparent();
    ~CE_Transparent();
    virtual void execute(void * _args);
};
//EDIT END FLAG
///@endcond
#endif

