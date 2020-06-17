#ifndef _MOVABLE_PTR_H
#define _MOVABLE_PTR_H

template <typename T>
class enable_movable_ptr;	//needed to prevent circular dependency

template <typename T> class movable_ptr;

template <typename T> movable_ptr<T> get_movable(enable_movable_ptr<T>& x);

template <typename T>
class movable_ptr
{
public:
	movable_ptr() : next(nullptr), prev(nullptr), trg(nullptr) {}
	movable_ptr(T* obj);
	~movable_ptr();
	movable_ptr(movable_ptr<T>&& other) noexcept;						//move constructor
	movable_ptr(const movable_ptr<T>& other) noexcept;					//copy constructor
	movable_ptr<T>& operator=(movable_ptr<T>&& other) noexcept;			//move assignment
	movable_ptr<T>& operator=(const movable_ptr<T>& other) noexcept;	//copy assignment

	T& operator*() const								{ return *(this->trg); }
	T* operator->() const								{ return this->trg; }
	bool operator!() const								{ return !get(); }
	bool operator==(const movable_ptr<T>& rhs) const	{ return this->trg == rhs.trg; }
	bool operator==(const T* rhs) const					{ return this->trg == rhs; }
	bool operator!=(const movable_ptr<T>& rhs) const	{ return !(this == &rhs); }
	operator bool() const								{ return get(); }

	friend movable_ptr<T> get_movable<>(enable_movable_ptr<T>& x);
	void reset();
	void reset(T* trg);
	T* get() const;

private:
	movable_ptr<T>* next;
	movable_ptr<T>* prev;
	T* trg;

	void SetUp(T* obj);
	void ClearMovable();
	void GetOut();
	void RankYourself(movable_ptr<T>* first);
	friend class enable_movable_ptr<T>;
};

template <typename T>
void movable_ptr<T>::ClearMovable()
{
	trg = nullptr;
	next = nullptr;
	prev = nullptr;
}

template <typename T>
movable_ptr<T>::~movable_ptr()
{
	reset();
}

template <typename T>
movable_ptr<T> get_movable(enable_movable_ptr<T>& x)
{
	return movable_ptr<T>(static_cast<T*>(&x));
}

template <typename T>
movable_ptr<T>::movable_ptr(T* obj)
{
	SetUp(obj);
}

template <typename T>
void movable_ptr<T>::SetUp(T* obj)
{
	if (obj == nullptr)
		return;
	if (obj->first == nullptr)	//this object isnt tracked by any movable_ptr yet
	{
		obj->first = this;
		this->trg = obj;
		this->prev = nullptr;
		this->next = nullptr;
	}
	else  //get set into hierarchy of pointers 
		RankYourself(obj->first);
}

template <typename T>
void movable_ptr<T>::RankYourself(movable_ptr<T>* first)
{
	movable_ptr<T>* tail = first;	//tail will represent the "last" item on linked list

	while (tail->next != nullptr && tail->next != first)	//get to the end
	{
		tail = tail->next;
	}

	tail->next = this;		//update linked list connections with new movable_ptr in it
	this->next = first;
	first->prev = this;
	this->prev = tail;

	this->trg = first->trg;
}

template <typename T>
void movable_ptr<T>::reset()
{
	if (trg != nullptr)	//if true then this movable_ptr is tracking some object
	{
		if (next == nullptr && prev == nullptr)	//if true then this is the only movable_ptr pointing on trg
		{
			trg->first = nullptr;
		}
		else
		{
			GetOut();
		}
	}
	ClearMovable();
}

template <typename T>
void movable_ptr<T>::reset(T* trg)
{
	*this = movable_ptr<T>(trg);
}

template <typename T>
movable_ptr<T>::movable_ptr(movable_ptr<T>&& other) noexcept
{
	this->SetUp(other.trg);	//set yourself to the hierarchy of targets pointers
	other.reset();	//clear moved object
}

template <typename T>
movable_ptr<T>& movable_ptr<T>::operator=(movable_ptr<T>&& other) noexcept
{
	if (this != &other)	//check for self-assignment
	{
		reset();	//clear current working space
		this->SetUp(other.trg);	//set yourself to the hierarchy of targets pointers
		other.reset();	//clear movable_ptr on the right side
	}
	return *this;
}

