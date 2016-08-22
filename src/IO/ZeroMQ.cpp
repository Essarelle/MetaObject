#include "MetaObject/IO/ZeroMQ.hpp"
#define HAVE_ZEROMQ

#ifdef HAVE_ZEROMQ
#include "zmq.hpp"
#include "MetaObject/IO/Message.hpp"
using namespace mo;

ZeroMQContext::ZeroMQContext()
{

}
ZeroMQContext* ZeroMQContext::Instance()
{
    static ZeroMQContext g_inst;
    
    return &g_inst;
}

struct ParameterPublisher::impl
{
    std::shared_ptr<IParameter> shared_input;
    IParameter* input;
};

ParameterPublisher::ParameterPublisher()
{
    _pimpl = new impl();
}

ParameterPublisher::~ParameterPublisher()
{
    delete _pimpl;
}

bool ParameterPublisher::GetInput(long long ts)
{
    return false;
}

// This gets a pointer to the variable that feeds into this input
IParameter* ParameterPublisher::GetInputParam()
{
    return nullptr;
}

// Set input and setup callbacks
bool ParameterPublisher::SetInput(std::shared_ptr<IParameter> param)
{
    return false;
}

bool ParameterPublisher::SetInput(IParameter* param)
{
    return false;
}

// Check for correct serialization routines, etc
bool ParameterPublisher::AcceptsInput(std::weak_ptr<IParameter> param) const
{
    return false;
}
bool ParameterPublisher::AcceptsInput(IParameter* param) const
{
    return false;
}
bool ParameterPublisher::AcceptsType(TypeInfo type) const
{
    return false;
}

#else





#endif