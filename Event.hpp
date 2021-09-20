#pragma once
#include <list>
#include <string>

template<class ...TArgs>
class IInvocable
{
public:
	virtual void invoke(TArgs&&...) = 0;
	virtual const void* getObjectPtr() const = 0;
	virtual const std::string getFunctionPtr() const = 0;
};

template<class ...TArgs>
bool operator ==(const IInvocable<TArgs...>& first, const IInvocable<TArgs...>& second)
{
	return first.getFunctionPtr() == second.getFunctionPtr() &&
		second.getObjectPtr() == second.getObjectPtr();
}

template<class ...TArgs>
class InvocableStaticFunction : public IInvocable<TArgs...>
{
	using TFuncPtr = void (*) (TArgs...);

public:
	InvocableStaticFunction(TFuncPtr function) :
		_function(function)
	{	};

	const void* getObjectPtr() const override
	{
		return nullptr;
	}

	const std::string getFunctionPtr() const override
	{
		return std::string(static_cast<const char*>(static_cast<const void*>(&_function)));
	}

private:
	TFuncPtr _function;

	void invoke(TArgs&&... args) override
	{
		(*_function)(std::forward<TArgs>(args)...);
	}
};

template<class TObject, class ...TArgs>
class InvocableMember : public IInvocable<TArgs...>
{
	using TFuncPtr = void (TObject::*) (TArgs...);

public:
	InvocableMember(TFuncPtr function, TObject* object) :
		_function(function),
		_object(object)
	{	};

	const void* getObjectPtr() const override
	{
		return static_cast<const void*>(_object);
	}

	const std::string getFunctionPtr() const override
	{
		return std::string(static_cast<const char*>(static_cast<const void*>(&_function)));
	}

private:
	TObject* _object;
	TFuncPtr _function;

	void invoke(TArgs&&... args) override
	{
		(_object->*_function)(std::forward<TArgs>(args)...);
	}
};

template<class ...TArgs>
class Event
{
	using TIInvocablePtr = std::unique_ptr<IInvocable<TArgs...>>;

public:
	Event() = default;
	
	template<class TObject>
	void attach(void (TObject::*function) (TArgs...), TObject* object)
	{
		_invocables.push_back(TIInvocablePtr(new InvocableMember<TObject, TArgs...>(function, object)));
	}

	void attach(void (*function) (TArgs...))
	{
		_invocables.push_back(TIInvocablePtr(new InvocableStaticFunction<TArgs...>(function)));
	}

	template<class TObject>
	void detach(void (TObject::* function) (TArgs...), TObject* object)
	{
		removeInvocable(InvocableMember<TObject, TArgs...>(function, object));
	}

	void detach(void (*function) (TArgs...))
	{
		removeInvocable(InvocableStaticFunction<TArgs...>(function));
	}

	void operator()(TArgs&&... args)
	{
		for(auto& invocable : _invocables)
			invocable->invoke(std::forward<TArgs>(args)...);
	}

private:
	std::list<TIInvocablePtr> _invocables;

	void removeInvocable(const IInvocable<TArgs...>& removable)
	{
		for(auto it = _invocables.begin(); it != _invocables.end(); ++it)
		{
			if (*(*(it)) == removable)
			{
				_invocables.erase(it);
				break;
			}
		}
	}
};