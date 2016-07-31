/*
Copyright (c) 2015 Daniel Moodie.
All rights reserved.

Redistribution and use in source and binary forms are permitted
provided that the above copyright notice and this paragraph are
duplicated in all such forms and that any documentation,
advertising materials, and other materials related to such
distribution and use acknowledge that the software was developed
by the Daniel Moodie. The name of
Daniel Moodie may not be used to endorse or promote products derived
from this software without specific prior written permission.
THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.

https://github.com/dtmoodie/parameters
*/
#include "MetaObject/Parameters/IParameter.hpp"
#include "MetaObject/Signals/TypedSignal.hpp"
#include "MetaObject/Signals/TypedSlot.hpp"
#include "MetaObject/Signals/TypedSignalRelay.hpp"
#include <algorithm>

using namespace mo;

IParameter::IParameter(const std::string& name_, ParameterType flags_, long long ts, Context* ctx) :
    _name(name_), 
    _flags(flags_), 
    modified(false), 
    _subscribers(0), 
    _timestamp(ts),
    _ctx(ctx)
{
    
}

IParameter::~IParameter()
{
	delete_signal(this);
}



IParameter* IParameter::SetName(const std::string& name_)
{
    _name = name_;
    return this;
}

IParameter* IParameter::SetTreeRoot(const std::string& treeRoot_)
{
    _tree_root = treeRoot_;
    return this;
}

IParameter* IParameter::SetTimestamp(long long ts)
{
    _timestamp = ts;
    return this;
}

IParameter* IParameter::SetContext(Context* ctx)
{
    _ctx = ctx;
    return this;
}

const std::string& IParameter::GetName() const
{
    return _name;
}

const std::string& IParameter::GetTreeRoot() const
{
    return _tree_root;
}

const std::string IParameter::GetTreeName() const
{
    return _tree_root + ":" + _name;
}

long long IParameter::GetTimestamp() const
{
    return _timestamp;
}

Context* IParameter::GetContext() const
{
    return _ctx;
}

std::shared_ptr<Connection> IParameter::RegisterUpdateNotifier(update_f* f)
{
	return f->Connect(&update_signal);
}

std::shared_ptr<Connection> IParameter::RegisterUpdateNotifier(ISlot* f)
{
	auto typed = dynamic_cast<update_f*>(f);
	if (typed)
	{
		return RegisterUpdateNotifier(typed);
	}
	return std::shared_ptr<Connection>();
}

std::shared_ptr<Connection> IParameter::RegisterUpdateNotifier(std::shared_ptr<ISignalRelay> relay)
{
	auto typed = std::dynamic_pointer_cast<TypedSignalRelay<void(Context*, IParameter*)>>(relay);
	if (typed)
	{
		return RegisterUpdateNotifier(typed);
	}
	return std::shared_ptr<Connection>();
}
std::shared_ptr<Connection> IParameter::RegisterUpdateNotifier(std::shared_ptr<TypedSignalRelay<void(Context*, IParameter*)>>& relay)
{
	return update_signal.Connect(relay);
}

std::shared_ptr<Connection> IParameter::RegisterDeleteNotifier(delete_f* f)
{
	return f->Connect(&delete_signal);
}

std::shared_ptr<Connection> IParameter::RegisterDeleteNotifier(ISlot* f)
{
	auto typed = dynamic_cast<delete_f*>(f);
	if (typed)
	{
		return RegisterDeleteNotifier(typed);
	}
	return std::shared_ptr<Connection>();
}

std::shared_ptr<Connection> IParameter::RegisterDeleteNotifier(std::shared_ptr<ISignalRelay> relay)
{
	auto typed = std::dynamic_pointer_cast<TypedSignalRelay<void(IParameter*)>>(relay);
	if (typed)
	{
		return RegisterDeleteNotifier(typed);
	}
	return std::shared_ptr<Connection>();
}
std::shared_ptr<Connection> IParameter::RegisterDeleteNotifier(std::shared_ptr<TypedSignalRelay<void(IParameter const*)>>& relay)
{
	return delete_signal.Connect(relay);
}


bool IParameter::Update(IParameter* other)
{
    return false;
}

void IParameter::OnUpdate(Context* ctx)
{
    modified = true;
	update_signal(ctx, this);
}

IParameter* IParameter::Commit(long long index_, Context* ctx)
{
    _timestamp= index_;
    modified = true;
	update_signal(ctx, this);
    return this;
}

std::recursive_mutex& IParameter::mtx()
{
    return _mtx;
}
void IParameter::Subscribe()
{
	--_subscribers;
	_subscribers = std::max(0, _subscribers);
}

void IParameter::Unsubscribe()
{
	++_subscribers;
}

bool IParameter::HasSubscriptions() const
{
	return _subscribers != 0;
}
void IParameter::SetFlags(ParameterType flags_)
{
	_flags = flags_;
}

void IParameter::AppendFlags(ParameterType flags_)
{
	_flags = ParameterType(_flags | flags_);
}

bool IParameter::CheckFlags(ParameterType flag)
{
	return _flags & flag;
}
std::shared_ptr<IParameter> IParameter::DeepCopy() const
{
    return std::shared_ptr<IParameter>();
}