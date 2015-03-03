#include "aithreadsafe.h"
#include "aireadwritemutex.h"

#include <iostream>
#include <cassert>
#include <mutex>

template<int size, typename T>
void do_asserts(void)
{
  using namespace thread_safe;
  static_assert(sizeof(T) == sizeof(Bits<T>), "sizeof(Bits<T) != sizeof(T)!");
  static_assert(alignof(T) == alignof(AIThreadSafe<T, policy::OneThread>), "alignof(AIThreadSafe<T, OneThread>) != alignof(T)!");
  static_assert(alignof(AIThreadSafe<T, policy::Primitive<std::mutex>>) % alignof(T) == 0, "alignof(AIThreadSafe<T, Primitive<std::mutex>>) is not a multiple of alignof(T)!");
}

template<int size>
void do_size_test(void)
{
  struct T0 { char a[size]; };
  struct T1 { char x; char a[size]; };
  struct T2 { short x; char a[size]; };
  struct T4 { int32_t x; char a[size]; };
  struct T8 { int64_t x; char a[size]; };

  do_asserts<size, AIThreadSafe<T1, thread_safe::policy::OneThread>>();
  do_asserts<size, AIThreadSafe<T2, thread_safe::policy::OneThread>>();
  do_asserts<size, AIThreadSafe<T4, thread_safe::policy::OneThread>>();
  do_asserts<size, AIThreadSafe<T8, thread_safe::policy::OneThread>>();
}

enum state_type { unlocked, readlocked, writelocked };

class TestRWMutex
{
  private:
    state_type m_state;

  public:
    TestRWMutex() : m_state(unlocked) { }

    void rdlock() { assert(m_state == unlocked); m_state = readlocked; }
    void rdunlock() { assert(m_state == readlocked); m_state = unlocked; }
    void wrlock() { assert(m_state == unlocked); m_state = writelocked; }
    void wrunlock() { assert(m_state == writelocked); m_state = unlocked; }
    void rd2wrlock() { assert(m_state == readlocked); m_state = writelocked; }
    void wr2rdlock() { assert(m_state == writelocked); m_state = readlocked; }
    void rd2wryield() { }

  public:
    bool is_unlocked() const { return m_state == unlocked; }
    bool is_readlocked() const { return m_state == readlocked; }
    bool is_writelocked() const { return m_state == writelocked; }
};

struct Foo {
  int x;
};

typedef AIThreadSafe<Foo, thread_safe::policy::ReadWrite<TestRWMutex>> foo_t;

// Hack access to TestRWMutex.
class LockAccess : public foo_t
{
  public:
    bool is_unlocked() const { return this->m_read_write_mutex.is_unlocked(); }
    bool is_readlocked() const { return this->m_read_write_mutex.is_readlocked(); }
    bool is_writelocked() const { return this->m_read_write_mutex.is_writelocked(); }
};

bool is_unlocked(foo_t const& wrapper)
{
  return static_cast<LockAccess const&>(wrapper).is_unlocked();
}

bool is_readlocked(foo_t const& wrapper)
{
  return static_cast<LockAccess const&>(wrapper).is_readlocked();
}

bool is_writelocked(foo_t const& wrapper)
{
  return static_cast<LockAccess const&>(wrapper).is_writelocked();
}

// Hack access to m_wrapper.
class Access : public foo_t::crat
{
  public:
    bool is_unlocked() const { return ::is_unlocked(this->m_wrapper); }
    bool is_readlocked() const { return ::is_readlocked(this->m_wrapper); }
    bool is_writelocked() const { return ::is_writelocked(this->m_wrapper); }
};

bool is_unlocked(foo_t::crat const& access)
{
  Access const& a = static_cast<Access const&>(access);
  return a.is_unlocked();
}

bool is_readlocked(foo_t::crat const& access)
{
  Access const& a = static_cast<Access const&>(access);
  return a.is_readlocked();
}

bool is_writelocked(foo_t::crat const& access)
{
  Access const& a = static_cast<Access const&>(access);
  return a.is_writelocked();
}

void func_read_const(foo_t::crat const& access)
{
  std::cout << access->x << std::endl;
  assert(is_readlocked(access) || is_writelocked(access));
}

void func_read_and_then_write(foo_t::rat& access)
{
  std::cout << access->x << std::endl;
  assert(is_readlocked(access) || is_writelocked(access));
  foo_t::wat write_access(access);				// This might throw if is_readlocked(access).
  write_access->x = 6;
  assert(is_writelocked(access));
}

void func_write(foo_t::wat const& access)
{
  access->x = 5;
  assert(is_writelocked(access));
}

