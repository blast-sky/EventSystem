#pragma once
#include <list>

template<class ...TArgs>
class IInvocable
{
public:
	virtual void invoke(TArgs&&...) = 0;
	virtual size_t getHash() const = 0;
	virtual ~IInvocable() {};
};

template<class ...TArgs>
class InvocableStaticFunction : public IInvocable<TArgs...>
{
	using TFuncPtr = void (*) (TArgs...);

public:
	InvocableStaticFunction(TFuncPtr function) :
		_function(function)
	{	};

	size_t getHash() const override
	{
		std::hash<std::string> hasher;

		size_t hashSum = 0;
		hashSum += hasher(static_cast<char*>(static_cast<void*>(_function)));
		return hashSum;
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

	size_t getHash() const override
	{
		std::hash<std::string> hasher;

		size_t hashSum = 0;
		hashSum += hasher(static_cast<char*>(static_cast<void*>(_object)));
		hashSum += hasher(static_cast<const char*>(static_cast<const void*>(&_function)));
		return hashSum;
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
		_invocables.remove_if([function, object](const TIInvocablePtr& invocable)
			{
				return invocable->getHash() == InvocableMember<TObject, TArgs...>(function, object).getHash();
			});
	}

	void detach(void (*function) (TArgs...))
	{
		_invocables.remove_if([function](const TIInvocablePtr& invocable)
			{
				return invocable->getHash() == InvocableStaticFunction<TArgs...>(function).getHash();
			});
	}

	void operator()(TArgs&&... args)
	{
		for(auto& invocable : _invocables)
			invocable->invoke(std::forward<TArgs>(args)...);
	}

private:
	std::list<TIInvocablePtr> _invocables;
};