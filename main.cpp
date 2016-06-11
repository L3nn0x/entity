#include <string>
#include <vector>
#include <set>
#include <memory>

template <class T>
struct BaseProperty_comp {
	typedef std::true_type is_transparent;

	struct Helper {
		std::string id;
		Helper() = default;
		Helper(Helper const&) = default;
		Helper(T *p) : id(p->id) {}
		template <class U, class... Ts>
		Helper(std::unique_ptr<U, Ts...> const &up) : id(up->id) {}
		bool operator<() (Helper o) const {
			return std::less<std::string>() (id, o.id);
		}
	};

	bool operator()(Helper const &&lhs, Helper const &&rhs) const {
		return lhs < rhs;
	}
};

template <typename T>
using owning_set = std::set<std::unique_ptr<T>, BaseProperty_comp<T>>;

class BaseProperty {
	public:
		BaseProperty(const std::string &id) : id(id) {}
		virtual ~BaseProperty() {}

		bool operator==(const BaseProperty &p) const {
			return id == p.id;
		}
		
		std::string id;
};

template <typename T>
class Property : public BaseProperty {
	public:
		Property(const std::string &id, T&& data)
			: BaseProperty(id), value(std::move(data)) {}
		virtual ~Property() {}

		T value;
};

class Entity {
	public:
		using Functor = std::function<void(Entity*)>;
		Entity(Functor registerP) : reg(registerP) {}

		template <typename T>
		void setProperty(const std::string &id, T&& data) {
			auto res = properties.emplace(id, std::move(data));
			reg(this);
		}

		template <typename T>
		auto getProperty(const std::string &id) {