int main()
{
  std::cout << "Testing size and alignment... " << std::flush;
  do_size_test<1>();
  do_size_test<2>();
  do_size_test<3>();
  do_size_test<4>();
  do_size_test<5>();
  do_size_test<6>();
  do_size_test<7>();
  do_size_test<8>();
  do_size_test<9>();
  do_size_test<10>();
  do_size_test<11>();
  do_size_test<12>();
  do_size_test<13>();
  do_size_test<14>();
  do_size_test<15>();
  do_size_test<16>();
  do_size_test<17>();
  do_size_test<18>();
  do_size_test<19>();
  do_size_test<20>();
  do_size_test<21>();
  do_size_test<22>();
  do_size_test<23>();
  do_size_test<24>();
  do_size_test<25>();
  std::cout << "OK" << std::endl;

  // ThreadSafe compile tests.
  struct A { int x; };
  typedef AIThreadSafe<A, thread_safe::policy::OneThread> onethread_t;
  typedef AIThreadSafe<A, thread_safe::policy::Primitive<std::mutex>> primitive_t;
  typedef AIThreadSafe<A, thread_safe::policy::ReadWrite<AIReadWriteMutex>> readwrite_t;

  onethread_t onethread;
  primitive_t primitive;
  readwrite_t readwrite;

  // Writing
  {
    onethread_t::wat onethread_w(onethread);
    onethread_w->x = 111;
  }
  {
    primitive_t::wat primitive_w(primitive);
    primitive_w->x = 222;
  }
  {
    readwrite_t::wat readwrite_w(readwrite);
    readwrite_w->x = 333;
  }

  // Reading
  {
    onethread_t::rat onethread_r(onethread);
    assert(onethread_r->x == 111);
  }
  {
    primitive_t::rat primitive_r(primitive);
    assert(primitive_r->x == 222);
  }
  {
    readwrite_t::rat readwrite_r(readwrite);
    assert(readwrite_r->x == 333);
  }

  // Conversions, write to read access.
#if 0
  {
    onethread_t::wat onethread_w(onethread);
    onethread_w->x = 444;
    onethread_t::rat onethread_r(onethread_w);
    assert(onethread_r->x == 444);
  }
  {
    primitive_t::wat primitive_w(primitive);
    primitive_w->x = 555;
    primitive_t::rat primitive_r(primitive_w);
    assert(primitive_r->x == 555);
  }
  {
    readwrite_t::wat readwrite_w(readwrite);
    readwrite_w->x = 666;
    readwrite_t::rat readwrite_r(readwrite_w);
    assert(readwrite_r->x == 666);
  }

  // Conversions, read to write access.
  {
    onethread_t::rat onethread_r(onethread);
    onethread_t::wat onethread_w(onethread_r);
    onethread_w->x = 777;
    assert(onethread_r->x == 777);
  }
  {
    primitive_t::rat primitive_r(primitive);
    primitive_t::wat primitive_w(primitive_r);
    primitive_w->x = 888;
    assert(primitive_r->x == 888);
  }
  {
    readwrite_t::rat readwrite_r(readwrite);
    readwrite_t::wat readwrite_w(readwrite_r);
    readwrite_w->x = 999;
    assert(readwrite_r->x == 999);
  }
#endif

  foo_t wrapper;
  foo_t const& const_wrapper(wrapper);

  // Things that should compile.
  {
    // Getting write access to non-const wrapper.
    foo_t::wat write_access(wrapper);
    write_access->x = 3;
    assert(is_writelocked(wrapper));
  }
  assert(is_unlocked(wrapper));
  {
    // Getting read only access to const wrapper.
    foo_t::crat read_access(const_wrapper);
    std::cout << read_access->x << std::endl;
    assert(is_readlocked(wrapper));
  }
  assert(is_unlocked(wrapper));
  {
    // Creating a crat from a non-const wrapper.
    foo_t::crat read_access(wrapper);
    std::cout << read_access->x << std::endl;
    assert(is_readlocked(wrapper));
  }
  assert(is_unlocked(wrapper));
  {
    // Getting first read access to non-const wrapper, and then write access.
    for(;;)
    {
      try
      {
	foo_t::rat read_access(wrapper);
	std::cout << read_access->x << std::endl;
	assert(is_readlocked(wrapper));
	foo_t::wat write_access(read_access);		// This might throw.
	write_access->x = 4;
	assert(is_writelocked(wrapper));
      }
      catch (std::exception const&)
      {
	wrapper.rd2wryield();				// Block until the other thread that tries to convert a read to write lock succeeded.
	// Try again.
	continue;
      }
      break;
    }
  }
  assert(is_unlocked(wrapper));
  {
    // Passing a crat to func_read_const
    foo_t::crat read_access_const(const_wrapper);	// OK
    func_read_const(read_access_const);	
    assert(is_readlocked(wrapper));
  }
  assert(is_unlocked(wrapper));
  {
    // Passing a rat to func_read_const
    foo_t::rat read_access(wrapper);			// OK
    func_read_const(read_access);
    assert(is_readlocked(wrapper));
  }
  assert(is_unlocked(wrapper));
  {
    // Passing a wat to func_read_const
    foo_t::wat write_access(wrapper);			// OK
    func_read_const(write_access);
    assert(is_writelocked(wrapper));
  }
  assert(is_unlocked(wrapper));
  {
    for(;;)
    {
      try
      {
	// Passing a rat to func_read
	foo_t::rat read_access(wrapper);		// OK
	func_read_and_then_write(read_access);		// This might throw.
	assert(is_readlocked(wrapper));
      }
      catch(std::exception const&)
      {
	wrapper.rd2wryield();
	continue;
      }
      break;
    }
  }
  assert(is_unlocked(wrapper));
  {
    // Passing a wat to func_read
    foo_t::wat write_access(wrapper);			// OK
    func_read_and_then_write(write_access);
    assert(is_writelocked(wrapper));
  }
  assert(is_unlocked(wrapper));
  {
    // Passing a wat to func_write
    foo_t::wat write_access(wrapper);			// OK
    func_write(write_access);
    assert(is_writelocked(wrapper));
  }
  assert(is_unlocked(wrapper));

  std::cout << "Success!" << std::endl;

  // Things that should not compile:
#ifdef TEST1
  {
    // Getting write access to a const wrapper.
    foo_t::wat fail(const_wrapper);			// TEST1 FAIL (error: no matching function for call to ‘thread_safe::WriteAccess<AIThreadSafe<Foo, thread_safe::policy::ReadWrite<TestRWMutex> > >::WriteAccess(const foo_t&)’)
  }
#endif
#ifdef TEST2
  {
    // Creating a rat from a const wrapper.
    foo_t::rat fail(const_wrapper);			// TEST2 FAIL (error: no matching function for call to ‘thread_safe::ReadAccess<AIThreadSafe<Foo, thread_safe::policy::ReadWrite<TestRWMutex> > >::ReadAccess(const foo_t&)’)
  }
#endif
#ifdef TEST3
  {
    // Getting write access from wat.
    foo_t::wat write_access(wrapper);			// OK
    foo_t::wat fail(write_access);			// TEST3 FAIL (error: use of deleted function ‘thread_safe::WriteAccess<AIThreadSafe<Foo, thread_safe::policy::ReadWrite<TestRWMutex> > >::WriteAccess(const thread_safe::WriteAccess<AIThreadSafe<Foo, thread_safe::policy::ReadWrite<TestRWMutex> > >&)’)
  }
#endif
#ifdef TEST4
  {
    // Getting write access from crat.
    foo_t::crat read_access_const(const_wrapper);	// OK
    foo_t::wat fail(read_access_const);			// TEST4 FAIL (error: no matching function for call to ‘thread_safe::WriteAccess<AIThreadSafe<Foo, thread_safe::policy::ReadWrite<TestRWMutex> > >::WriteAccess(AIThreadSafe<Foo, thread_safe::policy::ReadWrite<TestRWMutex> >::crat&)’)
  }
#endif
#ifdef TEST5
  {
    // Write to something that you only have read access too.
    foo_t::crat read_access_const(const_wrapper);	// OK
    read_access_const->x = -1;				// TEST5 FAIL (error: assignment of member ‘Foo::x’ in read-only object)
  }
#endif
#ifdef TEST6
  {
    // Write to something that you only have read access too.
    foo_t::rat read_access(wrapper);			// OK
    read_access->x = -1;				// TEST6 FAIL (error: assignment of member ‘Foo::x’ in read-only object)
  }
#endif
#ifdef TEST7
  {
    // Create crat from crat.
    foo_t::crat read_access_const(const_wrapper);	// OK
    foo_t::crat fail(read_access_const);		// TEST7 FAIL (error: ‘thread_safe::ConstReadAccess<WRAPPER>::ConstReadAccess(const thread_safe::ConstReadAccess<WRAPPER>&) [with WRAPPER = AIThreadSafe<Foo, thread_safe::policy::ReadWrite<TestRWMutex> >]’ is private)
  }
#endif
#ifdef TEST8
  {
    // Create crat from rat.
    foo_t::rat read_access(wrapper);			// OK
    foo_t::crat fail(read_access);			// TEST8 FAIL (error: ‘thread_safe::ConstReadAccess<WRAPPER>::ConstReadAccess(const thread_safe::ConstReadAccess<WRAPPER>&) [with WRAPPER = AIThreadSafe<Foo, thread_safe::policy::ReadWrite<TestRWMutex> >]’ is private)
  }
#endif
#ifdef TEST9
  {
    // Create crat from wat.
    foo_t::wat write_access(wrapper);			// OK
    foo_t::crat fail(write_access);			// TEST9 FAIL (error: ‘thread_safe::ConstReadAccess<WRAPPER>::ConstReadAccess(const thread_safe::ConstReadAccess<WRAPPER>&) [with WRAPPER = AIThreadSafe<Foo, thread_safe::policy::ReadWrite<TestRWMutex> >]’ is private)
  }
#endif
#ifdef TEST10
  {
    // Create rat from crat.
    foo_t::crat read_access_const(const_wrapper);	// OK
    foo_t::rat fail(read_access_const);			// TEST10 FAIL (error: no matching function for call to ‘thread_safe::ReadAccess<AIThreadSafe<Foo, thread_safe::policy::ReadWrite<TestRWMutex> > >::ReadAccess(AIThreadSafe<Foo, thread_safe::policy::ReadWrite<TestRWMutex> >::crat&)’)
  }
#endif
#ifdef TEST11
  {
    // Create rat from rat.
    foo_t::rat read_access(wrapper);			// OK
    foo_t::rat fail(read_access);			// TEST11 FAIL (error: use of deleted function ‘thread_safe::ReadAccess<AIThreadSafe<Foo, thread_safe::policy::ReadWrite<TestRWMutex> > >::ReadAccess(const thread_safe::ReadAccess<AIThreadSafe<Foo, thread_safe::policy::ReadWrite<TestRWMutex> > >&)’)
  }
#endif
#ifdef TEST12
  {
    // Create rat from wat.
    foo_t::wat write_access(wrapper);			// OK
    foo_t::rat fail(write_access);			// TEST12 FAIL (error: use of deleted function ‘thread_safe::ReadAccess<AIThreadSafe<Foo, thread_safe::policy::ReadWrite<TestRWMutex> > >::ReadAccess(const thread_safe::ReadAccess<AIThreadSafe<Foo, thread_safe::policy::ReadWrite<TestRWMutex> > >&)’)
  }
#endif
#ifdef TEST13
  {
    // Passing a crat to func_read.
    foo_t::crat read_access_const(const_wrapper);	// OK
    func_read_and_then_write(read_access_const);	// TEST13 FAIL (error: invalid initialization of reference of type ‘AIThreadSafe<Foo, thread_safe::policy::ReadWrite<TestRWMutex> >::rat& {aka thread_safe::ReadAccess<AIThreadSafe<Foo, thread_safe::policy::ReadWrite<TestRWMutex> > >&}’ from expression of type ‘AIThreadSafe<Foo, thread_safe::policy::ReadWrite<TestRWMutex> >::crat {aka thread_safe::ConstReadAccess<AIThreadSafe<Foo, thread_safe::policy::ReadWrite<TestRWMutex> > >}’)
  }
#endif
#ifdef TEST14
  {
    // Passing a crat to func_write.
    foo_t::crat read_access_const(const_wrapper);	// OK
    func_write(read_access_const);			// TEST14 FAIL (error: invalid initialization of reference of type ‘const wat& {aka const thread_safe::WriteAccess<AIThreadSafe<Foo, thread_safe::policy::ReadWrite<TestRWMutex> > >&}’ from expression of type ‘AIThreadSafe<Foo, thread_safe::policy::ReadWrite<TestRWMutex> >::crat {aka thread_safe::ConstReadAccess<AIThreadSafe<Foo, thread_safe::policy::ReadWrite<TestRWMutex> > >}’)
  }
#endif
#ifdef TEST15
  {
    // Passing a rat to func_write.
    foo_t::rat read_access(wrapper);			// OK
    func_write(read_access);				// TEST15 FAIL (error: invalid initialization of reference of type ‘const wat& {aka const thread_safe::WriteAccess<AIThreadSafe<Foo, thread_safe::policy::ReadWrite<TestRWMutex> > >&}’ from expression of type ‘AIThreadSafe<Foo, thread_safe::policy::ReadWrite<TestRWMutex> >::rat {aka thread_safe::ReadAccess<AIThreadSafe<Foo, thread_safe::policy::ReadWrite<TestRWMutex> > >}’)
  }
#endif
#ifdef TEST16
#error That was the last test
#endif
}
