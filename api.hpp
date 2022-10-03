#include "dbmanager.h"
#include "interface/employeesinterface.hpp"
#include "interface/departmentinterface.hpp"

template <typename Item, typename Interface>
class RawTableRow {
public:
	RawTableRow(Item item):
	_data(item)
	{}

	RawTableRow(Interface& interface, const int primary_id)
	{
		_data = interface.getByID(primary_id);
	}

	Item _data;

	Item& getRawData()
	{
		return _data;
	}
};



template <typename Item, typename Interface>
class IndependentItem : public RawTableRow<Item, Interface> {
public:
	IndependentItem(Interface& interface, const int primary_id) :
	RawTableRow<Item, Interface>(interface, primary_id)
	{

	}

};

template <typename Item, typename Interface, typename... DependencyItems>
class DependentItem : public RawTableRow<Item, Interface> {
public:

	DependentItem(Interface& interface, const int primary_id, DependencyItems... values) :
	RawTableRow<Item, Interface>(interface, primary_id),
	_dependency_items{std::forward_as_tuple(values...)}
	{

	}

	using Tuple = std::tuple<DependencyItems...>;

	template <std::size_t N>
    	typename std::tuple_element<N, Tuple>::type getItem() {
		return std::get<N>(_dependency_items);
	}


	private:

	std::tuple<DependencyItems...> _dependency_items;
};


