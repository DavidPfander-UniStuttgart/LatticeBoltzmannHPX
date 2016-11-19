#pragma once

namespace lattice {
namespace util {

namespace detail {

template<size_t min, size_t max, size_t cur, typename T, typename ...Ts>
typename std::enable_if<cur == max, void>::type do_unpack(hpx::util::tuple<Ts...> &t,
    T (&array)[max - min]) {
}

template<size_t min, size_t max, size_t cur, typename T, typename ...Ts>
typename std::enable_if<cur < max, void>::type do_unpack(hpx::util::tuple<Ts...> &t,
    T (&array)[max - min]) {
  array[cur] = hpx::util::get<min + cur>(t);
  do_unpack<min, max, cur + 1>(t, array);
}

}

template<size_t min, size_t max, typename T, typename ...Ts> //, typename ...
void unpack_tuple(hpx::util::tuple<Ts...> &t, T (&array)[max - min]) { //
  detail::do_unpack<min, max, 0, T, Ts...>(t, array);
}

}
}
