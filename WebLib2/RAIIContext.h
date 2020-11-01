#pragma once
typedef void (*RAIIFunction)(void);
/** 
 * Class to control initialization and deinitialization of a variable
 * @param <ctor> the constructor of the resource
 * @param <dtor> the destructor of the resource
 * Deinitialization occurs when all references are destroyed
 */
template<RAIIFunction ctor, RAIIFunction dtor>
class RAIIContext
{
private:
	static long refCount;
public:
	RAIIContext();
	~RAIIContext();
};

template<RAIIFunction ctor, RAIIFunction dtor>
inline RAIIContext<ctor, dtor>::RAIIContext()
{
	if (refCount++ == 0) ctor();
}

template<RAIIFunction ctor, RAIIFunction dtor>
inline RAIIContext<ctor, dtor>::~RAIIContext()
{
	if (--refCount == 0) dtor();
}
template<RAIIFunction ctor, RAIIFunction dtor>
long RAIIContext<ctor, dtor>::refCount = 0;
