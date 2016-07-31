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

https://github.com/dtmoodie/MetaObject
*/
#pragma once
#include "MetaObject/Detail/Export.hpp"
#include "MetaObject/Detail/TypeInfo.h"
#include "MetaObject/Signals/TypedSignal.hpp"
#include <boost/core/noncopyable.hpp>
#include <boost/parameter/name.hpp>

#include <string>
#include <mutex>
#include <memory>
namespace mo
{
    class Context;
	class Connection;
	class ISignal;
	template<class T> class TypedSignal;
	class TypeInfo;
	template<class T> class TypedSlot;
	class ISignalRelay;
	template<class T> class TypedSignalRelay;
    enum ParameterType
    {
        None_e = 0,
        Input_e = 1,
        Output_e = 2,
        State_e = 4,
        Control_e = 8,
        Buffer_e = 16
    };

    class MO_EXPORTS IParameter: boost::noncopyable
    {
    public:
        typedef std::shared_ptr<IParameter> Ptr;
        typedef TypedSlot<void(Context*, IParameter*)> update_f;
        typedef TypedSlot<void(IParameter const*)> delete_f;

		IParameter(const std::string& name_ = "", ParameterType flags_ = Control_e, long long ts = -1, Context* ctx = nullptr);

        virtual ~IParameter();

        IParameter*         SetName(const std::string& name_);
		IParameter*         SetTreeRoot(const std::string& tree_root_);
        IParameter*         SetContext(Context* ctx);
        virtual IParameter* SetTimestamp(long long ts_ = -1);

        const std::string& GetName()      const;
        const std::string  GetTreeName()  const;
        const std::string& GetTreeRoot()  const;
        long long          GetTimestamp() const;
        Context*           GetContext()   const;

		virtual void Subscribe();
		virtual void Unsubscribe();
		bool HasSubscriptions() const;

        // Implemented in concrete type
        virtual TypeInfo     GetTypeInfo() const = 0;

        // Update with the values from another parameter
        virtual bool         Update(IParameter* other);
        virtual std::shared_ptr<IParameter> DeepCopy() const;

        std::shared_ptr<Connection> RegisterUpdateNotifier(update_f* f);
		std::shared_ptr<Connection> RegisterUpdateNotifier(ISlot* f);
		std::shared_ptr<Connection> RegisterUpdateNotifier(std::shared_ptr<ISignalRelay> relay);
		std::shared_ptr<Connection> RegisterUpdateNotifier(std::shared_ptr<TypedSignalRelay<void(Context*, IParameter*)>>& relay);

        std::shared_ptr<Connection> RegisterDeleteNotifier(delete_f* f);
		std::shared_ptr<Connection> RegisterDeleteNotifier(ISlot* f);
		std::shared_ptr<Connection> RegisterDeleteNotifier(std::shared_ptr<ISignalRelay> relay);
		std::shared_ptr<Connection> RegisterDeleteNotifier(std::shared_ptr<TypedSignalRelay<void(IParameter const*)>>& relay);


        // Sets changed to true and emits update signal
        void OnUpdate(Context* ctx = nullptr);
        IParameter* Commit(long long timestamp_= -1, Context* ctx = nullptr);
        
        template<class Archive> void serialize(Archive& ar);
        
		
		template<typename T> T*   GetDataPtr(long long ts_ = -1, Context* ctx = nullptr);
		template<typename T> T    GetData(long long ts_ = -1, Context* ctx = nullptr);
		template<typename T> bool GetData(T& value, long long ts = -1, Context* ctx = nullptr);

        virtual std::recursive_mutex& mtx();

		void SetFlags(ParameterType flags_);
		void AppendFlags(ParameterType flags_);
		bool CheckFlags(ParameterType flag);


        bool modified;
		TypedSignal<void(Context*, IParameter*)> update_signal;
		TypedSignal<void(IParameter const*)>	 delete_signal;
	protected:
		std::string          _name;
		std::string          _tree_root;
		long long            _timestamp;
		std::recursive_mutex _mtx;
		Context*             _ctx; // Context of object that owns this parameter
		int                  _subscribers;
		ParameterType        _flags;
    };

    template<typename Archive> void IParameter::serialize(Archive& ar)
    {
        ar(_name);
        ar(_tree_root);
        ar(_timestamp);
        ar(_flags);
    }

    template<typename T> class ITypedParameter;
    
	template<typename T> T* IParameter::GetDataPtr(long long ts_, Context* ctx)
    {
		if (auto typed = dynamic_cast<ITypedParameter<T>*>(this))
			return typed->GetDataPtr(ts_, ctx);
		return nullptr;
    }

	template<typename T> T IParameter::GetData(long long ts_, Context* ctx)
	{
		if (auto typed = dynamic_cast<ITypedParameter<T>*>(this))
			return typed->GetData(ts_, ctx);
		throw "Bad cast. Requested " << typeid(T).name() << " actual " << GetTypeInfo().name();
	}

	template<typename T> bool GetData(T& value, long long ts, Context* ctx)
	{
		if (auto typed = dynamic_cast<ITypedParameter<T>*>(this))
			return typed->GetData(value, ts_, ctx);
		return false;
	}
}