template<typename t, typename First, typename ...Args>
struct is_in {
	const static bool value = is_in<t, Args...>::value;
};
template< typename First, typename ...Args>
struct is_in<First, First, Args...> {
	const static bool value = true;
};
template<typename t, typename First>
struct is_in<t, First> {
	const static bool value = false;
};
template<typename ...T>
struct count {

};
template<typename T, typename ...Types>
struct count<T, Types...> {
	static const size_t value = count<Types...>::value + 1;
};
template<>
struct count<> {
	static const size_t value = 0;
};