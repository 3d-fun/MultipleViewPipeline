#include <mvp/Algorithm/Dummy.h>

#include <mvp/Algorithm/Dummy/DerivedDummy.h>

#define CHECK_IMPL \
  if (!m_impl) { \
    vw::vw_throw(vw::LogicErr() << "method not defined"); \
  } \


namespace mvp {
namespace algorithm {

Dummy::Dummy(std::string const& type) {
  if (type == "DerivedDummy") {
    m_impl.reset(new DerivedDummy());
  } else {
    vw::vw_throw(vw::LogicErr() << "Unknown Dummy type");
  }
}

void Dummy::void0() {CHECK_IMPL; m_impl->void0();}

void Dummy::void1(int a) {CHECK_IMPL; m_impl->void1(a);}

void Dummy::void2(int a, int b) {CHECK_IMPL; m_impl->void2(a,b);}

int Dummy::function0() {CHECK_IMPL; return m_impl->function0();}

int Dummy::function1(int a) {CHECK_IMPL; return m_impl->function1(a);}

int Dummy::function2(int a, int b) {CHECK_IMPL; return m_impl->function2(a,b);}

int Dummy::x() {CHECK_IMPL; return m_impl->x();}

int Dummy::y() {CHECK_IMPL; return m_impl->y();}

vw::Vector2 Dummy::do_vector(vw::Vector3 const& a) {CHECK_IMPL; return m_impl->do_vector(a); }

}}
