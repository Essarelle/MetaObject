#pragma once
#include "MetaObject/Signals/TypedSlot.hpp"
#include "MetaObject/Signals/TypedSignalRelay.hpp"
#include "MetaObject/Logging/Log.hpp"
#include "MetaObject/Signals/Connection.hpp"
namespace mo
{
    template<class Sig> class TypedSignal;
	template<class R, class...T>
	TypedSignal<R(T...)>::TypedSignal()
	{
		(void*)&_relay_factory;
	}

    template<class R, class...T> 
	R TypedSignal<R(T...)>::operator()(T... args)
    {
		if (_typed_relay)
		{
			return (*_typed_relay)(this, args...);
		}
		THROW(debug) << "Not connected to a signal relay";
    }

    template<class R, class...T> 
	TypeInfo TypedSignal<R(T...)>::GetSignature() const
    {
        return TypeInfo(typeid(R(T...)));
    }

	template<class R, class...T>
	std::shared_ptr<Connection> TypedSignal<R(T...)>::Connect(ISlot* slot)
	{
		return slot->Connect(this);
	}

	template<class R, class...T>
	std::shared_ptr<Connection> TypedSignal<R(T...)>::Connect(std::shared_ptr<ISignalRelay>& relay)
	{
		if (relay == nullptr)
		{
			relay.reset(new TypedSignalRelay<R(T...)>());
		}
		auto typed = std::dynamic_pointer_cast<TypedSignalRelay<R(T...)>>(relay);
		if (typed)
			return Connect(typed);
		std::shared_ptr<Connection>();
	}

	template<class R, class...T>
	std::shared_ptr<Connection> TypedSignal<R(T...)>::Connect(std::shared_ptr<TypedSignalRelay<R(T...)>>& relay)
	{
		if (relay == nullptr)
		{
			relay.reset(new TypedSignalRelay<R(T...)>());
		}
		if (relay != _typed_relay)
		{
			_typed_relay = relay;
			return std::shared_ptr<Connection>(new SignalConnection(this, relay));
		}
		std::shared_ptr<Connection>();
	}

	template<class R, class...T>
	bool TypedSignal<R(T...)>::Disconnect()
	{
		if (_typed_relay)
		{
			_typed_relay.reset();
			return true;
		}
		return false;
	}

	template<class R, class...T>
	bool TypedSignal<R(T...)>::Disconnect(ISlot* slot)
	{
		if (_typed_relay)
		{
			if (_typed_relay->_slot == slot)
			{
				_typed_relay.reset();
				return true;
			}
		}
		return false;
	}

	template<class R, class...T>
	bool TypedSignal<R(T...)>::Disconnect(std::weak_ptr<ISignalRelay> relay_)
	{
		auto relay = relay_.lock();
		if (_typed_relay == relay)
		{
			_typed_relay.reset();
			return true;
		}
		return false;
	}

	template<class R, class...T> SignalRelayFactory<R(T...)> TypedSignal<R(T...)>::_relay_factory;
	// ---------------------------------------------------------------------
	// void specialization 
	template<class...T> 
	TypedSignal<void(T...)>::TypedSignal()
	{
		(void*)&_relay_factory;
	}

	template<class...T>
	void TypedSignal<void(T...)>::operator()(T... args)
	{
		for (auto& relay : _typed_relays)
		{
			if (relay)
			{
				(*relay)(this, args...);
			}
		}
	}

	template<class...T>
	TypeInfo TypedSignal<void(T...)>::GetSignature() const
	{
		return TypeInfo(typeid(void(T...)));
	}

	template<class...T>
	std::shared_ptr<Connection> TypedSignal<void(T...)>::Connect(ISlot* slot)
	{
		return slot->Connect(this);
	}

	template<class...T>
	std::shared_ptr<Connection> TypedSignal<void(T...)>::Connect(std::shared_ptr<ISignalRelay>& relay)
	{
		if (relay == nullptr)
		{
			relay.reset(new TypedSignalRelay<void(T...)>());
		}
		auto typed = std::dynamic_pointer_cast<TypedSignalRelay<void(T...)>>(relay);
		if (typed)
			return Connect(typed);
		std::shared_ptr<Connection>();
	}

	template<class...T>
	std::shared_ptr<Connection> TypedSignal<void(T...)>::Connect(std::shared_ptr<TypedSignalRelay<void(T...)>>& relay)
	{
		if (relay == nullptr)
		{
			relay.reset(new TypedSignalRelay<void(T...)>());
		}
		auto itr = std::find(_typed_relays.begin(), _typed_relays.end(), relay);
		if (itr == _typed_relays.end())
		{
			_typed_relays.push_back(relay);
			return std::shared_ptr<Connection>(new SignalConnection(this, relay));
		}
		std::shared_ptr<Connection>();
	}

	template<class...T>
	bool TypedSignal<void(T...)>::Disconnect()
	{
		if (_typed_relays.size())
		{
			_typed_relays.clear();
			return true;
		}
		return false;
	}

	template<class...T>
	bool TypedSignal<void(T...)>::Disconnect(ISlot* slot_)
	{
		for (auto relay = _typed_relays.begin(); relay != _typed_relays.end(); ++relay)
		{
			for (auto& slot : (*relay)->_slots)
			{
				if (slot == slot_)
				{
					_typed_relays.erase(relay);
					return true;
				}
			}
		}
		return false;
	}

	template<class...T>
	bool TypedSignal<void(T...)>::Disconnect(std::weak_ptr<ISignalRelay> relay_)
	{
		auto relay = relay_.lock();
		auto itr = std::find(_typed_relays.begin(), _typed_relays.end(), relay);
		if (itr != _typed_relays.end())
		{
			_typed_relays.erase(itr);
			return true;
		}
		return false;
	}

	template<class...T> SignalRelayFactory<void(T...)> TypedSignal<void(T...)>::_relay_factory;
}