template <typename T>
movable_ptr<T>::movable_ptr(const movable_ptr<T>& other) noexcept
{
	this->SetUp(other.trg);	//set yourself to the hierarchy of targets pointers
}

template <typename T>
movable_ptr<T>& movable_ptr<T>::operator=(const movable_ptr<T>& other) noexcept
{
	if (this != &other)	//check for self-assignment
	{
		reset();	//clear current working space
		this->SetUp(other.trg);	//set yourself to the hierarchy of targets pointers
	}
	return *this;
}

template <typename T>
T* movable_ptr<T>::get() const
{
	return this->trg;
}

template <typename T>
void movable_ptr<T>::GetOut()
{
	if (trg->first == this)	//i am deleting the first movable_ptr ==> i must fix trg->first pointer
		trg->first = next;

	prev->next = this->next;	//reorganize linked list connections
	next->prev = this->prev;

	if (next == next->next)	//if true then there is only one movable_ptr pointing on trg, set its next and prev to nullptr
	{
		next->next = nullptr;
		prev->prev = nullptr;
	}
}
#endif



#ifndef _ENABLE_MOVABLE_PTR_H
#define _ENABLE_MOVABLE_PTR_H
template <typename T>
class enable_movable_ptr
{
public:
	enable_movable_ptr() : first(nullptr) {}
	~enable_movable_ptr();
	enable_movable_ptr(enable_movable_ptr<T>&& other) noexcept;						//move constructor
	enable_movable_ptr(const enable_movable_ptr<T>& other) noexcept;				//copy constructor
	enable_movable_ptr<T>& operator=(enable_movable_ptr<T>&& other) noexcept;		//move assignment
	enable_movable_ptr<T>& operator=(const enable_movable_ptr<T>& other) noexcept;	//copy assignment

private:
	movable_ptr<T>* first;
	friend class movable_ptr<T>;

	void ClearEnable(); //clear the whole object (reset all movable pointers)
	void SetTarget(T* newTrg, movable_ptr<T>* first);
};

template <typename T>
enable_movable_ptr<T>::~enable_movable_ptr()
{
	ClearEnable();
}

template <typename T>
enable_movable_ptr<T>::enable_movable_ptr(enable_movable_ptr<T>&& other) noexcept
{
	SetTarget(static_cast<T*>(this), other.first);	//update target to all movable pointers pointing on moved object
	this->first = other.first;
	other.first = nullptr;
}

template <typename T>
enable_movable_ptr<T>& enable_movable_ptr<T>::operator=(enable_movable_ptr<T>&& other) noexcept
{
	if (this != &other)		//check for self-assignment
	{
		ClearEnable();	//clear this object(reset all its movable pointers)
		this->first = other.first;
		SetTarget(static_cast<T*>(this), other.first);	//update target to all movable pointers pointing on moved object
		other.first = nullptr;
	}

	return *this;
}

template <typename T>
void enable_movable_ptr<T>::SetTarget(T* newTrg, movable_ptr<T>* first)
{
	if (first == nullptr)	//no movable_ptr is pointing on this object, there is nothing to setup
		return;

	movable_ptr<T>* iter = first;
	iter->trg = newTrg;
	iter = iter->next;
	while (iter != nullptr && iter != first)	//set all movable pointers their new target
	{
		iter->trg = newTrg;
		iter = iter->next;
	}
}

template <typename T>
enable_movable_ptr<T>::enable_movable_ptr(const enable_movable_ptr<T>& other) noexcept
{
	this->first = nullptr;
}

template <typename T>
enable_movable_ptr<T>& enable_movable_ptr<T>::operator=(const enable_movable_ptr<T>& other) noexcept
{
	if (this != &other)		//check for self-assignment
	{
		ClearEnable();	//clear this object
	}

	return *this;
}

template <typename T>
void enable_movable_ptr<T>::ClearEnable()
{
	if (this->first == nullptr)	//nothing is pointing on this object, so there is nothing to clear
		return;

	movable_ptr<T>* iter = first->next;
	movable_ptr<T>* tail = first;
	tail->reset();
	while (iter != nullptr)		//iterate through all movable_ptr and reset them
	{
		tail = iter;
		iter = iter->next;
		tail->reset();
	}

	this->first = nullptr;
}

#endif