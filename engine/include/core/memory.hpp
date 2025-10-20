#pragma once

#include <cassert>
#include <atomic>
#include <cstdint>

namespace mental::core::memory
{
struct Object
{
    union
    {
        uint64_t integer;
        void* pointer;
    };

    Object(uint64_t i) : integer(i) {}
    Object(void* p) : pointer(p) {}

    template <typename T>
    operator T*() const
    {
        return static_cast<T*>(pointer);
    }
};

class IObject
{
public:
    virtual Object getNativeObject(uint32_t type) { return nullptr; }
};

class IResource : public IObject
{
protected:
    IResource() = default;
    virtual ~IResource() = default;

public:
    virtual unsigned long addRef() = 0;
    virtual unsigned long release() = 0;

    IResource(const IResource&) = delete;
    IResource(const IResource&&) = delete;
    IResource& operator=(const IResource&) = delete;
    IResource& operator=(const IResource&&) = delete;
};

class ISingleResource : public IObject
{
protected:
    ISingleResource() = default;

public:
    ISingleResource(const ISingleResource&) = delete;
    ISingleResource(const ISingleResource&&) = delete;
    ISingleResource& operator=(const ISingleResource&) = delete;
    ISingleResource& operator=(const ISingleResource&&) = delete;
};

template <typename T>
class RefCountPtr
{
public:
    typedef T InterfaceType;

protected:
    InterfaceType* ptr_;

    void internalAddRef() const noexcept
    {
        if (ptr_ != nullptr)
        {
            ptr_->addRef();
        }
    }

    unsigned long internalRelease() noexcept
    {
        unsigned long ref = 0;
        T* temp = ptr_;

        if (temp != nullptr)
        {
            ptr_ = nullptr;
            ref = temp->release();
        }

        return ref;
    }

public:
    RefCountPtr() noexcept : ptr_(nullptr) {}

    RefCountPtr(std::nullptr_t) noexcept : ptr_(nullptr) {}

    template <class U>
    RefCountPtr(U* other) noexcept : ptr_(other)
    {
        internalAddRef();
    }

    RefCountPtr(const RefCountPtr& other) noexcept : ptr_(other.ptr_) { internalAddRef(); }

    RefCountPtr(RefCountPtr&& other) noexcept : ptr_(nullptr)
    {
        if (this != reinterpret_cast<RefCountPtr*>(&reinterpret_cast<unsigned char&>(other)))
        {
            swap(other);
        }
    }

    ~RefCountPtr() noexcept { internalRelease(); }

    RefCountPtr& operator=(std::nullptr_t) noexcept
    {
        internalRelease();
        return *this;
    }

    RefCountPtr& operator=(T* other) noexcept
    {
        if (ptr_ != other)
        {
            RefCountPtr(other).swap(*this);
        }
        return *this;
    }

    template <typename U>
    RefCountPtr& operator=(U* other) noexcept
    {
        RefCountPtr(other).swap(*this);
        return *this;
    }

    RefCountPtr& operator=(const RefCountPtr& other) noexcept
    {
        if (ptr_ != other.ptr_)
        {
            RefCountPtr(other).swap(*this);
        }
        return *this;
    }

    template <class U>
    RefCountPtr& operator=(const RefCountPtr<U>& other) noexcept
    {
        RefCountPtr(other).swap(*this);
        return *this;
    }

    RefCountPtr& operator=(RefCountPtr&& other) noexcept
    {
        RefCountPtr(static_cast<RefCountPtr&&>(other)).swap(*this);
        return *this;
    }

    template <class U>
    RefCountPtr& operator=(RefCountPtr<U>&& other) noexcept
    {
        RefCountPtr(static_cast<RefCountPtr<U>&&>(other)).Swap(*this);
        return *this;
    }

    void swap(RefCountPtr&& r) noexcept
    {
        T* tmp = ptr_;
        ptr_ = r.ptr_;
        r.ptr_ = tmp;
    }

    void swap(RefCountPtr& r) noexcept
    {
        T* tmp = ptr_;
        ptr_ = r.ptr_;
        r.ptr_ = tmp;
    }

    [[nodiscard]] T* get() const noexcept { return ptr_; }

    operator T*() const { return ptr_; }

    InterfaceType* operator->() const noexcept { return ptr_; }

    T** operator&() { return &ptr_; }

    [[nodiscard]] T* const* getAddressOf() const noexcept { return &ptr_; }

    [[nodiscard]] T** getAddressOf() noexcept { return &ptr_; }

    [[nodiscard]] T** releaseAndGetAddressOf() noexcept
    {
        internalRelease();
        return &ptr_;
    }

    T* detach() noexcept
    {
        T* ptr = ptr_;
        ptr_ = nullptr;
        return ptr;
    }

    // Set the pointer while keeping the object's reference count unchanged
    void attach(InterfaceType* other)
    {
        if (ptr_ != nullptr)
        {
            auto ref = ptr_->release();
            (void)ref;

            // Attaching to the same object only works if duplicate references are being coalesced. Otherwise
            // re-attaching will cause the pointer to be released and may cause a crash on a subsequent dereference.
            assert(ref != 0 || ptr_ != other);
        }

        ptr_ = other;
    }

    // Create a wrapper around a raw object while keeping the object's reference count unchanged
    static RefCountPtr<T> create(T* other)
    {
        RefCountPtr<T> Ptr;
        Ptr.attach(other);
        return Ptr;
    }

    unsigned long reset() { return internalRelease(); }
};

template <class T>
class RefCounter : public T
{
private:
    std::atomic<unsigned long> mRefCount = 1;

public:
    virtual unsigned long addRef() override { return ++mRefCount; }

    virtual unsigned long release() override
    {
        unsigned long result = --mRefCount;
        if (result == 0)
        {
            delete this;
        }
        return result;
    }
};

typedef RefCountPtr<IResource> ResourceHandle;
}  // namespace mental::core::memory
