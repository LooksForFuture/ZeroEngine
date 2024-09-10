#ifndef ZENGINE_ZREF
#define ZENGINE_ZREF

#include <memory>

/*Just an extra layer over weak pointer which makes usage and validity check easier*/
template <typename T>
class ZRef {
	std::weak_ptr<T> m_ptr;

public:
	ZRef() {}

	ZRef(std::weak_ptr<T> ptr) {
		m_ptr = ptr;
	}

	//tells us how many shared pointers are using the managed object
	long useCount() {
		return m_ptr.use_count();
	}

	//tells us if it has ever been assigned or not
	bool isEmpty() {
		return !(m_ptr.owner_before(std::weak_ptr<T>{})) && !std::weak_ptr<T>{}.owner_before(m_ptr);
	}

	//releases the reference to the managed object
	void reset() {
		m_ptr.reset();
	}

	//tells us if the managed object is valid -> is usable
	bool isValid() {
		return !m_ptr.expired();
	}

	//tells us if the managed object is expired -> has been destroyed
	bool isExpired() {
		return m_ptr.expired();
	}

	//returns the managed object if available and if not returns nullptr
	T* get() {
		if (m_ptr.expired()) {
			return nullptr;
		}

		return m_ptr.lock().get();
	}

	//returns the managed object if available and if not returns nullptr
	T* operator ->() {
		if (m_ptr.expired()) {
			return nullptr;
		}

		return m_ptr.lock().get();
	}

	template <typename U>
	bool operator == (ZRef<U> other) {
		return (m_ptr.lock().get() == other.m_ptr.lock().get());
	}

	template <typename U>
	bool operator != (ZRef<U> other) {
		return (m_ptr.lock().get() == other.m_ptr.lock().get());
	}

	template <typename U>
	operator ZRef<U>() {
		std::shared_ptr<U> casted = std::reinterpret_pointer_cast<U>(m_ptr.lock());
		return ZRef<U>(std::weak_ptr<U>(casted));
	}
};

#endif // !ZENGINE_ZREF